#version 330

// input
in vec4 fragmentColor1;

uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);

// output
layout (location = 0) out vec4 fragColor;


// program
void main()
{
	if (overrideColor.x >= 0.0)
		fragColor = overrideColor;
	else fragColor = fragmentColor1;
}


