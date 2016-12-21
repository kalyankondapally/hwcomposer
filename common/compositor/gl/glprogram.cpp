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

#include "glprogram.h"

#include <string>
#include <sstream>

#include "hwctrace.h"
#include "renderstate.h"

namespace hwcomposer {

// Shaders adopted from drm_hwcomposer project.
static ScopedGLShader CompileAndCheckShader(GLenum type, unsigned source_count,
                                            const GLchar **sources,
                                            std::ostringstream *shader_log) {
  GLint status;
  ScopedGLShader shader(glCreateShader(type));
  if (shader.get() == 0) {
    if (shader_log)
      *shader_log << "Failed glCreateShader call";
    return 0;
  }

  glShaderSource(shader.get(), source_count, sources, NULL);
  glCompileShader(shader.get());
  glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &status);
  if (!status) {
    if (shader_log) {
      GLint log_length;
      glGetShaderiv(shader.get(), GL_INFO_LOG_LENGTH, &log_length);
      std::string info_log(log_length, ' ');
      glGetShaderInfoLog(shader.get(), log_length, NULL, &info_log.front());
      *shader_log << "Failed to compile shader:\n" << info_log.c_str()
                  << "\nShader Source:\n";
      for (unsigned i = 0; i < source_count; i++) {
        *shader_log << sources[i];
      }
      *shader_log << "\n";
    }
    return 0;
  }

  return shader;
}

static std::string GenerateVertexShader(int layer_count) {
  std::ostringstream vertex_shader_stream;
  vertex_shader_stream
      << "#version 300 es\n"
      << "#define LAYER_COUNT " << layer_count << "\n"
      << "precision mediump int;\n"
      << "uniform vec4 uViewport;\n"
      << "uniform vec4 uLayerCrop[LAYER_COUNT];\n"
      << "uniform mat2 uTexMatrix[LAYER_COUNT];\n"
      << "in vec2 vPosition;\n"
      << "in vec2 vTexCoords;\n"
      << "out vec2 fTexCoords[LAYER_COUNT];\n"
      << "void main() {\n"
      << "  for (int i = 0; i < LAYER_COUNT; i++) {\n"
      << "    vec2 tempCoords = vTexCoords * uTexMatrix[i];\n"
      << "    fTexCoords[i] =\n"
      << "        uLayerCrop[i].xy + tempCoords * uLayerCrop[i].zw;\n"
      << "  }\n"
      << "  vec2 scaledPosition = uViewport.xy + vPosition * uViewport.zw;\n"
      << "  gl_Position =\n"
      << "      vec4(scaledPosition * vec2(2.0) - vec2(1.0), 0.0, 1.0);\n"
      << "}\n";
  return vertex_shader_stream.str();
}

static std::string GenerateFragmentShader(int layer_count) {
  std::ostringstream fragment_shader_stream;
  fragment_shader_stream << "#version 300 es\n"
                         << "#define LAYER_COUNT " << layer_count << "\n"
                         << "#extension GL_OES_EGL_image_external : require\n"
                         << "precision mediump float;\n";
  for (int i = 0; i < layer_count; ++i) {
    fragment_shader_stream << "uniform samplerExternalOES uLayerTexture" << i
                           << ";\n";
  }
  fragment_shader_stream << "uniform float uLayerAlpha[LAYER_COUNT];\n"
                         << "uniform float uLayerPremult[LAYER_COUNT];\n"
                         << "in vec2 fTexCoords[LAYER_COUNT];\n"
                         << "out vec4 oFragColor;\n"
                         << "void main() {\n"
                         << "  vec3 color = vec3(0.0, 0.0, 0.0);\n"
                         << "  float alphaCover = 1.0;\n"
                         << "  vec4 texSample;\n"
                         << "  vec3 multRgb;\n";
  for (int i = 0; i < layer_count; ++i) {
    if (i > 0)
      fragment_shader_stream << "  if (alphaCover > 0.5/255.0) {\n";
    // clang-format off
    fragment_shader_stream << "  texSample = texture2D(uLayerTexture" << i
                           << ",\n"
                           << "                        fTexCoords[" << i
                           << "]);\n"
                           << "  multRgb = texSample.rgb *\n"
                           << "            max(texSample.a, uLayerPremult[" << i
                           << "]);\n"
                           << "  color += multRgb * uLayerAlpha[" << i
                           << "] * alphaCover;\n"
                           << "  alphaCover *= 1.0 - texSample.a * uLayerAlpha["
                           << i << "];\n";
    // clang-format on
  }
  for (int i = 0; i < layer_count - 1; ++i)
    fragment_shader_stream << "  }\n";
  fragment_shader_stream << "  oFragColor = vec4(color, 1.0 - alphaCover);\n"
                         << "}\n";
  return fragment_shader_stream.str();
}

