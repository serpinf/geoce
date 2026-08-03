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

namespace gl
{

class Shader;

class ProgramInfo final
{
    friend class Program;
public:
    ProgramInfo();
    ~ProgramInfo();

    void addShaderFile(GLenum shaderType, const char *fileName);
    void addShaderSource(GLenum shaderType, const char *source);

    //!< Set the input primitive type for the geometry shader
    void SetInputPrimitiveType(int nInputPrimitiveType)
    {
        _nInputPrimitiveType = nInputPrimitiveType;
    }

    //!< Set the output primitive type for the geometry shader
    void SetOutputPrimitiveType(int nOutputPrimitiveType)
    {
        _nOutputPrimitiveType = nOutputPrimitiveType;
    }

    //!< Set the maximal number of vertices the geometry shader can output
    void SetVerticesOut(int nVerticesOut)
    {
        _nVerticesOut = nVerticesOut;
    }
private:
    std::vector<Shader> m_shaders;	   //!< List of all Shaders
    int _nInputPrimitiveType = GL_TRIANGLES;
    int _nOutputPrimitiveType = GL_TRIANGLE_STRIP;
    int _nVerticesOut = 3;
};

class Program final : boost::noncopyable
{
public:
    ~Program();

    bool IsOk() const
    {
        return glIsProgram(m_program) == GL_TRUE;
    }

    //! get program id
    GLuint getId() const
    {
        return m_program;
    }

    void Begin()
    {
        glUseProgram(m_program);
    }

    void End()
    {
        glUseProgram(0);
    }

    void setValue(GLint loc, bool value)
    {
        glUniform1i(loc, value ? GL_TRUE : GL_FALSE);
    }

    void setValue(GLint loc, float value)
    {
        glUniform1f(loc, value);
    }

    void setValue(GLint loc, const glm::dvec2 &value)
    {
        glUniform2dv(loc, 1, &value[0]);
    }

    void setValue(GLint loc, const glm::fvec4 &value)
    {
        glUniform4fv(loc, 1, &value[0]);
    }

    void setValue(GLint loc, const glm::mat4 &value)
    {
        glUniformMatrix4fv(loc, 1, GL_FALSE, &value[0][0]);
    }

    void setValue(GLint loc, const glm::dmat4 &value)
    {
        glUniformMatrix4dv(loc, 1, GL_FALSE, &value[0][0]);
    }


    GLint getAttribLocation(const char *attribName)
    {
        return glGetAttribLocation(m_program, attribName);
    }

    GLint getUniformLocation(const char *name);

    bool Create(const ProgramInfo &info);

private:
    GLuint m_program = 0; //!< GL ProgramObject
};
typedef std::shared_ptr<Program> HProgram;

class ProgramManager final : boost::noncopyable
{
    friend class Program;
public:
    HProgram FindOrCreate(const std::string &name);
private:
    std::map<std::string, std::weak_ptr<Program>> m_progs;
};
}; //namespace gl
