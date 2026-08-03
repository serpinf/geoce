// Copyright 2026 Sergei Pikin
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#pragma once

#include <map>
#include <alg/gc_algebra.h>

namespace gl
{
class Context;
}

namespace gl_fbo
{
    /**
     * @struct fboConfig
     * @brief Defines the configuration properties of a Framebuffer Object (FBO).
     *
     * This structure specifies the rendering parameters and quality settings for
     * creating a framebuffer object, including color and depth buffer formats,
     * and multisample/supersample levels for antialiasing.
     */
struct fboConfig
{
    std::string name;           /*!< Configuration identifier name */
    GLenum colorFormat;         /*!< OpenGL format for the color buffer (e.g., GL_RGBA8) */
    GLenum depthFormat;         /*!< OpenGL format for the depth buffer (e.g., GL_DEPTH24_STENCIL8) */
    int depthSamples;           /*!< MSAA (Multisample Anti-Aliasing) sample count for depth buffer */
    int coverageSamples;        /*!< CSAA (Coverage Sampling Anti-Aliasing) sample count for coverage */
};

/**
 * @struct fboData
 * @brief Contains the GPU resource handles and state for a created Framebuffer Object.
 *
 * This structure holds OpenGL object IDs (GLuint) for all resources allocated
 * as part of an FBO, including render buffers, textures, and framebuffer objects.
 * It maintains both the main render target and the resolve target for MSAA operations.
 */
struct fboData
{
    GLuint resolveTex = 0;  /*!< Resolve target texture ID for MSAA resolve */
    GLuint resolveFB = 0;   /*!< Resolve target framebuffer ID for MSAA resolve */

    /* Main render target */
    GLuint fb = 0;          /*!< Multisample framebuffer object ID */
    GLuint colorRB = 0;     /*!< Color render buffer ID */
    GLuint depthRB = 0;     /*!< Depth render buffer ID */
};

/**
 * @class FBOLayer
 * @brief Abstract base class for framebuffer object attachment layers.
 *
 * FBOLayer defines the interface for different types of FBO attachments (such as
 * textures or render buffers). Derived classes implement specific attachment strategies
 * and storage management for rendering targets.
 *
 * @note This class is non-copyable (inherits from boost::noncopyable).
 */
class FBOLayer : boost::noncopyable
{
public:
    /**
     * @brief Virtual destructor for proper cleanup of derived classes.
     */
    virtual ~FBOLayer() {}

    /**
     * @brief Reallocates the attachment storage to a new size with optional MSAA samples.
     *
     * Changes the dimensions and/or multisample configuration of this attachment.
     * This may trigger GPU memory reallocation.
     *
     * @param size The new dimensions (width, height) in pixels.
     * @param samples The number of MSAA samples (1 for no multisampling).
     */
    virtual void reallocate(const glm::ivec2 &size, int samples) = 0;

    /**
     * @brief Attaches this layer to a framebuffer at the specified attachment point.
     *
     * Configures this layer as an attachment to an FBO at the given target and
     * attachment location.
     *
     * @param target The OpenGL framebuffer target (e.g., GL_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER).
     * @param attachment The attachment point (e.g., GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT).
     */
    virtual void _attach(GLenum target, GLenum attachment) = 0;
};

/**
 * @class FBOLayerTexture
 * @brief A framebuffer layer implemented as a texture attachment.
 *
 * FBOLayerTexture provides a texture-based attachment for FBO rendering. It supports
 * texture filtering, mipmapping, and can be used for sampling the render results.
 * Textures are more flexible than render buffers as they can be sampled in shaders.
 */
class FBOLayerTexture : public FBOLayer
{
public:
    /**
     * @brief Constructs a texture-based FBO layer.
     *
     * Creates a new texture attachment for framebuffer rendering with the specified
     * internal format and optional MSAA.
     *
     * @param format The OpenGL internal format for the texture (e.g., GL_RGBA8, GL_RGB16F).
     * @param size The dimensions (width, height) of the texture in pixels.
     * @param samples The number of MSAA samples (1 for no multisampling, should be 1 for textures).
     */
    FBOLayerTexture(GLint format, const glm::ivec2 &size, int samples);

