#version 330

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexcolor;
layout(location = 3) in ivec3 boneIDs;
layout(location = 4) in vec3 weights;

uniform mat4 model;			// model matrix (has to be present at this location)
uniform mat4 view;			// view matrix
uniform mat4 projection;	// projection matrix

const int MAX_BONES = 32;
uniform mat4 skeleton[MAX_BONES]; // bone matrix

// output
out vec3 lightDirectionCameraSpace;
out vec3 fragmentNormal;
out vec3 fragmentColor;

vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

// program
void main()
{
	// compute vertex position
	mat4 boneTransform = skeleton[boneIDs[0]] * weights[0];
	boneTransform += skeleton[boneIDs[1]] * weights[1];
	boneTransform += skeleton[boneIDs[2]] * weights[2];
	vec4 transformPosition = boneTransform * vec4(position, 1.0);
	
	// end
	gl_Position = projection * view * model * transformPosition;
	fragmentNormal = (view * model * boneTransform * vec4(normal,0.0)).xyz;
	fragmentColor = 2.0 * vertexcolor;
	
	vec3 eyeDirectionCameraSpace = - ( view * model * transformPosition).xyz;
	vec3 lightPositionCameraSpace = (view * lightCoordinateWorldSpace).xyz;
	lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
}

