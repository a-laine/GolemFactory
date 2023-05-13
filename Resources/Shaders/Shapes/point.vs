#version 330

// input
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 vertexcolor;

// output
out vec4 fragmentColor;


// program
void main()
{
	gl_Position = position;
	fragmentColor = vertexcolor;
}