    /**
     * @brief Destructor for FBOLayerTexture.
     */
    virtual ~FBOLayerTexture();

    /**
     * @brief Enables or disables mipmapping for this texture.
     *
     * Controls whether mipmaps are used for minification filtering.
     *
     * @param use true to enable mipmapping, false to disable.
     */
    void UseMipmap(bool use);

    /**
     * @brief Generates mipmaps for this texture.
     *
     * Automatically generates the complete mipmap chain from the base texture level.
     * Requires OpenGL 3.0+ or an appropriate extension.
     */
    void GenerateMipmap();

    /**
     * @brief Binds this texture to the current OpenGL context.
     *
     * Makes this texture the active texture for subsequent texture operations
     * (e.g., sampling in shaders, filtering configuration).
     */
    void BindTexture();

    /**
     * @brief Reallocates the texture storage to a new size with optional MSAA.
     *
     * @param size The new dimensions (width, height) in pixels.
     * @param samples The number of MSAA samples.
     */
    virtual void reallocate(const glm::ivec2 &size, int samples);

    /**
     * @brief Attaches this texture to a framebuffer at the specified attachment point.
     *
     * @param target The OpenGL framebuffer target.
     * @param attachment The attachment point.
     */
    virtual void _attach(GLenum target, GLenum attachment);

private:
    GLuint m_idTexture;         /*!< OpenGL texture object ID */
    const GLenum m_textarget;   /*!< Texture target type (e.g., GL_TEXTURE_2D, GL_TEXTURE_2D_MULTISAMPLE) */
    const GLint m_format;       /*!< Internal texture format */
};

/**
 * @class FBOLayerRenderBuffer
 * @brief A framebuffer layer implemented as a render buffer attachment.
 *
 * FBOLayerRenderBuffer provides a render-buffer-based attachment for FBO rendering.
 * Render buffers are optimized for rendering but cannot be directly sampled as textures.
 * They may be more memory-efficient than textures for rendering-only targets.
 */
class FBOLayerRenderBuffer final : public FBOLayer
{
public:
    /**
     * @brief Constructs a render buffer-based FBO layer.
     *
     * Creates a new render buffer attachment for framebuffer rendering with the specified
     * format and optional MSAA.
     *
     * @param format The OpenGL internal format for the render buffer (e.g., GL_RGBA8, GL_DEPTH24_STENCIL8).
     * @param size The dimensions (width, height) of the render buffer in pixels.
     * @param samples The number of MSAA samples (1 for no multisampling).
     */
    FBOLayerRenderBuffer(GLenum format, const glm::ivec2 &size, int samples);

    /**
     * @brief Destructor for FBOLayerRenderBuffer.
     */
    virtual ~FBOLayerRenderBuffer();

    /**
     * @brief Reallocates the render buffer storage to a new size with optional MSAA.
     *
     * @param size The new dimensions (width, height) in pixels.
     * @param samples The number of MSAA samples.
     */
    virtual void reallocate(const glm::ivec2 &size, int samples);

    /**
     * @brief Attaches this render buffer to a framebuffer at the specified attachment point.
     *
     * @param target The OpenGL framebuffer target.
     * @param attachment The attachment point.
     */
    virtual void _attach(GLenum target, GLenum attachment);

private:
    GLuint m_idRenderBuffer;    /*!< OpenGL render buffer object ID */
    const GLenum m_format;      /*!< Internal render buffer format */
};


    /**
     * @class FrameBuffer
     * @brief Manages a complete Framebuffer Object (FBO) with multiple attachment layers.
     *
     * FrameBuffer encapsulates the creation, configuration, and management of OpenGL
     * framebuffer objects. It handles MSAA rendering with automatic resolve operations,
     * attachment management, and rendering context switching. Supports both texture and
     * render buffer attachments.
     *
     * Typical usage:
     * - Create a FrameBuffer with desired size and sample count
     * - Attach color and depth layers using FBOLayerTexture or FBOLayerRenderBuffer
     * - Call Begin() to activate rendering to this FBO
     * - Perform rendering operations
     * - Call End() to return to previous render target
     * - Optionally resolve MSAA results to image or another FBO
     *
     * @note This class is non-copyable (inherits from boost::noncopyable).
     */
class FrameBuffer : boost::noncopyable
{
    typedef std::map<GLenum, FBOLayer *> container_type;

public:
    /**
     * @brief Constructs a FrameBuffer with specified dimensions and MSAA configuration.
     *
     * Initializes the FBO framework but attachments must be added separately using Attach().
     *
     * @param size The dimensions (width, height) of the framebuffer in pixels.
     * @param samples The number of MSAA samples (1 for no multisampling).
     */
    FrameBuffer(const glm::ivec2 &size, int samples);

