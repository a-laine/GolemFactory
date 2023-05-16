Segment
{	
	uniform :
	{
		model : "mat4";
		vector : "vec4";
		overrideColor : "vec4";
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
		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 vertexcolor;

		// output
		out vec4 fragmentColor;


		// program
		void main()
		{
			gl_Position = position;
			fragmentColor = vertexcolor;
		}
	};
	geometry : 
	{
		layout(points) in;
		layout(line_strip, max_vertices = 2) out;

		uniform mat4 model;
		uniform vec4 vector = vec4(0.0 , 0.0 , 0.0 , 0.0);

		in vec4 fragmentColor[];
		out vec4 fragmentColor1;

		//	program
		void main()
		{
			//	create alias
			vec3 p1 = (model * gl_in[0].gl_Position).xyz;
			vec3 p2 = p1 + vector.xyz;
			
			//	draw segment
			gl_Position = projection * view * vec4(p1 , 1.0);
			fragmentColor1 = fragmentColor[0];
			EmitVertex();
			gl_Position = projection * view * vec4(p2, 1.0);
			fragmentColor1 = fragmentColor[0];
			EmitVertex();
			EndPrimitive();
		}
	};
	fragment : 
	{
		// input
		in vec4 fragmentColor1;

		uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);

		// output
		layout (location = 0) out vec4 fragColor;


		// program
		void main()
		{
			if (overrideColor.x >= 0.0)
				fragColor = overrideColor;
			else fragColor = fragmentColor1;
		}
	};
}