#version 460

uniform bool useStipple;

in vec3 color;
flat in vec2 startPos;
in vec2 vertPos;

layout(location = 0) out vec4 FragColor;

uint mask = 0xf0f0f0f0u;


void main()
{
	if (useStipple) {
		vec2  dir  = (vertPos.xy-startPos.xy) * gl_FragCoord.xy/(1.0+vertPos.xy);
		float dist = length(dir);
		//uint fac = uint(length(gl_FragCoord.xy)) & 31u;
		uint fac = uint(length(dir)) & 31u;
		if ((mask & (1u << fac)) == 0u) discard;
	}

	FragColor.rgb = color;
}

