// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "apps/maxwell/src/suggestion_engine/ask_subscriber.h"
#include "apps/maxwell/src/suggestion_engine/suggestion_channel.h"

namespace maxwell {
namespace suggestion {

class AskSubscriber;
class AskController;
typedef BoundSuggestionChannel<AskSubscriber, AskController> AskChannel;

}  // namespace suggestion
}  // namespace maxwell
