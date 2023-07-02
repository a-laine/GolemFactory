OcclusionResult
{
	vertex :
	{
		#version 430
		out vec2 fragmentUv;
		
		void main()
		{
			fragmentUv = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
			gl_Position = vec4(fragmentUv * vec2(2 , -2) + vec2(-1 , 1), 0 , 1);
		}
	};
	fragment :
	{
		#version 430
		#include "UniformBuffers.cginc"
		
		// textures
		uniform sampler2D tex;   //texture unit 0
		
		// input
		in vec2 fragmentUv;
		
		// output
		layout (location = 0) out vec4 fragColor;
		
		void main()
		{
			// compute depth in m
			
			float depth = -texture(tex, vec2(fragmentUv.x, fragmentUv.y)).r;
			float normalizedDepth = min(depth / occlusionResultCuttoff, 1.0);
			
			vec4 color = vec4(normalizedDepth);
			color.w = occlusionResultDrawAlpha;
			fragColor = color;
		}
	};
}