#version 330

layout(location = 0) in vec3 position;

uniform mat4 mvp;
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
        color = 0.5+0.5*position;
    }
	vec4 pos = mvp * vec4(position, 1.0);
    vertPos     = pos.xy / pos.w;
    startPos    = vertPos;
    gl_Position = pos;
} 
