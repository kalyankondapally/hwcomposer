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

#ifndef DISPLAY_PLANE_ATOMIC_H_
#define DISPLAY_PLANE_ATOMIC_H_

#include <stdint.h>
#include <xf86drmMode.h>

#include "displayplane.h"

namespace hwcomposer {

class DisplayPlaneAtomic : public DisplayPlane {
 public:
  DisplayPlaneAtomic(uint32_t plane_id, uint32_t possible_crtcs);
  virtual ~DisplayPlaneAtomic();

  bool UpdateProperties(drmModeAtomicReqPtr property_set, uint32_t crtc_id,
                        const OverlayLayer* layer) const override;

  bool Disable(drmModeAtomicReqPtr property_set) override;

  bool CanCompositeLayer(const OverlayLayer* layer) override;

 private:
  bool InitializeProperties(
      uint32_t gpu_fd, const ScopedDrmObjectPropertyPtr& plane_props) override;
  void DumpAtomic() const override;

  struct Property {
    Property();
    bool Initialize(uint32_t fd, const char* name,
                    const ScopedDrmObjectPropertyPtr& plane_properties);
    uint32_t id = 0;
  };

  Property crtc_prop_;
  Property fb_prop_;
  Property crtc_x_prop_;
  Property crtc_y_prop_;
  Property crtc_w_prop_;
  Property crtc_h_prop_;
  Property src_x_prop_;
  Property src_y_prop_;
  Property src_w_prop_;
  Property src_h_prop_;
  Property rotation_prop_;
  Property alpha_prop_;
  Property in_fence_fd_prop_;
  std::vector<uint32_t> supported_formats_;
};

}  // namespace hwcomposer
#endif  // DISPLAY_PLANE_ATOMIC_H_
