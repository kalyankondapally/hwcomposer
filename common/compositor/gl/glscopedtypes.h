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

#ifndef GL_SCOPED_TYPES_H_
#define GL_SCOPED_TYPES_H_

#include <memory>
#define GL_GLEXT_PROTOTYPES

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace hwcomposer {

struct GLFramebufferDeleter {
  typedef GLuint pointer;
  void operator()(pointer fb_id) const;
};

struct GLVertexArrayDeleter {
  typedef GLuint pointer;
  void operator()(pointer vertexarray_id) const;
};

struct GLTextureDeleter {
  typedef GLuint pointer;
  void operator()(pointer texture_id) const;
};

struct GLShaderDeleter {
  typedef GLint pointer;
  void operator()(pointer shader_id) const;
};

struct GLProgramDeleter {
  typedef GLint pointer;
  void operator()(pointer program_id) const;
};

typedef std::unique_ptr<GLuint, GLFramebufferDeleter> ScopedGLFramebuffer;
typedef std::unique_ptr<GLuint, GLVertexArrayDeleter>
    ScopedGLVertexArrayDeleter;
typedef std::unique_ptr<GLuint, GLTextureDeleter> ScopedGLTexture;
typedef std::unique_ptr<GLint, GLShaderDeleter> ScopedGLShader;
typedef std::unique_ptr<GLint, GLProgramDeleter> ScopedGLProgram;

}  // namespace hwcomposer
#endif  // GL_SCOPED_TYPES_H_
