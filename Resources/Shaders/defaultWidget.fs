#version 330

// output
layout (location = 0) out vec4 fragColor;

uniform vec4 color;

// program
void main()
{
	fragColor = color;
}


