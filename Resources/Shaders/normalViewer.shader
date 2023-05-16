NormalViewer{
	uniform :
	{
		model : "mat4";
		
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

		uniform mat4 model; 	// model matrix (has to be present at this location)

		// output
		out vec4 delta_gs;

		// program
		void main()
		{
			gl_Position = projection * view * model * position;
			delta_gs = projection * view * model * (position + 0.1 * normal);
		}
	};
	geometry : 
	{
		layout(triangles) in;
		layout(triangle_strip, max_vertices = 12) out;

		// input
		in vec4 delta_gs[];

		// output
		out vec4 fragmentColor_fs;
		out vec3 barycentricCoord;

		vec4 normalVertexColor = vec4(0.0,0.0,1.0,1.0);
		vec4 normalFaceColor = vec4(0.0,1.0,1.0,1.0);


		void drawNormal(int p1, int p2)
		{
			gl_Position = gl_in[p1].gl_Position;
			fragmentColor_fs = normalVertexColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = delta_gs[p1];
			fragmentColor_fs = normalVertexColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[p2].gl_Position;
			fragmentColor_fs = normalVertexColor;
			barycentricCoord = vec3(0.0 , 1.0 , 0.0);
			EmitVertex();
			
			EndPrimitive();
		}

		void drawFaceNormal()
		{
			gl_Position = 0.3 * (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position);
			fragmentColor_fs = normalFaceColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = 0.3 * (delta_gs[0] + delta_gs[1] + delta_gs[2]);
			fragmentColor_fs = normalFaceColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[0].gl_Position;
			fragmentColor_fs = normalFaceColor;
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
		// input
		in vec4 fragmentColor_fs;
		in vec3 barycentricCoord;
		 
		uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);

		// output
		layout (location = 0) out vec4 fragColor;


		// program
		float edgeFactor()
		{
			vec3 d = fwidth(barycentricCoord);
			vec3 a3 = smoothstep(vec3(0.0), d * 0.9 , barycentricCoord);
			return min(a3.x, a3.y);
		}

		void main()
		{
			vec4 color = fragmentColor_fs;
			if (overrideColor.x >= 0.0)
				color = overrideColor;
				
			if(edgeFactor() < 1.0) fragColor = color;
			else discard;
		}
	};
}