#version 460 core

in vec3 position;
in vec3 normal;
in vec2 texp;

uniform mat4 mvp;
uniform vec4 texbox;

out vec2 texPos;
out vec3 fnormal;

void main()
{
    //vec2 texp = vec2(gl_VertexID / 33, gl_VertexID % 33);
    texPos = mix(texbox.xy, texbox.zw, texp);
    //texPos = texp;

    vec4 pos = mvp * vec4(position, 1.0);
    fnormal = normal;
    gl_Position = pos;
} 
