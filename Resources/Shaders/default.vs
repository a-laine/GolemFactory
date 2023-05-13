#version 330

// input
layout(location = 0) in vec4 position;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec4 vertexcolor;

uniform mat4 model; 	// model matrix (has to be present at this location)
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix

// output
out vec4 lightDirectionCameraSpace;
out vec4 fragmentNormal;
out vec4 fragmentColor;

vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

// program
void main()
{
	gl_Position = projection * view * model * position;
	fragmentNormal = view * model * normal;
	fragmentColor = vertexcolor;
	
	vec4 eyeDirectionCameraSpace = -(view * model * position);
	vec4 lightPositionCameraSpace = view * lightCoordinateWorldSpace;
	lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
}

