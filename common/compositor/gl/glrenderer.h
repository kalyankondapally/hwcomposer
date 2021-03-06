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

#ifndef GL_RENDERER_H_
#define GL_RENDERER_H_

#include "renderer.h"

#include "egloffscreencontext.h"
#include "glprogram.h"
#include "glscopedtypes.h"

namespace hwcomposer {

class GLRenderer : public Renderer {
 public:
  GLRenderer() = default;

  bool Init() override;
  void Draw(const std::vector<RenderState> &commands,
            NativeSurface *surface) override;

  void RestoreState() override;

  bool MakeCurrent() override;

 private:
  GLProgram *GetProgram(unsigned texture_count);

  EGLOffScreenContext context_;

  std::vector<std::unique_ptr<GLProgram>> programs_;
  ScopedGLVertexArrayDeleter vertex_array_;
};

}  // namespace hwcomposer
#endif  // GL_RENDERER_H_
