#version 330

// from http://outerra.blogspot.com/2009/08/logarithmic-z-buffer.html
// the formula for OpenGL depth [-1,+1] is 
// z = (2*log(C*z + 1) / log(C*Far + 1) - 1);
// NOTE: as we use infinite perspective projection, need to
// protect FragDepth from falling outside [-1,+1] range

const float C=1.0;
const float Far = 2.0e+7;
const float offset = 2.0;

//! just call this in fragment shader to enable logariphmic depth
/*void logarithmic_depth(float clipZ)
{
	gl_FragDepth = min(log(C*clipZ+offset)/log(C*Far+offset), 1.0);
}*/

void logarithmic_depth()
{
	gl_FragDepth = min(log(C*gl_FragCoord.z/gl_FragCoord.w+offset)/log(C*Far+offset), 1.0);
}