static ScopedGLProgram GenerateProgram(unsigned num_textures,
                                       std::ostringstream *shader_log) {
  std::string vertex_shader_string = GenerateVertexShader(num_textures);
  const GLchar *vertex_shader_source = vertex_shader_string.c_str();
  ScopedGLShader vertex_shader = CompileAndCheckShader(
      GL_VERTEX_SHADER, 1, &vertex_shader_source, shader_log);
  if (!vertex_shader.get())
    return 0;

  std::string fragment_shader_string = GenerateFragmentShader(num_textures);
  const GLchar *fragment_shader_source = fragment_shader_string.c_str();
  ScopedGLShader fragment_shader = CompileAndCheckShader(
      GL_FRAGMENT_SHADER, 1, &fragment_shader_source, shader_log);
  if (!fragment_shader.get())
    return 0;

  ScopedGLProgram program(glCreateProgram());
  if (!program.get()) {
    if (shader_log)
      *shader_log << "Failed to create program." << "\n";
    return 0;
  }

  glAttachShader(program.get(), vertex_shader.get());
  glAttachShader(program.get(), fragment_shader.get());
  glBindAttribLocation(program.get(), 0, "vPosition");
  glBindAttribLocation(program.get(), 1, "vTexCoords");
  glLinkProgram(program.get());
  glDetachShader(program.get(), vertex_shader.get());
  glDetachShader(program.get(), fragment_shader.get());

  GLint status;
  glGetProgramiv(program.get(), GL_LINK_STATUS, &status);
  if (!status) {
    if (shader_log) {
      GLint log_length;
      glGetProgramiv(program.get(), GL_INFO_LOG_LENGTH, &log_length);
      std::string program_log(log_length, ' ');
      glGetProgramInfoLog(program.get(), log_length, NULL,
                          &program_log.front());
      *shader_log << "Failed to link program:\n" << program_log.c_str() << "\n";
    }
    return 0;
  }

  return program;
}

GLProgram::GLProgram() : initialized_(false) {
}

GLProgram::~GLProgram() {
}

bool GLProgram::Init(unsigned texture_count) {
  std::ostringstream shader_log;
  program_ = GenerateProgram(texture_count, &shader_log);
  if (program_.get() == 0) {
    ETRACE("%s", shader_log.str().c_str());
    return false;
  }

  return true;
}

void GLProgram::UseProgram(const RenderState &state, GLuint viewport_width,
                           GLuint viewport_height) {
  glUseProgram(program_.get());
  unsigned size = state.layer_state_.size();
  if (!initialized_) {
    viewport_loc_ = glGetUniformLocation(program_.get(), "uViewport");
    crop_loc_ = glGetUniformLocation(program_.get(), "uLayerCrop");
    alpha_loc_ = glGetUniformLocation(program_.get(), "uLayerAlpha");
    premult_loc_ = glGetUniformLocation(program_.get(), "uLayerPremult");
    tex_matrix_loc_ = glGetUniformLocation(program_.get(), "uTexMatrix");
    for (unsigned src_index = 0; src_index < size; src_index++) {
      std::ostringstream texture_name_formatter;
      texture_name_formatter << "uLayerTexture" << src_index;
      GLuint tex_loc = glGetUniformLocation(
          program_.get(), texture_name_formatter.str().c_str());
      glUniform1i(tex_loc, src_index);
    }

    initialized_ = true;
  }

  glUniform4f(viewport_loc_, state.x_ / (float)viewport_width,
              state.y_ / (float)viewport_height,
              (state.width_) / (float)viewport_width,
              (state.height_) / (float)viewport_height);

  for (unsigned src_index = 0; src_index < size; src_index++) {
    const RenderState::LayerState &src = state.layer_state_[src_index];
    glUniform1f(alpha_loc_ + src_index, src.alpha_);
    glUniform1f(premult_loc_ + src_index, src.premult_);
    glUniform4f(crop_loc_ + src_index, src.crop_bounds_[0], src.crop_bounds_[1],
                src.crop_bounds_[2] - src.crop_bounds_[0],
                src.crop_bounds_[3] - src.crop_bounds_[1]);
    glUniformMatrix2fv(tex_matrix_loc_ + src_index, 1, GL_FALSE,
                       src.texture_matrix_);
    glActiveTexture(GL_TEXTURE0 + src_index);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, src.handle_);
  }
}

}  // namespace hwcomposer
