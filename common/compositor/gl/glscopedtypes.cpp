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

#include "glscopedtypes.h"

namespace hwcomposer {
void GLFramebufferDeleter::operator()(pointer fb_id) const {
  if (fb_id)
    glDeleteFramebuffers(1, &fb_id);
}

void GLVertexArrayDeleter::operator()(pointer vertexarray_id) const {
  if (vertexarray_id)
    glDeleteVertexArraysOES(1, &vertexarray_id);
}

void GLTextureDeleter::operator()(pointer texture_id) const {
  if (texture_id)
    glDeleteTextures(1, &texture_id);
}

void GLShaderDeleter::operator()(pointer shader_id) const {
  if (shader_id)
    glDeleteShader(shader_id);
}

void GLProgramDeleter::operator()(pointer program_id) const {
  if (program_id)
    glDeleteProgram(program_id);
}

}  // namespace hwcomposer
