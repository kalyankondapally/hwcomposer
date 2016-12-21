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

#include "displayplane.h"

#include <drm_fourcc.h>

#include <gpudevice.h>
#include <hwctrace.h>

#include <overlaylayer.h>
#include "overlaybuffer.h"

namespace hwcomposer {

DisplayPlane::DisplayPlane(uint32_t plane_id, uint32_t possible_crtcs)
    : id_(plane_id), possible_crtc_mask_(possible_crtcs), enabled_(false) {
}

DisplayPlane::~DisplayPlane() {
}

bool DisplayPlane::Initialize(uint32_t gpu_fd,
                              const std::vector<uint32_t>& formats) {
  supported_formats_ = formats;

  ScopedDrmObjectPropertyPtr plane_props(
      drmModeObjectGetProperties(gpu_fd, id_, DRM_MODE_OBJECT_PLANE));
  if (!plane_props) {
    ETRACE("Unable to get plane properties.");
    return false;
  }
  uint32_t count_props = plane_props->count_props;
  for (uint32_t i = 0; i < count_props; i++) {
    ScopedDrmPropertyPtr property(
        drmModeGetProperty(gpu_fd, plane_props->props[i]));
    if (property && !strcmp(property->name, "type")) {
      type_ = plane_props->prop_values[i];
      break;
    }
  }

  return InitializeProperties(gpu_fd, plane_props);
}
#ifdef USE_DRM_ATOMIC
bool DisplayPlane::UpdateProperties(drmModeAtomicReqPtr /*property_set*/,
                                    uint32_t /*crtc_id*/,
                                    const OverlayLayer* /*layer*/) const {
  return false;
}

bool DisplayPlane::Disable(drmModeAtomicReqPtr /*property_set*/) {
  return false;
}
#endif
uint32_t DisplayPlane::id() const {
  return id_;
}

bool DisplayPlane::GetCrtcSupported(uint32_t pipe_id) const {
  return !!((1 << pipe_id) & possible_crtc_mask_);
}

void DisplayPlane::SetEnabled(bool enabled) {
  enabled_ = enabled;
}

uint32_t DisplayPlane::type() const {
  return type_;
}

bool DisplayPlane::ValidateLayer(const OverlayLayer* layer) {
  uint32_t format = layer->GetBuffer()->GetFormat();
  if (!IsSupportedFormat(format)) {
    // In case of primary we can fallback to XRGB.
    if (type_ == DRM_PLANE_TYPE_PRIMARY) {
      format = GetFormatForFrameBuffer(format);
      if (IsSupportedFormat(format)) {
        layer->GetBuffer()->SetRecommendedFormat(format);
        return true;
      }
    }

    IDISPLAYMANAGERTRACE(
        "Layer cannot be supported as format is not supported.");
    return false;
  }

  return CanCompositeLayer(layer);
}

bool DisplayPlane::CanCompositeLayer(const OverlayLayer* /*layer*/) {
  return true;
}

bool DisplayPlane::IsSupportedFormat(uint32_t format) {
  if (last_valid_format_ == format)
    return true;

  for (auto& element : supported_formats_) {
    if (element == format) {
      last_valid_format_ = format;
      return true;
    }
  }

  return false;
}

uint32_t DisplayPlane::GetFormatForFrameBuffer(uint32_t format) const {
  // We only support 24 bit colordepth for primary planes on
  // pre SKL Hardware. Ideally, we query format support from
  // plane to determine this.
  switch (format) {
    case DRM_FORMAT_ABGR8888:
      return DRM_FORMAT_XBGR8888;
    case DRM_FORMAT_ARGB8888:
      return DRM_FORMAT_XRGB8888;
    default:
      break;
  }

  return format;
}

bool DisplayPlane::InitializeProperties(
    uint32_t /*gpu_fd*/, const ScopedDrmObjectPropertyPtr& /*plane_props*/) {
  return true;
}

void DisplayPlane::Dump() const {
  DUMPTRACE("Plane Information Starts. -------------");
  DUMPTRACE("Plane ID: %d", id_);
  switch (type_) {
    case DRM_PLANE_TYPE_OVERLAY:
      DUMPTRACE("Type: Overlay.");
      break;
    case DRM_PLANE_TYPE_PRIMARY:
      DUMPTRACE("Type: Primary.");
      break;
    case DRM_PLANE_TYPE_CURSOR:
      DUMPTRACE("Type: Cursor.");
      break;
    default:
      ETRACE("Invalid plane type %d", type_);
  }

  for (uint32_t j = 0; j < supported_formats_.size(); j++)
    DUMPTRACE("Format: %4.4s", (char*)&supported_formats_[j]);

  DUMPTRACE("Enabled: %d", enabled_);

  DumpAtomic();
  DUMPTRACE("Plane Information Ends. -------------");
}

void DisplayPlane::DumpAtomic() const {
}

}  // namespace hwcomposer
