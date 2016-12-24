#version 330

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexcolor;

//in vec2 texture;
//in vec3 weight;

uniform mat4 model; 	// model matrix (has to be present at this location)
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix

uniform vec4 wind;

// output
out vec3 lightDirectionCameraSpace;
out vec3 fragmentNormal;
out vec3 fragmentColor;

vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

// program
void main()
{
	vec3 tmp = position + position.z * wind.xyz;
	
	gl_Position = projection * view * model * vec4(tmp,1);
	fragmentNormal = (view * model * vec4(normal,0.0)).xyz;
	fragmentColor = 2.0 * vertexcolor;
	
	vec3 eyeDirectionCameraSpace = - ( view * model * vec4(tmp, 1.0)).xyz;
	vec3 lightPositionCameraSpace = (view * lightCoordinateWorldSpace).xyz;
	lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
}

