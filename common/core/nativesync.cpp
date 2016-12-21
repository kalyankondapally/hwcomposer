/*
// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include "nativesync.h"

#include <stdlib.h>

#include <sw_sync.h>
#include <sync/sync.h>

#include <hwctrace.h>

namespace hwcomposer {

NativeSync::NativeSync() {
}

NativeSync::~NativeSync() {
  if (timeline_fd_.get() >= 0)
    SignalCompositionDone();
}

bool NativeSync::Init() {
  timeline_fd_.Reset(sw_sync_timeline_create());
  if (timeline_fd_.get() < 0) {
    ETRACE("Failed to create sw sync timeline %s", PRINTERROR());
    return false;
  }

  return true;
}

int NativeSync::CreateNextTimelineFence() {
  ++timeline_;
  return sw_sync_fence_create(timeline_fd_.get(), "NativeSync", timeline_);
}

int NativeSync::MergeFence(int fence1, int fence2) {
  return sync_merge("MergeFence", fence1, fence2);
}

int NativeSync::IncreaseTimelineToPoint(int point) {
  int timeline_increase = point - timeline_current_;
  if (timeline_increase <= 0)
    return 0;

  int ret = sw_sync_timeline_inc(timeline_fd_.get(), timeline_increase);
  if (ret)
    ETRACE("Failed to increment sync timeline %s", PRINTERROR());
  else
    timeline_current_ = point;

  return ret;
}
}