    /**
     * @brief Destructor for FrameBuffer.
     *
     * Cleans up all attached layers and the framebuffer object.
     */
    ~FrameBuffer();

    /**
     * @brief Attempts to resize all attachments to new dimensions.
     *
     * Reallocates GPU storage for all attached layers to accommodate the new size
     * and optionally updates the MSAA sample count.
     *
     * @param size The new dimensions (width, height) in pixels.
     * @param samples The new number of MSAA samples.
     * @return true if resizing succeeded, false otherwise.
     */
    bool Resize(const glm::ivec2 &size, int samples);

    /**
     * @brief Attaches a layer to this framebuffer.
     *
     * Adds an FBOLayer (texture or render buffer) at the specified attachment point
     * (e.g., color attachment 0, depth attachment).
     *
     * @param layer Pointer to the FBOLayer to attach. Ownership is retained by the caller.
     * @param attachment The attachment point (e.g., GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT).
     */
    void Attach(FBOLayer *layer, GLenum attachment);

    /**
     * @brief Activates this framebuffer for rendering.
     *
     * Saves the current OpenGL framebuffer and viewport state, then binds this FBO
     * as the active render target. All subsequent rendering calls will draw to this FBO
     * until End() is called.
     */
    void Begin();

    /**
     * @brief Deactivates this framebuffer and restores the previous render target.
     *
     * Restores the framebuffer and viewport state that was saved by the most recent Begin() call.
     */
    void End();

    /**
     * @brief Returns the dimensions of this framebuffer.
     *
     * @return The framebuffer size (width, height) in pixels.
     */
    const glm::ivec2 &getSize() const
    {
        return m_frameSize;
    }

    /**
     * @brief Returns the OpenGL object ID of this framebuffer.
     *
     * @return The GLuint ID of the framebuffer object.
     */
    GLuint getName() const
    {
        return m_idFB;
    }

    /**
     * @brief Resolves the multisample render target to a wxImage.
     *
     * Performs MSAA resolve and reads the framebuffer contents into a wxImage.
     * Useful for exporting rendered content to image files or further processing.
     *
     * @param format The OpenGL format for reading pixels (e.g., GL_RGBA, GL_RGB).
     * @return A wxImage containing the resolved framebuffer contents.
     */
    wxImage ResolveToImage(GLenum format);

    /**
     * @brief Resolves the multisample render target to another framebuffer.
     *
     * Performs MSAA resolve by blitting from this multisample FBO to a single-sample
     * destination framebuffer. Useful for post-processing or subsequent rendering operations.
     *
     * @param resolveFB Reference to the destination framebuffer to receive the resolved content.
     */
    void ResolveToFBO(FrameBuffer &resolveFB);

private:
    /**
     * @brief Initializes a 1x1 dummy framebuffer.
     *
     * Creates a minimal framebuffer for testing or placeholder purposes.
     */
    void init_1x1();

    /**
     * @brief Reallocates storage for all attachments.
     *
     * Updates GPU memory allocation for all currently attached layers based on
     * current framebuffer size and sample configuration.
     *
     * @throws gl::Exception if reallocation fails.
     */
    void reallocate();

    GLuint m_idFB;                  /*!< This framebuffer object ID */
    GLuint m_saveFB;                /*!< Saved framebuffer ID (for nested Begin/End calls) */
    glm::ivec4 m_saveViewport;      /*!< Saved OpenGL viewport state */
    glm::ivec2 m_frameSize;         /*!< Framebuffer dimensions (width, height) */
    int m_depthSamples;             /*!< Depth buffer sample count for MSAA */
    container_type m_attachments;   /*!< Map of attachment points to FBOLayer pointers */
};
};
