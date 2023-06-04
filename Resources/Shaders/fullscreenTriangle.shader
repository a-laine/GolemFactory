FullscreenTriangle
{
	uniform :
	{
		alpha : "float"
	};

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
		
		// textures
		uniform sampler2D tex;   //texture unit 0
		
		// uniforms
		uniform float alpha = 1.0;
		
		// input
		in vec2 fragmentUv;
		
		// output
		layout (location = 0) out vec4 fragColor;
		
		void main()
		{
			vec4 color = texture(tex, fragmentUv);
			color.w = alpha;
			fragColor = color;
		}
	};
}