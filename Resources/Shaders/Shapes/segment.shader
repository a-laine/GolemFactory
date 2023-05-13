Segment
{	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		vector : "vec4";
		
		overrideColor : "vec4";
	};
	
	vertex :   "Shapes/point.vs";
	fragment : "Shapes/point.fs";
	
	geometry : 
	{
		#version 330

		layout(points) in;
		layout(line_strip, max_vertices = 2) out;

		uniform mat4 model;
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

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
}