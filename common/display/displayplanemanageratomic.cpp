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

#include "displayplanemanageratomic.h"

#include <overlaylayer.h>
#include <hwctrace.h>

#include "displayplaneatomic.h"
#include "overlaybuffer.h"

namespace hwcomposer {

DisplayPlaneManagerAtomic::DisplayPlaneManagerAtomic(uint32_t gpu_fd,
                                                     uint32_t pipe_id,
                                                     uint32_t crtc_id)
    : DisplayPlaneManager(gpu_fd, pipe_id, crtc_id) {
}

DisplayPlaneManagerAtomic::~DisplayPlaneManagerAtomic() {
}

bool DisplayPlaneManagerAtomic::CommitFrameAtomic(
    DisplayPlaneStateList &comp_planes, drmModeAtomicReqPtr pset,
    bool needs_modeset, PageFlipState *state) {
  CTRACE();
  if (!pset) {
    ETRACE("Failed to allocate property set %d", -ENOMEM);
    return false;
  }

  uint32_t flags = DRM_MODE_PAGE_FLIP_EVENT;
  if (needs_modeset) {
    flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
  } else {
#ifdef DISABLE_OVERLAY_USAGE
    flags |= DRM_MODE_ATOMIC_ALLOW_MODESET;
#else
    flags |= DRM_MODE_ATOMIC_NONBLOCK;
#endif
  }

  for (DisplayPlaneState &comp_plane : comp_planes) {
    DisplayPlane *plane = comp_plane.plane();
    OverlayLayer *layer = comp_plane.GetOverlayLayer();
    IDISPLAYMANAGERTRACE("Adding layer for Display Composition. Index: %d",
                         layer->GetIndex());

    if (!plane->UpdateProperties(pset, crtc_id_, layer))
      return false;

    plane->SetEnabled(true);
    layer->GetBuffer()->SetInUse(true);
  }

  int ret = drmModeAtomicCommit(gpu_fd_, pset, flags, state);

  if (ret) {
    ETRACE("Failed to commit pset ret=%s\n", PRINTERROR());
    return false;
  }

  return true;
}

void DisplayPlaneManagerAtomic::EndUpdate(drmModeAtomicReqPtr pset) {
  for (auto i = cursor_planes_.begin(); i != cursor_planes_.end(); ++i) {
    if ((*i)->IsEnabled())
      continue;

    (*i)->Disable(pset);
  }

  for (auto i = overlay_planes_.begin(); i != overlay_planes_.end(); ++i) {
    if ((*i)->IsEnabled())
      continue;

    (*i)->Disable(pset);
  }

  for (auto i = overlay_buffers_.begin(); i != overlay_buffers_.end();) {
    OverlayBuffer *buffer = i->get();
    if (buffer->InUse()) {
      buffer->IncrementRefCount();
      i++;
      continue;
    }

    buffer->DecreaseRefCount();

    if (buffer->RefCount() >= 0) {
      i++;
      continue;
    }
    IDISPLAYMANAGERTRACE("Deleted Buffer.");
    i->reset(nullptr);
    i = overlay_buffers_.erase(i);
  }
}

bool DisplayPlaneManagerAtomic::TestCommit(
    const std::vector<OverlayPlane> &commit_planes) const {
  ScopedDrmAtomicReqPtr pset(drmModeAtomicAlloc());
  IDISPLAYMANAGERTRACE("Total planes for Test Commit. %d ",
                       commit_planes.size());
  for (auto i = commit_planes.begin(); i != commit_planes.end(); i++) {
    if (!(i->plane->UpdateProperties(pset.get(), crtc_id_, i->layer))) {
      IDISPLAYMANAGERTRACE("Failed to update Plane. %s ", PRINTERROR());
      return false;
    }
  }

  if (drmModeAtomicCommit(gpu_fd_, pset.get(), DRM_MODE_ATOMIC_TEST_ONLY,
                          NULL)) {
    IDISPLAYMANAGERTRACE("Test Commit Failed. %s ", PRINTERROR());
    return false;
  }

  return true;
}

std::unique_ptr<DisplayPlane> DisplayPlaneManagerAtomic::CreatePlane(
    uint32_t plane_id, uint32_t possible_crtcs) {
  return std::unique_ptr<DisplayPlane>(
      new DisplayPlaneAtomic(plane_id, possible_crtcs));
}

}  // namespace hwcomposer
