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

#ifndef DISPLAY_PLANE_MANAGER_ATOMIC_H_
#define DISPLAY_PLANE_MANAGER_ATOMIC_H_

#include "displayplanemanager.h"

namespace hwcomposer {

class DisplayPlane;
class GpuDevice;

class DisplayPlaneManagerAtomic : public DisplayPlaneManager {
 public:
  DisplayPlaneManagerAtomic(uint32_t gpu_fd, uint32_t pipe_id,
                            uint32_t crtc_id);

  virtual ~DisplayPlaneManagerAtomic();

  bool CommitFrameAtomic(DisplayPlaneStateList &planes,
                         drmModeAtomicReqPtr pset, bool needs_modeset,
                         PageFlipState *state) override;

  void EndUpdate(drmModeAtomicReqPtr pset) override;

 protected:
  std::unique_ptr<DisplayPlane> CreatePlane(uint32_t plane_id,
                                            uint32_t possible_crtcs) override;
  bool TestCommit(
      const std::vector<OverlayPlane> &commit_planes) const override;
};

}  // namespace hwcomposer
#endif  // DISPLAY_PLANE_MANAGER_ATOMIC_H_
