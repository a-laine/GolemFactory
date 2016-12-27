#version 330

// input
layout(location = 0) in vec3 position;

uniform mat4 model; 	// model matrix (has to be present at this location)
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix

out vec3 vPosition;
out mat4 vMVPMatrix;

// program
void main()
{
	vMVPMatrix = projection * view * model;
	vPosition = position;
}

