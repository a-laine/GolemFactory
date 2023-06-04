Triangle
{
	uniform :
	{
		matrixArray : "struct array32";
		
		vector1 : "vec4";
		vector2 : "vec4";
		wired : "int";
		overrideColor : "vec4";
	};
	
	vertex :   "Shapes/point.vs";
	
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
	geometry : 
	{
		layout(points) in;
		layout(triangle_strip, max_vertices = 3) out;

		uniform mat4 matrixArray[2];

		uniform vec4 vector1 = vec3(0.0 , 0.0 , 0.0 , 0.0);
		uniform vec4 vector2 = vec3(0.0 , 0.0 , 0.0 , 0.0);

		// output
		out vec3 barycentricCoord;


		//	program
		void main()
		{
			//	create alias
			vec4 p1 = matrixArray[0] * gl_in[0].gl_Position;
			vec4 p2 = p1 + vector1;
			vec4 p3 = p1 + vector2;
			
			//	draw segment
			gl_Position = projection * view * vec4(p1 , 1.0);
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			gl_Position = projection * view * vec4(p2, 1.0);
			barycentricCoord = vec3(0.0 , 1.0 , 0.0);
			EmitVertex();
			gl_Position = projection * view * vec4(p3, 1.0);
			barycentricCoord = vec3(0.0 , 0.0 , 1.0);
			EmitVertex();
			EndPrimitive();
		}
	};
	
	fragment : 
	{
		// input
		in vec3 barycentricCoord;
		in vec4 fragmentColor;

		uniform int wired = 0;
		uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);

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
			vec4 color = fragmentColor;
			if (overrideColor.x >= 0.0)
				color = overrideColor;
			if(wired != 0)
			{
				if(edgeFactor() < 1.0) fragColor = color;
				else discard;	
			}
			else fragColor = color;
		}
	};
}