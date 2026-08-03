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
#include "teches/gl_program.h"
#include "unique_handle.h"

#include "engine.hpp"
#include <map>
#include <set>

namespace gce
{
template <class T> class store
{
public:
    T &operator[] (size_t id)
    {
        if (id >= m_store.size())
        {
            m_store.resize(id + 1);
        }
        return m_store[id];
    }
    void clear()
    {
        m_store.clear();
    }
    void reserve(size_t n)
    {
        m_store.reserve(n);
    }
    const std::vector<T> &data() const
    {
        return m_store;
    }
private:
    std::vector<T> m_store;
};
}

class BufferGL
{
public:
    struct Deleter
    {
        void operator()(GLuint h) const
        {
            glDeleteBuffers(1, &h);
        }
    };
    using ubuffer = gce::unique_handle <GLuint, Deleter>;

    struct tag_nocreate {};
    static constexpr tag_nocreate nocreate;

    BufferGL(tag_nocreate) {}

    BufferGL()
    {
        GLuint name;
        glCreateBuffers(1, &name);
        m_buffer = ubuffer{name};
    }

    GLuint name() const
    {
        return m_buffer.name();
    }
    bool isBuffer() const
    {
        return m_buffer.name() != 0;
    }
private:
    ubuffer m_buffer;
};

class VertexArrayGL
{
public:
    struct Deleter
    {
        void operator()(GLuint h) const
        {
            glDeleteVertexArrays(1, &h);
        }
    };
    using uvao = gce::unique_handle < GLuint, Deleter >;

    struct tag_nocreate {};
    static constexpr tag_nocreate nocreate;

    VertexArrayGL(tag_nocreate) {}

    VertexArrayGL()
    {
        GLuint name;
        glCreateVertexArrays(1, &name);
        m_vao = uvao{name};
    }
    GLuint name() const
    {
        return m_vao.name();
    }
    bool isVertexArray() const
    {
        return m_vao.name() != 0;
    }
private:
    uvao m_vao;
};

class TextureGL
{
public:
    struct Deleter
    {
        void operator()(GLuint h) const
        {
            glDeleteTextures(1, &h);
        }
    };
    using utexture = gce::unique_handle < GLuint, Deleter >;
    struct tag_nocreate {};
    static constexpr tag_nocreate nocreate;

    TextureGL(tag_nocreate) {}

    TextureGL(GLenum target)
    {
        GLuint name;
        glCreateTextures(target, 1, &name);
        m_texture = utexture{name};
    }
    GLuint name() const
    {
        return m_texture.name();
    }
    bool isTexture() const
    {
        return m_texture.name() != 0;
    }

private:
    utexture m_texture;
};

inline GLenum toOpenGL(gcePimitiveType mode)
{
    switch (mode)
    {
    case gcePimitiveType::TRIANGLES:
        return GL_TRIANGLES;
    case gcePimitiveType::LINE_STRIP:
        return GL_LINE_STRIP;
    case gcePimitiveType::POINTS:
        return GL_POINTS;
    }
    return GL_POINTS;
}

struct gceTileCache
{
    struct TileCacheData
    {
        TextureGL tex{GL_TEXTURE_2D};
    };
    using tilekey = std::pair<gce::uuid, gce::tileid>;
    std::map<tilekey, TileCacheData> m_tree;
    std::set<tilekey> m_missing;
    std::set<tilekey> m_queried;
};

//! QPatch cache for DEM tiles
//! Note: patches are generated from lower LOD if not present in DB, so we do not track missing tiles
struct gceQPatchCache
{
    struct QPatchCacheData
    {
        geom::psAABB bbox;
        float hmax = 1.0f;
        gceQPatch patch;
    };
    using tilekey = std::pair<gce::uuid, gce::tileid>;
    std::map<tilekey, QPatchCacheData> m_tree;
    std::set<tilekey> m_queried;
};

struct gceStorageGL
{
    gl::ProgramManager progs;
    gceTileCache tiles;
    gceQPatchCache qpatches;
};

