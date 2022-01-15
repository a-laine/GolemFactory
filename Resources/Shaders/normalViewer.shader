NormalViewer{
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		overrideColor : "vec3";
	};
	
	vertex : 
	{
			#version 330

			// input
			layout(location = 0) in vec3 position;
			layout(location = 1) in vec3 normal;
			layout(location = 2) in vec3 vertexcolor;

			uniform mat4 model; 	// model matrix (has to be present at this location)
			uniform mat4 view; 		// view matrix
			uniform mat4 projection;// projection matrix

			// output
			out vec4 delta_gs;

			// program
			void main()
			{
				gl_Position = projection * view * model * vec4(position, 1.0);
				delta_gs = projection * view * model * vec4(position + 0.1 * normal, 1.0);
			}
	};
	
	geometry : 
	{
		#version 330

		layout(triangles) in;
		layout(triangle_strip, max_vertices = 12) out;

		// input
		in vec4 delta_gs[];

		// output
		out vec3 fragmentColor_fs;
		out vec3 barycentricCoord;

		vec3 normalColor = vec3(1.0,1.0,1.0);


		void drawNormal(int p1, int p2)
		{
			gl_Position = gl_in[p1].gl_Position;
			fragmentColor_fs = normalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = delta_gs[p1];
			fragmentColor_fs = normalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[p2].gl_Position;
			fragmentColor_fs = normalColor;
			barycentricCoord = vec3(0.0 , 1.0 , 0.0);
			EmitVertex();
			
			EndPrimitive();
		}

		void drawFaceNormal()
		{
			gl_Position = 0.3 * (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position);
			fragmentColor_fs = normalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = 0.3 * (delta_gs[0] + delta_gs[1] + delta_gs[2]);
			fragmentColor_fs = normalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[0].gl_Position;
			fragmentColor_fs = normalColor;
			barycentricCoord = vec3(0.0 , 1.0 , 0.0);
			EmitVertex();
			
			EndPrimitive();
		}

		void main()
		{
			drawNormal(0 , 1);
			drawNormal(1 , 2);
			drawNormal(2 , 0);
			drawFaceNormal();
		}
	};
	
	fragment : 
	{
		#version 330

		// input
		in vec3 fragmentColor_fs;
		in vec3 barycentricCoord;
		 
		uniform vec3 overrideColor = vec3(-1.0 , 0.0 , 0.0);

		// output
		layout (location = 0) out vec3 fragColor;


		// program
		float edgeFactor()
		{
			vec3 d = fwidth(barycentricCoord);
			vec3 a3 = smoothstep(vec3(0.0), d * 0.9 , barycentricCoord);
			return min(a3.x, a3.y);
		}

		void main()
		{
			vec3 color = fragmentColor_fs;
			if (overrideColor.x >= 0.0)
				color = overrideColor;
				
			if(edgeFactor() < 1.0) fragColor = color;
			else discard;
		}
	};
}