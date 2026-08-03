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
#include "gcprec.h"

#include "gl_fbo.h"

namespace gl_fbo
{
static bool CheckFramebufferStatus(wxString &msg)
{
    GLenum status = (GLenum)glCheckFramebufferStatus(GL_FRAMEBUFFER);
    bool res = false;
    switch (status)
    {
    case GL_FRAMEBUFFER_COMPLETE:
        msg = wxT("FBO is ok!");
        res = true;
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        msg = wxT("Unsupported framebuffer format\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        msg = wxT("Framebuffer incomplete, missing attachment\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        msg = wxT("Framebuffer incomplete, duplicate attachment\n");
        break;
    //case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
    //	msg = wxT("Framebuffer incomplete, attached images must have same dimensions\n");
    //	break;
    //case GL_FRAMEBUFFER_INCOMPLETE_FORMATS:
    //	msg = wxT("Framebuffer incomplete, attached images must have same format\n");
    //	break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        msg = wxT("Framebuffer incomplete, missing draw buffer\n");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        msg = wxT("Framebuffer incomplete, missing read buffer\n");
        break;
    }
    return res;
}

///////////////////////////////////////////////////////////////////////////
//
// FBO layer as texture implementation
//
///////////////////////////////////////////////////////////////////////////
FBOLayerTexture::FBOLayerTexture(GLint format, const glm::ivec2 &size, int /*samples*/) : m_textarget(GL_TEXTURE_2D), m_format(format)
{
    /* init & allocate source texture */
    glGenTextures(1, &m_idTexture);
    glBindTexture(m_textarget, m_idTexture);
    glTexImage2D(m_textarget, 0, m_format, size.x, size.y, 0, GL_RGBA, GL_FLOAT, nullptr);

    glTexParameterf(m_textarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(m_textarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(m_textarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(m_textarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(m_textarget, 0);
}


FBOLayerTexture::~FBOLayerTexture()
{
    if (m_idTexture != 0)
    {
        glDeleteTextures(1, &m_idTexture);
        m_idTexture = 0;
    }
}
void FBOLayerTexture::reallocate(const glm::ivec2 &size, int /*samples*/)
{
    glBindTexture(m_textarget, m_idTexture);
    glTexImage2D(m_textarget, 0, m_format, size.x, size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(m_textarget, 0);
}

void FBOLayerTexture::_attach(GLenum target, GLenum attachment)
{
    glFramebufferTexture2D(target, attachment, m_textarget, m_idTexture, 0);
}

void FBOLayerTexture::BindTexture()
{
    glBindTexture(m_textarget, m_idTexture);
}

void FBOLayerTexture::UseMipmap(bool use)
{
    glBindTexture(m_textarget, m_idTexture);
    glTexParameterf(m_textarget, GL_TEXTURE_MIN_FILTER, use ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
    glBindTexture(m_textarget, 0);
}

void FBOLayerTexture::GenerateMipmap()
{
    glBindTexture(m_textarget, m_idTexture);
    glGenerateMipmap(m_textarget);
    glBindTexture(m_textarget, 0);
}
///////////////////////////////////////////////////////////////////////////
//
// FBO layer as render buffer implementation
//
///////////////////////////////////////////////////////////////////////////
FBOLayerRenderBuffer::FBOLayerRenderBuffer(GLenum format, const glm::ivec2 &size, int samples) : m_format(format)
{
    glGenRenderbuffers(1, &m_idRenderBuffer);

    // allocate some storage
    reallocate(size, samples);
}

FBOLayerRenderBuffer::~FBOLayerRenderBuffer()
{
    if (m_idRenderBuffer)
    {
        glDeleteRenderbuffers(1, &m_idRenderBuffer);
        m_idRenderBuffer = 0;
    }
}

void FBOLayerRenderBuffer::reallocate(const glm::ivec2 &size, int samples)
{
    // recreate a regular MSAA buffer
    glBindRenderbuffer(GL_RENDERBUFFER, m_idRenderBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, m_format, size.x, size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void FBOLayerRenderBuffer::_attach(GLenum target, GLenum attachment)
{
    glFramebufferRenderbuffer(target, attachment, GL_RENDERBUFFER, m_idRenderBuffer);
}

///////////////////////////////////////////////////////////////////////////
//
// FBO implementation
//
///////////////////////////////////////////////////////////////////////////
FrameBuffer::FrameBuffer(const glm::ivec2 &size, int samples) : m_saveFB(0), m_frameSize(size), m_depthSamples(samples)
{
    /* generate render-to-clipmap framebuffer */
    glGenFramebuffers(1, &m_idFB);
}

FrameBuffer::~FrameBuffer()
{
    if (m_idFB)
    {
        glDeleteFramebuffers(1, &m_idFB);
        m_idFB = 0;
    }
}

bool FrameBuffer::Resize(const glm::ivec2 &size, int samples)
{
    bool retOK = false;

    m_frameSize = size;
    m_depthSamples = samples;

    //try
    {
        reallocate();

        wxString msg;
        if (!CheckFramebufferStatus(msg))
        {
            //throw gl::Exception("buffer status error");
        }
        retOK = true;
    }
    //catch(const gl::Exception& s)
    {
        init_1x1();
        //wxLogError(s.error);
    }
    return retOK;
}

void FrameBuffer::Begin()
{
    glGetIntegerv(GL_VIEWPORT, &m_saveViewport[0]);
    glViewport(0, 0, m_frameSize.x, m_frameSize.y);

    GLint queryFB = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &queryFB);
    m_saveFB = static_cast<GLuint>(queryFB);
    glBindFramebuffer(GL_FRAMEBUFFER, m_idFB);
}

void FrameBuffer::End()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_saveFB);
    m_saveFB = 0;
    glViewport(m_saveViewport[0], m_saveViewport[1], m_saveViewport[2], m_saveViewport[3]);
}

void FrameBuffer::init_1x1()
{
    m_frameSize = glm::ivec2(1);
    m_depthSamples = 0;

    //try
    {
        reallocate();
    }
    //catch(const gl::Exception& s)
    {
        //wxLogError(s.error);
    }
}

void FrameBuffer::reallocate()
{
    for (container_type::value_type &_val : m_attachments)
    {
        _val.second->reallocate(m_frameSize, m_depthSamples);
    }
}

wxImage FrameBuffer::ResolveToImage(GLenum format)
{
    wxImage img(wxSize(m_frameSize.x, m_frameSize.y), false);
    if (img.IsOk())
    {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        // write AVI
        glReadPixels(0, 0, m_frameSize.x, m_frameSize.y, format, GL_UNSIGNED_BYTE, img.GetData());
    }
    return img;
}

void FrameBuffer::Attach(FBOLayer *layer, GLenum attachment)
{
    // update of GL state is done the way specific to FBOLayer imlementation
    layer->_attach(GL_FRAMEBUFFER, attachment);

    // now update our data structure to be in sync
    auto it = m_attachments.find(attachment);
    if (it == m_attachments.end())
    {
        // insert new attachment
        m_attachments.emplace(attachment, layer);
    }
    else
    {
        // just update existing attachment
        it->second = layer;
    }
}

void FrameBuffer::ResolveToFBO(FrameBuffer &resolveFB)
{
    wxASSERT(m_frameSize == resolveFB.m_frameSize);

    GLint saveFB = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &saveFB);

    // bind resolve buffers
    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_idFB);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFB.m_idFB);

    // resolve
    glBlitFramebuffer(0, 0, m_frameSize.x, m_frameSize.y, 0, 0, m_frameSize.x, m_frameSize.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // restore prev. state
    glBindFramebuffer(GL_FRAMEBUFFER, saveFB);
}
}
