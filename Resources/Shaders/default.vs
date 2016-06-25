#version 330

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexcolor;

//in vec2 texture;
//in vec3 weight;

uniform mat4 m; // model matrix (has to be present at this location)
uniform mat4 v; // view matrix
uniform mat4 p; // projection matrix

// output
out vec3 lightDirectionCameraSpace;
out vec3 fragmentNormal;
out vec3 fragmentColor;

vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

// program
void main()
{
	gl_Position = p*v*m * vec4(position,1);
	fragmentNormal = (v*m * vec4(normal,0.0)).xyz;
	fragmentColor = vertexcolor;
	
	vec3 eyeDirectionCameraSpace = - ( v*m * vec4(position,1)).xyz;
	vec3 lightPositionCameraSpace = (v * lightCoordinateWorldSpace).xyz;
	lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
}

