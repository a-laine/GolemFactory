LightClustering
{
	computeShader : true;
	
	includes :
	{
		#version 430
		
		layout(std140, binding = 0) uniform GlobalMatrices
		{
			mat4 view;
			mat4 projection;
			vec4 cameraPosition;
		};
		
		struct Light
		{
			vec4 m_position;
			vec4 m_direction;
			vec4 m_color;
			float m_range;
			float m_intensity;
			float m_inCutOff;
			float m_outCutOff;
		};
		layout(std140, binding = 2) uniform Lights
		{
		    int lightCount;
			uint shadingConfiguration;
			float clusterDepthScale;
			float clusterDepthBias;
			float near;
			float far;
			float tanFovX;
			float tanFovY;
			Light lights[128];
		};
	};
	compute :
	{
		layout (local_size_x = 4 , local_size_y = 3 , local_size_z = 8) in;

		layout(r32ui, binding = 0) restrict uniform uimage3D lightClusters;

		void main()
		{
			// aliases
			ivec3 clusterIndex = ivec3(gl_GlobalInvocationID.xyz);
			ivec3 clusterSize = imageSize(lightClusters);
			vec3 invImageSize = vec3(1.0 / clusterSize.x, 1.0 / clusterSize.y, 1.0 / clusterSize.z);
			float nearFarRatio = far / near;
			uvec4 clusterLightMask = uvec4(0xFFFFFFFF);
			
			if (lightCount > 0)
			{
				float maxDepthBound = -pow(nearFarRatio, float(clusterIndex.z + 1) * invImageSize.z) * near;
				float minDepthBound;
				if (clusterIndex.z == 0)
					minDepthBound = 1.0;
				else
					minDepthBound = -pow(nearFarRatio, float(clusterIndex.z) * invImageSize.z) * near;
				
				vec2 frustrumSize = vec2(-tanFovX * maxDepthBound, -tanFovY * maxDepthBound);
				vec2 cellSize = 2.0 * frustrumSize * invImageSize.xy;
				
				vec3 cellCornerMin = vec3(clusterIndex.xy * cellSize - frustrumSize, maxDepthBound);
				vec3 cellCornerMax = vec3(cellCornerMin.xy + cellSize, minDepthBound);
				vec3 cellCenter = 0.5 * (cellCornerMax + cellCornerMin);
				vec3 cellHalfSize = 0.5 * vec3(cellCornerMax - cellCornerMin);

				// gather lights
				int colorIndex = 0;
				int byteShift = 0;
				for (int i = 0; i < lightCount && colorIndex < 4; i++)
				{
					vec3 lightPosition = (view * lights[i].m_position).xyz;
					vec3 closest = cellCenter + clamp(lightPosition - cellCenter, -cellHalfSize, cellHalfSize);
					vec3 delta = lightPosition - closest;
					if (dot(delta, delta) <= lights[i].m_range * lights[i].m_range)
					{
						clusterLightMask[colorIndex] &= ~(0xFF << byteShift);
						clusterLightMask[colorIndex] |= (i << byteShift);
						byteShift += 8;
						if (byteShift >= 32)
						{
							byteShift = 0;
							colorIndex++;
						}
					}
				}
			}
			imageStore(lightClusters, clusterIndex, clusterLightMask);
		}
		
	};
}