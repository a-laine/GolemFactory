#version 330

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 color;

uniform mat4 model; 	// model matrix (has to be present at this location)
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix
uniform int listsize = 0;

// output
out vec3 lightDirectionCameraSpace_gs;
out vec3 normal_gs;
out vec3 color_gs;

vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

// program
void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	normal_gs = (view * model * vec4(normal, 0.0)).xyz;
	color_gs = color;
	
	vec3 eyeDirectionCameraSpace = - ( view * model * vec4(position, 1.0)).xyz;
	vec3 lightPositionCameraSpace = (view * lightCoordinateWorldSpace).xyz;
	lightDirectionCameraSpace_gs = lightPositionCameraSpace + eyeDirectionCameraSpace;
}

