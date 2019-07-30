#version 330

// input
layout(location = 0) in vec3 position;

uniform mat4 model; 	// model matrix (has to be present at this location)
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix

// program
void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
}

