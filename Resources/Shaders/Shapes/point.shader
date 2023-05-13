Point
{	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		overrideColor : "vec4";
	};
	
	vertex :   "Shapes/point.vs";
	fragment : "Shapes/point.fs";
	
	geometry : 
	{
		#version 330

		layout(points) in;
		layout(line_strip, max_vertices = 8) out;

		uniform mat4 model;
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

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
			vec3 p = (model * gl_in[0].gl_Position).xyz;
			float size = 0.2;
			
			//	draw 4 segment (a star)
			drawSegment(p - up * size, p + up * size);
			drawSegment(p - right * size, p + right * size);
			drawSegment(p - normalize(right + up) * size, p + normalize(right + up) * size);
			drawSegment(p - normalize(right - up) * size, p + normalize(right - up) * size);
		}
	};
}