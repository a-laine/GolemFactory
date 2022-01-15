WiredSkinning
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
		out vec3 lightDirectionCameraSpace_gs;
		out vec3 fragmentNormal_gs;
		out vec3 fragmentColor_gs;

		vec4 lightPositionWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			// compute vertex position
			mat4 boneTransform  = skeletonPose[boneIDs.x] * inverseBindPose[boneIDs.x] * weights.x;
			boneTransform += skeletonPose[boneIDs.y] * inverseBindPose[boneIDs.y] * weights.y;
			boneTransform += skeletonPose[boneIDs.z] * inverseBindPose[boneIDs.z] * weights.z;
			vec4 transformPosition = boneTransform * vec4(position, 1.0);
			
			// end
			gl_Position = projection * view * model * transformPosition;
			fragmentNormal_gs = (view * model * boneTransform * vec4(normal,0.0)).xyz;
			normalize(fragmentNormal_gs);
			fragmentColor_gs = vertexcolor;
			
			vec3 eyeDirectionCameraSpace = - ( view * model * transformPosition).xyz;
			vec3 lightPositionCameraSpace = (view * lightPositionWorldSpace).xyz;
			lightDirectionCameraSpace_gs = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	
	geometry :
	{
		#version 330

		layout(triangles) in;
		layout(triangle_strip, max_vertices = 3) out;

		// input
		in vec3 lightDirectionCameraSpace_gs[];
		in vec3 fragmentNormal_gs[];
		in vec3 fragmentColor_gs[];

		// output
		out vec3 lightDirectionCameraSpace_fs;
		out vec3 fragmentNormal_fs;
		out vec3 fragmentColor_fs;
		out vec3 barycentricCoord;

		void main()
		{
			gl_Position = gl_in[0].gl_Position;
			fragmentNormal_fs = fragmentNormal_gs[0];
			fragmentColor_fs = fragmentColor_gs[0];
			lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[0];
			barycentricCoord = vec3(1.0,0.0,0.0);
			EmitVertex();
			
			gl_Position = gl_in[1].gl_Position;
			fragmentNormal_fs = fragmentNormal_gs[1];
			fragmentColor_fs = fragmentColor_gs[1];
			lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[1];
			barycentricCoord = vec3(0.0,1.0,0.0);
			EmitVertex();
			
			gl_Position = gl_in[2].gl_Position;
			fragmentNormal_fs = fragmentNormal_gs[2];
			fragmentColor_fs = fragmentColor_gs[2];
			lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[2];
			barycentricCoord = vec3(0.0,0.0,1.0);
			EmitVertex();
			
			EndPrimitive();
		}
	};

	fragment :
	{
		#version 330

		// input
		in vec3 lightDirectionCameraSpace_fs;
		in vec3 fragmentNormal_fs;
		in vec3 fragmentColor_fs;
		in vec3 barycentricCoord;

		// output
		layout (location = 0) out vec3 fragColor;


		// program
		float edgeFactor()
		{
			vec3 d = fwidth(barycentricCoord);
			vec3 a3 = smoothstep(vec3(0.0), d * 0.9 , barycentricCoord);
			return min(min(a3.x, a3.y), a3.z);
		}

		void main()
		{
			float costeta = clamp( dot(normalize(fragmentNormal_fs), normalize(lightDirectionCameraSpace_fs)), 0,1 );
			vec3 expectedColor = fragmentColor_fs * (0.7*costeta + 0.3);	

			if(edgeFactor() < 1.0) fragColor = expectedColor;
			else discard;
		}
	};
}