#version 330
#define MAX_SKELETON_BONE 32

// input
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 vertexcolor;
layout(location = 3) in ivec3 boneIDs;
layout(location = 4) in vec3 weights;

uniform mat4 model;			// model matrix (has to be present at this location)
uniform mat4 view;			// view matrix
uniform mat4 projection;	// projection matrix

uniform mat4 skeletonPose[MAX_SKELETON_BONE]; 	// bone matrix
uniform mat4 inverseBindPose[MAX_SKELETON_BONE]; 		// vertex to bone transformation matrix

// output
out vec3 lightDirectionCameraSpace;
out vec3 fragmentNormal;
out vec3 fragmentColor;

vec4 lightPositionWorldSpace = vec4(1000,200,1500,1);

// program
void main()
{
	// length of normalized weights
	//normalize(weights);

	// compute vertex position
	mat4 boneTransform  = skeletonPose[boneIDs.x] * inverseBindPose[boneIDs.x] * weights.x;
	boneTransform += skeletonPose[boneIDs.y] * inverseBindPose[boneIDs.y] * weights.y;
	boneTransform += skeletonPose[boneIDs.z] * inverseBindPose[boneIDs.z] * weights.z;
	vec4 transformPosition = boneTransform * vec4(0.05 * position, 1.0);
	
	// end
	gl_Position = projection * view * model * transformPosition;
	fragmentNormal = (view * model * boneTransform * vec4(normal,0.0)).xyz;
	normalize(fragmentNormal);
	
	
	fragmentColor = vertexcolor;
	
	vec3 eyeDirectionCameraSpace = - ( view * model * transformPosition).xyz;
	vec3 lightPositionCameraSpace = (view * lightPositionWorldSpace).xyz;
	lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
}

