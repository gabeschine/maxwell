// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/maxwell/src/integration/test.h"

#include "lib/ftl/synchronization/sleep.h"
#include "lib/mtl/tasks/message_loop.h"

constexpr auto kYieldSleepPeriod = ftl::TimeDelta::FromMilliseconds(1);
constexpr auto kYieldBatchPeriod = ftl::TimeDelta::FromMilliseconds(0);

void Yield() {
  // Tried a combination of Thread::sleep_for (formerly required) and
  // PostDelayedTask delays for a particular test sequence:
  //
  //        PostDelayedTask
  // s        0ms  1ms
  // l   w/o: 9.8s 8s
  // e   1ns: 8s
  // e   1ms: 7.9s 7.9s
  // p  10ms: 8s
  //
  // However, we've observed some additional flakiness in the Launcher tests
  // without the sleep.
  //
  // Based on those results, opt to sleep 1ms; post delayed w/ 0ms.
  ftl::SleepFor(kYieldSleepPeriod);

  // Combinations tried:
  //                      PostQuitTask QuitNow
  //               inline    no msgs    hang (invalid call per docs)
  // SetAfterTaskCallback     hang      hang
  //      PostDelayedTask      ok        ok
  mtl::MessageLoop::GetCurrent()->task_runner()->PostDelayedTask(
      [] { mtl::MessageLoop::GetCurrent()->PostQuitTask(); },
      kYieldBatchPeriod);
  mtl::MessageLoop::GetCurrent()->Run();
}

Predicate operator&&(const Predicate& a, const Predicate& b) {
  return [&a, &b] { return a() && b(); };
}

Predicate operator||(const Predicate& a, const Predicate& b) {
  return [&a, &b] { return a() || b(); };
}

Predicate operator!(const Predicate& a) {
  return [&a] { return !a(); };
}

Predicate Deadline(const ftl::TimeDelta& duration) {
  const auto deadline = ftl::TimePoint::Now() + duration;
  return [deadline] { return ftl::TimePoint::Now() >= deadline; };
}

void Sleep(const ftl::TimeDelta& duration) {
  WaitUntil(Deadline(duration));
}

void Sleep() {
  Sleep(ftl::TimeDelta::FromMilliseconds(1500));
}

MaxwellTestBase::MaxwellTestBase()
    : test_environment_host_(root_environment),
      test_environment_host_binding_(&test_environment_host_) {
  fidl::InterfaceHandle<app::ApplicationEnvironmentHost>
      test_environment_host_handle;
  test_environment_host_binding_.Bind(&test_environment_host_handle);
  root_environment->CreateNestedEnvironment(
      std::move(test_environment_host_handle), test_environment_.NewRequest(),
      NULL, "maxwell-test");
  test_environment_->GetApplicationLauncher(test_launcher_.NewRequest());
  agent_launcher_ =
      std::make_unique<maxwell::AgentLauncher>(test_environment_.get());
}

app::ServiceProviderPtr MaxwellTestBase::StartServiceProvider(
    const std::string& url) {
  app::ServiceProviderPtr services;
  auto launch_info = app::ApplicationLaunchInfo::New();
  launch_info->url = url;
  launch_info->services = services.NewRequest();
  test_launcher_->CreateApplication(std::move(launch_info), NULL);
  return services;
}

app::ApplicationEnvironment* root_environment;

int main(int argc, char** argv) {
  mtl::MessageLoop loop;
  auto app_context = app::ApplicationContext::CreateFromStartupInfo();
  root_environment = app_context->environment().get();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
