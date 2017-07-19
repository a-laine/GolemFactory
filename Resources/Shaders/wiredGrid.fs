#version 330

// output
layout (location = 0) out vec4 fragColor;

// input
in vec3 barycentricCoord;

// program
void main()
{
	if(barycentricCoord.x < 0.1)
		fragColor = vec4(0.937,0.894,0.690,1.0);
	else discard;
}


