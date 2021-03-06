// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/maxwell/src/agents/ideas.h"

#include <rapidjson/document.h>

#include "application/lib/app/application_context.h"
#include "apps/maxwell/services/context/context_subscriber.fidl.h"
#include "apps/maxwell/services/suggestion/proposal_publisher.fidl.h"
#include "lib/mtl/tasks/message_loop.h"

constexpr char maxwell::agents::IdeasAgent::kIdeaId[];

namespace {

class IdeasAgentApp : public maxwell::agents::IdeasAgent,
                      public maxwell::ContextSubscriberLink {
 public:
  IdeasAgentApp()
      : app_context_(app::ApplicationContext::CreateFromStartupInfo()),
        maxwell_context_(
            app_context_
                ->ConnectToEnvironmentService<maxwell::ContextSubscriber>()),
        in_(this),
        out_(app_context_
                 ->ConnectToEnvironmentService<maxwell::ProposalPublisher>()) {
    fidl::InterfaceHandle<maxwell::ContextSubscriberLink> in_handle;
    in_.Bind(&in_handle);
    maxwell_context_->Subscribe("/location/region", std::move(in_handle));
  }

  void OnUpdate(maxwell::ContextUpdatePtr update) override {
    FTL_VLOG(1) << "OnUpdate from " << update->source << ": "
                << update->json_value;

    rapidjson::Document d;
    d.Parse(update->json_value.data());

    if (d.IsString()) {
      const std::string region = d.GetString();

      std::string idea;

      if (region == "Antarctica")
        idea = "Find penguins near me";
      else if (region == "The Arctic")
        idea = "Buy a parka";
      else if (region == "America")
        idea = "Go on a road trip";

      if (idea.empty()) {
        out_->Remove(kIdeaId);
      } else {
        auto p = maxwell::Proposal::New();
        p->id = kIdeaId;
        p->on_selected = fidl::Array<maxwell::ActionPtr>::New(0);
        auto d = maxwell::SuggestionDisplay::New();

        d->headline = idea;
        d->subheadline = "";
        d->details = "";
        d->color = 0x00aaaa00;  // argb yellow
        d->icon_urls = fidl::Array<fidl::String>::New(1);
        d->icon_urls[0] = "";
        d->image_url = "";
        d->image_type = maxwell::SuggestionImageType::PERSON;

        p->display = std::move(d);

        out_->Propose(std::move(p));
      }
    }
  }

 private:
  std::unique_ptr<app::ApplicationContext> app_context_;

  maxwell::ContextSubscriberPtr maxwell_context_;
  fidl::Binding<maxwell::ContextSubscriberLink> in_;
  maxwell::ProposalPublisherPtr out_;
};

}  // namespace

int main(int argc, const char** argv) {
  mtl::MessageLoop loop;
  IdeasAgentApp app;
  loop.Run();
  return 0;
}
