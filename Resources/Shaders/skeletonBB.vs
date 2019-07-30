#version 330
#define MAX_SKELETON_BONE 32

// input
layout(location = 0) in ivec2 segmentIndex;
layout(location = 1) in float segmentRadius;

uniform mat4 model;			// model matrix (has to be present at this location)
uniform mat4 view;			// view matrix
uniform mat4 projection;	// projection matrix

uniform mat4 skeletonPose[MAX_SKELETON_BONE]; 	// bone matrix

// output
out vec4 segmentEnd;
out float radius;
out mat4 PVM;

// program
void main()
{
	if(segmentIndex.x == -1)
	{
		segmentEnd = vec4(0,0,0,0);
		gl_Position = vec4(0,0,0,0);
		radius = 0;
		PVM = projection * view * model;
	}
	else 
	{
		PVM = projection * view * model;
		radius = segmentRadius;
		segmentEnd = skeletonPose[segmentIndex.y][3];
		gl_Position = skeletonPose[segmentIndex.x][3];
	}
}

