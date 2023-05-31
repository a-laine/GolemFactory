Skinning
{
	uniform :
	{
		matrixArray : "mat4";
		view : "mat4";
		projection : "mat4";
		skeletonPose : "mat4 array32";
		inverseBindPose : "mat4 array32";
	};
	
	includes :
	{
		#version 420
		
		layout(std140, binding = 0) uniform GlobalMatrices
		{
			mat4 view;
			mat4 projection;
			vec4 cameraPosition;
		};
	};
	vertex :
	{
		#define MAX_SKELETON_BONE 32

		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in ivec4 boneIDs;
		layout(location = 3) in vec4 weights;


		uniform mat4 matrixArray[2];
		uniform mat4 skeletonPose[MAX_SKELETON_BONE]; 	// bone matrix
		uniform mat4 inverseBindPose[MAX_SKELETON_BONE]; 		// vertex to bone transformation matrix

		// output
		out vec4 fragmentPosition_gs;
		out vec4 fragmentNormal_gs;

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
			mat4 model = matrixArray[0];
			fragmentPosition_gs = model * transformPosition;
			gl_Position = projection * view * fragmentPosition_gs;
			fragmentNormal_gs = model * boneTransform * normal;
			normalize(fragmentNormal_gs);
		}
	};
	fragment : 
	{
		// input
		in vec4 fragmentNormal_fs;
		in vec4 fragmentPosition_fs;
		in vec3 barycentricCoord;
		 
		uniform vec4 overrideColor = vec4(0.0 , 0.0 , 0.0 , 0.0);

		// output
		layout (location = 0) out vec4 fragColor;


		// program
		float edgeFactor()
		{
			vec3 d = fwidth(barycentricCoord);
			vec3 a3 = smoothstep(vec3(0.0), d * 0.9 , barycentricCoord);
			return min(min(a3.x, a3.y), a3.z);
		}

		void main()
		{
			vec4 albedoColor = overrideColor;
			vec4 emmisiveColor = vec4(0.3 , 0.3 , 0.3 , 1);
			vec4 diffuse = clamp(dot(normalize(fragmentNormal_fs), normalize(-m_directionalLightDirection)), 0 , 1 ) * m_directionalLightColor;
			float metalicParam = 0.5;
			
			vec4 viewDir = normalize(cameraPosition - fragmentPosition_fs);
			vec4 reflectDir = reflect(normalize(m_directionalLightDirection), fragmentNormal_fs);  
			float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
			vec4 specular = metalicParam * spec * m_directionalLightColor;  
			
			fragColor = (diffuse + m_ambientColor + specular) * albedoColor + emmisiveColor;
		}
	};
}