#version 330

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexcolor;

// output
out vec3 fragmentColor;


// program
void main()
{
	gl_Position = vec4(position, 1.0);
	fragmentColor = vertexcolor;
}
