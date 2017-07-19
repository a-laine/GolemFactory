#version 330

// input
in vec3 fragmentColor;

// output
layout (location = 0) out vec3 fragColor;

// program
void main()
{
	fragColor = fragmentColor;
}


