#version 330

// input
layout(location = 0) in vec3 vertexPosition_modelspace;

// program
void main()
{
	gl_Position.xyz = vertexPosition_modelspace; 
    gl_Position.w = 1.0; 
}