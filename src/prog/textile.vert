#version 460 core

layout(location = 0) in vec2 position;

uniform mat4 mvp;
uniform vec4 texbox;

out vec2 texPos;

void main()
{
    vec2 texp = vec2(gl_VertexID >> 1, gl_VertexID & 1);
    texPos = mix(texbox.xy, texbox.zw, texp);
	vec4 pos = mvp * vec4(position, 0.0, 1.0);
    gl_Position = pos;
} 
