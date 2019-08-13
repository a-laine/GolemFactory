#version 330

// input
in vec3 fragmentColor1;

uniform vec3 overrideColor = vec3(-1.0 , 0.0 , 0.0);

// output
layout (location = 0) out vec3 fragColor;


// program
void main()
{
	if (overrideColor.x >= 0.0)
		fragColor = overrideColor;
	else fragColor = fragmentColor1;
}


