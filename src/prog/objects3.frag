#version 330

uniform vec4 fixedColor;

layout(location = 0) out vec4 FragColor;
void logarithmic_depth()
{
    const float C=1.0;
    const float Far = 2.0e+7;
    const float offset = 2.0;

    gl_FragDepth = min(log(C*gl_FragCoord.z/gl_FragCoord.w+offset)/log(C*Far+offset), 1.0);
}

void main()
{
	FragColor = fixedColor;
    logarithmic_depth();
}

