#version 460 core

layout(location = 0) out vec4 FragColor;

uniform sampler2D tex;

in vec2 texPos;
in vec3 fnormal;

void logarithmic_depth()
{
    const float C=1.0;
    const float Far = 2.0e+7;
    const float offset = 2.0;

    gl_FragDepth = min(log(C*gl_FragCoord.z/gl_FragCoord.w+offset)/log(C*Far+offset), 1.0);
}

void main()
{
    float lt = max(0.0, dot(normalize(fnormal), normalize(vec3(1.0))));
	FragColor = lt * texture(tex, texPos);
	//FragColor = texture(tex, texPos);
    logarithmic_depth();
}

