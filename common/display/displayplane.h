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

#ifndef DISPLAY_PLANE_H_
#define DISPLAY_PLANE_H_

#include <vector>

#include <stdint.h>
#include <xf86drmMode.h>

#include <drmscopedtypes.h>

namespace hwcomposer {

class GpuDevice;
struct OverlayLayer;

class DisplayPlane {
 public:
  DisplayPlane(uint32_t plane_id, uint32_t possible_crtcs);

  virtual ~DisplayPlane();

  bool Initialize(uint32_t gpu_fd, const std::vector<uint32_t>& formats);

  virtual bool UpdateProperties(drmModeAtomicReqPtr property_set,
                                uint32_t crtc_id,
                                const OverlayLayer* layer) const;

  bool ValidateLayer(const OverlayLayer* layer);
#ifdef USE_DRM_ATOMIC
  virtual bool Disable(drmModeAtomicReqPtr property_set);
#endif
  uint32_t id() const;

  bool GetCrtcSupported(uint32_t pipe_id) const;

  uint32_t type() const;

  void SetEnabled(bool enabled);

  bool IsEnabled() const {
    return enabled_;
  }

  bool IsSupportedFormat(uint32_t format);

  void Dump() const;

 protected:
  virtual bool CanCompositeLayer(const OverlayLayer* layer);
  uint32_t GetFormatForFrameBuffer(uint32_t format) const;
  virtual bool InitializeProperties(
      uint32_t gpu_fd, const ScopedDrmObjectPropertyPtr& plane_props);
  virtual void DumpAtomic() const;

  uint32_t id_;

  uint32_t possible_crtc_mask_;

  uint32_t type_;

  uint32_t last_valid_format_;

  bool enabled_;

  std::vector<uint32_t> supported_formats_;
};

}  // namespace hwcomposer
#endif  // DISPLAY_PLANE_H_
