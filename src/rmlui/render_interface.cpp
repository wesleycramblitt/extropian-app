#include "render_interface.hpp"
#include <cstdio>
#include <cstring>
#include <vector>
#include <cmath>
#include <cstddef>

// ── Embedded shader sources ──────────────────────────────────────────
static const char* kVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 a_color;
layout(location = 2) in vec2 a_texcoord;

uniform mat4 u_projection;
uniform vec2 u_translation;

out vec4 v_color;
out vec2 v_texcoord;

void main() {
    vec2 pos = a_position + u_translation;
    gl_Position = u_projection * vec4(pos, 0.0, 1.0);
    v_color = a_color;
    v_texcoord = a_texcoord;
}
)";

static const char* kFragmentShader = R"(
#version 330 core
in vec4 v_color;
in vec2 v_texcoord;

uniform sampler2D u_texture;

out vec4 frag_color;

void main() {
    frag_color = v_color * texture(u_texture, v_texcoord);
}
)";

namespace exd::app::rmlui {

RenderInterface_GL::RenderInterface_GL() {
    setup_shader();
}

RenderInterface_GL::~RenderInterface_GL() {
    if (shader_program_) glDeleteProgram(shader_program_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (ebo_) glDeleteBuffers(1, &ebo_);
    for (auto& [_, tex] : textures_) glDeleteTextures(1, &tex);
}

void RenderInterface_GL::setup_shader() {
    // Compile vertex shader
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &kVertexShader, nullptr);
    glCompileShader(vs);

    // Compile fragment shader
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &kFragmentShader, nullptr);
    glCompileShader(fs);

    // Link program
    shader_program_ = glCreateProgram();
    glAttachShader(shader_program_, vs);
    glAttachShader(shader_program_, fs);
    glLinkProgram(shader_program_);

    glDeleteShader(vs);
    glDeleteShader(fs);

    u_projection_  = glGetUniformLocation(shader_program_, "u_projection");
    u_translation_ = glGetUniformLocation(shader_program_, "u_translation");
    u_texture_     = glGetUniformLocation(shader_program_, "u_texture");

    // Create VAO + VBO + EBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // RmlUi vertex layout: position (vec2), color (ubyte4), texcoord (vec2)
    // Allocate max space — we'll upload each frame
    glBufferData(GL_ARRAY_BUFFER, 256 * 1024, nullptr, GL_DYNAMIC_DRAW); // 256KB vertex buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 256 * 1024, nullptr, GL_DYNAMIC_DRAW); // 256KB index buffer

    // a_position: vec2
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex),
                          (void*)offsetof(Rml::Vertex, position));
    glEnableVertexAttribArray(0);
    // a_color: vec4 (ubyte normalized)
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Rml::Vertex),
                          (void*)offsetof(Rml::Vertex, colour));
    glEnableVertexAttribArray(1);
    // a_texcoord: vec2
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex),
                          (void*)offsetof(Rml::Vertex, tex_coord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void RenderInterface_GL::set_viewport(int width, int height) {
    viewport_width_ = width;
    viewport_height_ = height;
}

void RenderInterface_GL::begin_frame() {
    glUseProgram(shader_program_);
    glBindVertexArray(vao_);

    // Orthographic projection: (0,0) at top-left
    float L = 0.0f, R = static_cast<float>(viewport_width_);
    float T = static_cast<float>(viewport_height_), B = 0.0f;
    float proj[16] = {
        2.0f/(R-L), 0, 0, 0,
        0, 2.0f/(B-T), 0, 0,
        0, 0, -1, 0,
        (R+L)/(L-R), (T+B)/(B-T), 0, 1
    };
    glUniformMatrix4fv(u_projection_, 1, GL_FALSE, proj);
    glUniform1i(u_texture_, 0);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    blend_enabled_ = true;
    scissor_currently_ = false;
    glDisable(GL_SCISSOR_TEST);
}

void RenderInterface_GL::end_frame() {
    if (blend_enabled_) {
        glDisable(GL_BLEND);
        blend_enabled_ = false;
    }
    if (scissor_currently_) {
        glDisable(GL_SCISSOR_TEST);
        scissor_currently_ = false;
    }
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
    glUseProgram(0);
}

void RenderInterface_GL::RenderGeometry(Rml::Vertex* vertices, int num_vertices,
                                         int* indices, int num_indices,
                                         Rml::TextureHandle texture,
                                         const Rml::Vector2f& translation) {
    if (num_vertices == 0 || num_indices == 0) return;

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, num_vertices * sizeof(Rml::Vertex), vertices);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, num_indices * sizeof(int), indices);

    // Bind texture
    auto it = textures_.find(texture);
    if (it != textures_.end()) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, it->second);
    }

    // Set translation
    glUniform2f(u_translation_, translation.x, translation.y);

    // Apply scissor if enabled
    if (scissor_enabled_ && scissor_region_set_) {
        if (!scissor_currently_) glEnable(GL_SCISSOR_TEST);
        glScissor(scissor_x_, viewport_height_ - (scissor_y_ + scissor_h_), scissor_w_, scissor_h_);
        scissor_currently_ = true;
    } else if (scissor_currently_) {
        glDisable(GL_SCISSOR_TEST);
        scissor_currently_ = false;
    }

    // Draw
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, nullptr);
}

void RenderInterface_GL::EnableScissorRegion(bool enable) {
    scissor_enabled_ = enable;
}

void RenderInterface_GL::SetScissorRegion(int x, int y, int width, int height) {
    scissor_x_ = x;
    scissor_y_ = y;
    scissor_w_ = width;
    scissor_h_ = height;
    scissor_region_set_ = true;
}

bool RenderInterface_GL::LoadTexture(Rml::TextureHandle& texture_handle,
                                      Rml::Vector2i& texture_dimensions,
                                      const Rml::String& source) {
    // RmlUi will call GenerateTexture for us with decoded data,
    // so we don't need to implement file loading here.
    // This is only called when RmlUi needs a texture from disk.
    return false; // Let RmlUi fall back to GenerateTexture
}

bool RenderInterface_GL::GenerateTexture(Rml::TextureHandle& texture_handle,
                                          const Rml::byte* source,
                                          const Rml::Vector2i& source_dimensions) {
    GLuint tex_id = 0;
    glGenTextures(1, &tex_id);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, source);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    texture_handle = next_texture_handle_++;
    textures_[texture_handle] = tex_id;
    return true;
}

void RenderInterface_GL::ReleaseTexture(Rml::TextureHandle texture) {
    auto it = textures_.find(texture);
    if (it != textures_.end()) {
        glDeleteTextures(1, &it->second);
        textures_.erase(it);
    }
}

void RenderInterface_GL::set_scissor_enabled(bool enabled) {
    if (!enabled && scissor_currently_) {
        glDisable(GL_SCISSOR_TEST);
        scissor_currently_ = false;
    }
}

} // namespace exd::app::rmlui
