#version 330

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexcolor;

uniform mat4 model; 	// model matrix (has to be present at this location)
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix

// output
out vec4 delta_gs;

// program
void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	delta_gs = projection * view * model * vec4(position + 0.3 * normal, 1.0);
}

