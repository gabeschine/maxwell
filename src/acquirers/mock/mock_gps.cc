// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "apps/maxwell/src/acquirers/mock/mock_gps.h"

namespace maxwell {
namespace acquirers {

constexpr char GpsAcquirer::kLabel[];

MockGps::MockGps(ContextEngine* context_engine) {
  maxwell::ContextPublisherPtr cx;
  context_engine->RegisterPublisher("MockGps", cx.NewRequest());

  cx->Publish(kLabel, out_.NewRequest());
}

void MockGps::Publish(float latitude, float longitude) {
  std::ostringstream json;
  json << "{ \"lat\": " << latitude << ", \"lng\": " << longitude << " }";
  out_->Update(json.str());
}

}  // namespace acquirers
}  // namespace maxwell
