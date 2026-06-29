#pragma once

#include <RmlUi/Core/RenderInterface.h>
#include <glad/gl.h>
#include <unordered_map>
#include <cstdint>

namespace exd::app::rmlui {

/// @brief OpenGL 3.3+ render backend for RmlUi.
///
/// Submits 2D textured geometry batches from RmlUi to OpenGL.
/// Uses a simple shader program: orthographic projection, single texture sampler.
class RenderInterface_GL : public Rml::RenderInterface {
public:
    RenderInterface_GL();
    ~RenderInterface_GL() override;

    /// Called by RmlUi to submit a batch of textured geometry.
    void RenderGeometry(Rml::Vertex* vertices, int num_vertices,
                        int* indices, int num_indices,
                        Rml::TextureHandle texture,
                        const Rml::Vector2f& translation) override;

    /// Enable/disable the scissor region for clipping.
    void EnableScissorRegion(bool enable) override;
    void SetScissorRegion(int x, int y, int width, int height) override;

    /// Texture management.
    bool LoadTexture(Rml::TextureHandle& texture_handle,
                     Rml::Vector2i& texture_dimensions,
                     const Rml::String& source) override;
    bool GenerateTexture(Rml::TextureHandle& texture_handle,
                         const Rml::byte* source,
                         const Rml::Vector2i& source_dimensions) override;
    void ReleaseTexture(Rml::TextureHandle texture) override;

    /// Called when the viewport resizes.
    void set_viewport(int width, int height);

    /// Called each frame before rendering.
    void begin_frame();
    /// Called after all contexts have been rendered.
    void end_frame();

    /// Enable/disable the scissor test for the current frame.
    void set_scissor_enabled(bool enabled);

private:
    void setup_shader();

    GLuint shader_program_ = 0;
    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;

    GLint u_projection_ = -1;
    GLint u_texture_ = -1;
    GLint u_translation_ = -1;

    int viewport_width_ = 1280;
    int viewport_height_ = 720;
    bool scissor_enabled_ = false;
    bool scissor_region_set_ = false;
    int scissor_x_ = 0, scissor_y_ = 0, scissor_w_ = 0, scissor_h_ = 0;

    // Texture cache: handle → OpenGL texture ID
    std::unordered_map<Rml::TextureHandle, GLuint> textures_;
    Rml::TextureHandle next_texture_handle_ = 1;

    // Track GL state to avoid redundant calls
    bool blend_enabled_ = false;
    bool scissor_currently_ = false;
};

} // namespace exd::app::rmlui
