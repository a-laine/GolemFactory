#version 440

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture;
layout (location = 3) in vec3 weight;
layout (location = 4) in vec4 vertexcolor;

out vec2 texCoord;
out vec3 vPosition;
out mat4 vProjModMatrix;

void main()
{
	vProjModMatrix = gl_ModelViewProjectionMatrix;
	texCoord = texture;
	vPosition = position;
	
	//gl_Position = gl_ModelViewProjectionMatrix*vec4(position,1);
}

