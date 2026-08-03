#version 330

layout(location = 0) in vec3 position;

uniform mat4 mvp;
uniform float scale;

void main()
{
    gl_Position = mvp * vec4(position * scale, 1.0);
} 
