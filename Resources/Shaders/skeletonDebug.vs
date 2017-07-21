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

// output
out int boneID;
out mat4 PVM;

// program
void main()
{	
	// compute most revelant bone ID
	boneID = boneIDs.x;
	if(weights.y > 0) boneID = boneIDs.y;
	if(weights.z > 0) boneID = boneIDs.z;
	
	// compute joint position (bone position)
	gl_Position = skeletonPose[boneID][3];
	
	// save matrix
	PVM = projection * view * model;
}

