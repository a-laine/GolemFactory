Point
{	
	uniform :
	{
		matrixArray : "struct array32";
		
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
		layout(line_strip, max_vertices = 8) out;

		uniform mat4 matrixArray[2];

		in vec4 fragmentColor[];
		out vec4 fragmentColor1;

		//	draw functions
		void drawSegment(vec3 p1, vec3 p2)
		{
			gl_Position = projection * view * vec4(p1 , 1.0);
			fragmentColor1 = fragmentColor[0];
			EmitVertex();
			gl_Position = projection * view * vec4(p2, 1.0);
			fragmentColor1 = fragmentColor[0];
			EmitVertex();
			EndPrimitive();
		}

		//	entry point
		void main()
		{
			//	create alias
			vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
			vec3 up = vec3(view[0][1], view[1][1], view[2][1]);
			vec3 p = (matrixArray[0] * gl_in[0].gl_Position).xyz;
			float size = 0.2;
			
			//	draw 4 segment (a star)
			drawSegment(p - up * size, p + up * size);
			drawSegment(p - right * size, p + right * size);
			drawSegment(p - normalize(right + up) * size, p + normalize(right + up) * size);
			drawSegment(p - normalize(right - up) * size, p + normalize(right - up) * size);
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