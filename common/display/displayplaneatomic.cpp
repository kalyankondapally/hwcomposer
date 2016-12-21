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

#include "displayplaneatomic.h"

#include <hwcdefs.h>
#include <hwctrace.h>

#include "overlaybuffer.h"
#include "overlaylayer.h"

namespace hwcomposer {

DisplayPlaneAtomic::Property::Property() {
}

bool DisplayPlaneAtomic::Property::Initialize(
    uint32_t fd, const char* name,
    const ScopedDrmObjectPropertyPtr& plane_props) {
  uint32_t count_props = plane_props->count_props;
  for (uint32_t i = 0; i < count_props; i++) {
    ScopedDrmPropertyPtr property(
        drmModeGetProperty(fd, plane_props->props[i]));
    if (property && !strcmp(property->name, name)) {
      id = property->prop_id;
      break;
    }
  }
  if (!id) {
    ETRACE("Could not find property %s", name);
    return true;
  }
  return false;
}

DisplayPlaneAtomic::DisplayPlaneAtomic(uint32_t plane_id,
                                       uint32_t possible_crtcs)
    : DisplayPlane(plane_id, possible_crtcs) {
}

DisplayPlaneAtomic::~DisplayPlaneAtomic() {
}

bool DisplayPlaneAtomic::InitializeProperties(
    uint32_t fd, const ScopedDrmObjectPropertyPtr& plane_props) {
  int ret = crtc_prop_.Initialize(fd, "CRTC_ID", plane_props);
  if (ret) {
    ETRACE("Could not get CRTC_ID property");
    return ret;
  }

  ret = fb_prop_.Initialize(fd, "FB_ID", plane_props);
  if (ret) {
    ETRACE("Could not get FB_ID property");
    return ret;
  }

  ret = crtc_x_prop_.Initialize(fd, "CRTC_X", plane_props);
  if (ret) {
    ETRACE("Could not get CRTC_X property");
    return ret;
  }

  ret = crtc_y_prop_.Initialize(fd, "CRTC_Y", plane_props);
  if (ret) {
    ETRACE("Could not get CRTC_Y property");
    return ret;
  }

  ret = crtc_w_prop_.Initialize(fd, "CRTC_W", plane_props);
  if (ret) {
    ETRACE("Could not get CRTC_W property");
    return ret;
  }

  ret = crtc_h_prop_.Initialize(fd, "CRTC_H", plane_props);
  if (ret) {
    ETRACE("Could not get CRTC_H property");
    return ret;
  }

  ret = src_x_prop_.Initialize(fd, "SRC_X", plane_props);
  if (ret) {
    ETRACE("Could not get SRC_X property");
    return ret;
  }

  ret = src_y_prop_.Initialize(fd, "SRC_Y", plane_props);
  if (ret) {
    ETRACE("Could not get SRC_Y property");
    return ret;
  }

  ret = src_w_prop_.Initialize(fd, "SRC_W", plane_props);
  if (ret) {
    ETRACE("Could not get SRC_W property");
    return ret;
  }

  ret = src_h_prop_.Initialize(fd, "SRC_H", plane_props);
  if (ret) {
    ETRACE("Could not get SRC_H property");
    return ret;
  }

  ret = rotation_prop_.Initialize(fd, "rotation", plane_props);
  if (ret)
    ETRACE("Could not get rotation property");

  ret = alpha_prop_.Initialize(fd, "alpha", plane_props);
  if (ret)
    ETRACE("Could not get alpha property");

  ret = in_fence_fd_prop_.Initialize(fd, "IN_FENCE_FD", plane_props);
  if (ret)
    ETRACE("Could not get IN_FENCE_FD property");

  return true;
}

bool DisplayPlaneAtomic::UpdateProperties(drmModeAtomicReqPtr property_set,
                                          uint32_t crtc_id,
                                          const OverlayLayer* layer) const {
  uint64_t alpha = 0xFF;
  OverlayBuffer* buffer = layer->GetBuffer();
  const HwcRect<int>& display_frame = layer->GetDisplayFrame();
  const HwcRect<float>& source_crop = layer->GetSourceCrop();
  int fence = layer->GetAcquireFence();
  if (layer->GetBlending() == HWCBlending::kBlendingPremult)
    alpha = layer->GetAlpha();

  IDISPLAYMANAGERTRACE("buffer->GetFb() ---------------------- STARTS %d",
                       buffer->GetFb());
  int success =
      drmModeAtomicAddProperty(property_set, id_, crtc_prop_.id, crtc_id) < 0;
  success |= drmModeAtomicAddProperty(property_set, id_, fb_prop_.id,
                                      buffer->GetFb()) < 0;
  success |= drmModeAtomicAddProperty(property_set, id_, crtc_x_prop_.id,
                                      display_frame.left) < 0;
  success |= drmModeAtomicAddProperty(property_set, id_, crtc_y_prop_.id,
                                      display_frame.top) < 0;
  if (type_ == DRM_PLANE_TYPE_CURSOR) {
    success |= drmModeAtomicAddProperty(property_set, id_, crtc_w_prop_.id,
                                        buffer->GetWidth()) < 0;
    success |= drmModeAtomicAddProperty(property_set, id_, crtc_h_prop_.id,
                                        buffer->GetHeight()) < 0;
  } else {
    success |= drmModeAtomicAddProperty(property_set, id_, crtc_w_prop_.id,
                                        layer->GetDisplayFrameWidth()) < 0;
    success |= drmModeAtomicAddProperty(property_set, id_, crtc_h_prop_.id,
                                        layer->GetDisplayFrameHeight()) < 0;
  }

  success |= drmModeAtomicAddProperty(property_set, id_, src_x_prop_.id,
                                      (int)(source_crop.left) << 16) < 0;
  success |= drmModeAtomicAddProperty(property_set, id_, src_y_prop_.id,
                                      (int)(source_crop.top) << 16) < 0;
  if (type_ == DRM_PLANE_TYPE_CURSOR) {
    success |= drmModeAtomicAddProperty(property_set, id_, src_w_prop_.id,
                                        buffer->GetWidth() << 16) < 0;
    success |= drmModeAtomicAddProperty(property_set, id_, src_h_prop_.id,
                                        buffer->GetHeight() << 16) < 0;
  } else {
    success |= drmModeAtomicAddProperty(property_set, id_, src_w_prop_.id,
                                        layer->GetSourceCropWidth() << 16) < 0;
    success |= drmModeAtomicAddProperty(property_set, id_, src_h_prop_.id,
                                        layer->GetSourceCropHeight() << 16) < 0;
  }

  if (rotation_prop_.id) {
    success = drmModeAtomicAddProperty(property_set, id_, rotation_prop_.id,
                                       layer->GetRotation()) < 0;
  }

  if (alpha_prop_.id) {
    success =
        drmModeAtomicAddProperty(property_set, id_, alpha_prop_.id, alpha) < 0;
  }

  if (fence != -1 && in_fence_fd_prop_.id) {
    success = drmModeAtomicAddProperty(property_set, id_, in_fence_fd_prop_.id,
                                       fence) < 0;
  }

  if (success) {
    ETRACE("Could not update properties for plane with id: %d", id_);
    return false;
  }
  IDISPLAYMANAGERTRACE("buffer->GetFb() ---------------------- ENDS%d",
                       buffer->GetFb());
  return true;
}

bool DisplayPlaneAtomic::Disable(drmModeAtomicReqPtr property_set) {
  enabled_ = false;
  int success =
      drmModeAtomicAddProperty(property_set, id_, crtc_prop_.id, 0) < 0;
  success |= drmModeAtomicAddProperty(property_set, id_, fb_prop_.id, 0) < 0;

  if (success) {
    ETRACE("Failed to disable plane with id: %d", id_);
    return false;
  }

  return true;
}

bool DisplayPlaneAtomic::CanCompositeLayer(const OverlayLayer* layer) {
  CTRACE();
  uint64_t alpha = 0xFF;

  if (layer->GetBlending() == HWCBlending::kBlendingPremult)
    alpha = layer->GetAlpha();

  if (type_ == DRM_PLANE_TYPE_OVERLAY && (alpha != 0 && alpha != 0xFF) &&
      alpha_prop_.id == 0) {
    IDISPLAYMANAGERTRACE(
        "Alpha property not supported, Cannot composite layer using Overlay.");
    return false;
  }

  if (layer->GetRotation() && rotation_prop_.id == 0) {
    IDISPLAYMANAGERTRACE(
        "Rotation property not supported, Cannot composite layer using "
        "Overlay.");
    return false;
  }

  return true;
}

void DisplayPlaneAtomic::DumpAtomic() const {
  if (alpha_prop_.id != 0)
    DUMPTRACE("Alpha property is supported.");

  if (rotation_prop_.id != 0)
    DUMPTRACE("Rotation property is supported.");

  if (crtc_prop_.id != 0)
    DUMPTRACE("CRTC_ID property is supported.");

  if (fb_prop_.id != 0)
    DUMPTRACE("FB_ID property is supported.");

  if (crtc_x_prop_.id != 0)
    DUMPTRACE("CRTC_X property is supported.");

  if (crtc_y_prop_.id != 0)
    DUMPTRACE("CRTC_Y property is supported.");

  if (crtc_w_prop_.id != 0)
    DUMPTRACE("CRTC_W property is supported.");

  if (crtc_h_prop_.id != 0)
    DUMPTRACE("CRTC_H property is supported.");

  if (src_x_prop_.id != 0)
    DUMPTRACE("SRC_X property is supported.");

  if (src_y_prop_.id != 0)
    DUMPTRACE("SRC_Y property is supported.");

  if (src_w_prop_.id != 0)
    DUMPTRACE("SRC_W property is supported.");

  if (src_h_prop_.id != 0)
    DUMPTRACE("SRC_H property is supported.");

  if (in_fence_fd_prop_.id != 0)
    DUMPTRACE("IN_FENCE_FD is supported.");
}

}  // namespace hwcomposer
