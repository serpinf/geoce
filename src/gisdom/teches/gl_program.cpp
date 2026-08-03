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

#include "gl_program.h"
#include "engine.hpp"
#include "unique_handle.h"
#include "io/utils.h"

namespace gl
{
struct ShderDeleter
{
    void operator()(GLuint h) const
    {
        glDeleteShader(h);
    }
};

using ushader = gce::unique_handle < GLuint, ShderDeleter >;

class Shader
{
public:
    explicit Shader(const GLenum shaderType) : shaderType(shaderType), m_shader(0)
    {}

    bool create(const char *source, std::string &infoLog)
    {
        m_shader = ushader(glCreateShader(shaderType));

        // Get strings for glShaderSource.
        GLint length = (GLint)strlen((const char *)source);
        glShaderSource(m_shader.name(), 1, &source, &length);

        glCompileShader(m_shader.name());

        GLint isCompiled = GL_FALSE;
        glGetShaderiv(m_shader.name(), GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE)
        {
            GLint maxLength = 0;
            glGetShaderiv(m_shader.name(), GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the nullptr character
            infoLog.resize(maxLength);
            glGetShaderInfoLog(m_shader.name(), maxLength, &maxLength, infoLog.data());

            // Exit with failure.
            m_shader = ushader(0); // Don't leak the shader.
        }
        return isCompiled == GL_TRUE;
    }

    bool isOk() const
    {
        return glIsShader(m_shader.name());
    }

    GLenum shaderType;  //!< The OpenGL program type.
    ushader m_shader;  //!< Shader name
};

ProgramInfo::ProgramInfo()
{}

ProgramInfo::~ProgramInfo()
{}

void ProgramInfo::addShaderFile(GLenum shaderType, const char *fileName)
{
    auto ShaderSource = get_file_contents(fileName);
    if (ShaderSource.empty())
    {
        gceContext::log_error("Failed to read shader file {}", fileName);
    }
    addShaderSource(shaderType, ShaderSource.c_str());
}

void ProgramInfo::addShaderSource(GLenum shaderType, const char *source)
{
    Shader sh{shaderType};
    std::string infoLog;
    if (sh.create(source, infoLog))
    {
        m_shaders.push_back(std::move(sh));
    }
    else
    {
        gceContext::log_error("{}", infoLog);
    }
}

GLint Program::getUniformLocation(const char *name)
{
    GLint loc = glGetUniformLocation(m_program, name);
    if (loc < 0)
    {
        gceContext::log_error("Program->getAttribLocation('{}') failed", name);
    }
    return loc;
}

bool Program::Create(const ProgramInfo &info)
{
    m_program = glCreateProgram();

    //Attach our shaders to our program
    bool usesGeometryShader = false;
    for (auto &sh : info.m_shaders)
    {
        glAttachShader(m_program, sh.m_shader.name());
        if (sh.shaderType == GL_GEOMETRY_SHADER) usesGeometryShader = true;
    }

    if (usesGeometryShader)
    {
        glProgramParameteri(m_program, GL_GEOMETRY_INPUT_TYPE, info._nInputPrimitiveType);
        glProgramParameteri(m_program, GL_GEOMETRY_OUTPUT_TYPE, info._nOutputPrimitiveType);
        glProgramParameteri(m_program, GL_GEOMETRY_VERTICES_OUT, info._nVerticesOut);
    }

    //Link our program
    glLinkProgram(m_program);

    GLint isLinked = 0;
    glGetProgramiv(m_program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &maxLength);

        //The maxLength includes the nullptr character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(m_program, maxLength, &maxLength, infoLog.data());

        //We don't need the program anymore.
        glDeleteProgram(m_program);
        m_program = 0;

        gceContext::log_message("glLinkProgram:\n{}", infoLog.data());
        return false;
    }

    for (auto &sh : info.m_shaders)
    {
        glDetachShader(m_program, sh.m_shader.name());
    }
    return true;
}

Program::~Program()
{
    if (m_program != 0)
    {
        glDeleteProgram(m_program);
    }
}


///////////////////////////////////////////////////////////////////////////
//
// ProgramManager_GL imlementation
//
HProgram ProgramManager::FindOrCreate(const std::string &name)
{
    gl::HProgram res;

    if (auto it = m_progs.find(name); it != m_progs.end())
    {
        res = it->second.lock();
        if (!res)
        {
            res = std::make_shared<Program>();
            it->second = res;
        }
    }
    else
    {
        res = std::make_shared<Program>();
        m_progs.emplace(name, res);
    }
    return res;
}
}; //namespace gl
