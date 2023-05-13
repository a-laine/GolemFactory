Skinning
{
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		skeletonPose : "mat4 array32";
		inverseBindPose : "mat4 array32";
	};
	
	vertex :
	{
		#version 330
		#define MAX_SKELETON_BONE 32

		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 vertexcolor;
		layout(location = 3) in ivec4 boneIDs;
		layout(location = 4) in vec4 weights;

		uniform mat4 model;			// model matrix (has to be present at this location)
		uniform mat4 view;			// view matrix
		uniform mat4 projection;	// projection matrix

		uniform mat4 skeletonPose[MAX_SKELETON_BONE]; 	// bone matrix
		uniform mat4 inverseBindPose[MAX_SKELETON_BONE]; 		// vertex to bone transformation matrix

		// output
		out vec4 lightDirectionCameraSpace;
		out vec4 fragmentNormal;
		out vec4 fragmentColor;

		vec4 lightPositionWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			// compute vertex position
			mat4 boneTransform  = skeletonPose[boneIDs.x] * inverseBindPose[boneIDs.x] * weights.x;
			boneTransform += skeletonPose[boneIDs.y] * inverseBindPose[boneIDs.y] * weights.y;
			boneTransform += skeletonPose[boneIDs.z] * inverseBindPose[boneIDs.z] * weights.z;
			boneTransform += skeletonPose[boneIDs.w] * inverseBindPose[boneIDs.w] * weights.w;
			vec4 transformPosition = boneTransform * position;
			
			// end
			gl_Position = projection * view * model * transformPosition;
			fragmentNormal = view * model * boneTransform * normal;
			normalize(fragmentNormal);
			fragmentColor = vertexcolor;
			
			vec4 eyeDirectionCameraSpace = -(view * model * transformPosition);
			vec4 lightPositionCameraSpace = view * lightPositionWorldSpace;
			lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	fragment:
	{
		#version 330

		// input
		in vec4 lightDirectionCameraSpace;
		in vec4 fragmentNormal;
		in vec4 fragmentColor;

		// output
		layout (location = 0) out vec4 fragColor;


		// program
		void main()
		{
			float costeta = clamp( dot(normalize(fragmentNormal), normalize(lightDirectionCameraSpace)), 0,1 );
			fragColor = fragmentColor * (0.5*costeta +0.9);
			fragColor.w = 1.0;
		}
	};
}