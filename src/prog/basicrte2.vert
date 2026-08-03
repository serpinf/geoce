#version 460

layout(location = 0) in dvec2 position;

uniform mat4 mvp;
uniform dvec2 offset;
uniform bool useColor;
uniform vec4 fixedColor;

out vec3 color;
flat out vec2 startPos;
out vec2 vertPos;

void main()
{
    if (useColor) {
        color = fixedColor.rgb;
    }
    else {
        color = 0.5+0.5*vec3(position, 0.0);
    }
	vec4 pos = mvp * vec4(position+offset, 0.0, 1.0);
    vertPos     = pos.xy / pos.w;
    startPos    = vertPos;
    gl_Position = pos;
} 
