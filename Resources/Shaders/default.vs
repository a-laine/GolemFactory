#version 330

// input
layout(location = 0) in vec3 position;
in vec3 normal;
in vec2 texture;
in vec3 weight;
in vec4 vertexcolor;

uniform mat4 m; // model matrix (has to be present at this location)
uniform mat4 v; // view matrix
uniform mat4 p; // projection matrix

// output
out vec2 textureCoord;
out vec4 fragmentColor;
out vec3 fragmentNormal;

// program
void main()
{
	gl_Position = p*v*m * vec4(position,1);
	fragmentNormal = (v*m * vec4(normal,0.0)).xyz;
	textureCoord = texture;
	fragmentColor = vertexcolor;
}

