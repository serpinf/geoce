#version 460 core

layout(location = 0) out vec4 FragColor;

uniform sampler2D tex;

in vec2 texPos;

void main()
{
	FragColor = texture(tex, texPos);
}

