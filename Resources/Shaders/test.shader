Test
{
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
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
		layout (location = 0) in vec3 position;
		layout (location = 1) in vec3 normal;
		layout (location = 2) in vec2 texture;
		layout (location = 3) in vec3 weight;
		layout (location = 4) in vec4 vertexcolor;

		out vec2 texCoord;
		out vec3 vPosition;
		out mat4 vProjModMatrix;

		void main()
		{
			vProjModMatrix = gl_ModelViewProjectionMatrix;
			texCoord = texture;
			vPosition = position;
			
			//gl_Position = gl_ModelViewProjectionMatrix*vec4(position,1);
		}
	};
	control : 
	{
		layout(vertices = 3) out;

		in vec3 vPosition[];
		in mat4 vProjModMatrix[];

		out vec3 tcPosition[];
		out mat4 tcProjModMatrix[];

		layout (location = 0) uniform float tessFactor = 1.0;

		void main()
		{
			tcPosition[gl_InvocationID] = vPosition[gl_InvocationID];
			tcProjModMatrix[gl_InvocationID] = vProjModMatrix[gl_InvocationID];
			
			if (gl_InvocationID == 0)
			{
				gl_TessLevelInner[0] = tessFactor;
				gl_TessLevelOuter[0] = tessFactor;
				gl_TessLevelOuter[1] = tessFactor;
				gl_TessLevelOuter[2] = tessFactor;
			}
		}
	};
	evaluation : 
	{
		layout(triangles,equal_spacing,cw) in;

		in vec3 tcPosition[];
		in mat4 tcProjModMatrix[];

		//layout (location = 1) uniform float displacementFactor = 0;

		void main()
		{
			vec3 tePosition = gl_TessCoord.x*tcPosition[0] + 
							  gl_TessCoord.y*tcPosition[1] + 
							  gl_TessCoord.z*tcPosition[2];
			gl_Position = tcProjModMatrix[0]*vec4(tePosition, 1);
		}
	};
	geometry : 
	{
		layout(triangles) in;
		layout(triangle_strip, max_vertices = 3) out;

		void main()
		{
			gl_Position = gl_in[0].gl_Position; EmitVertex();
			gl_Position = gl_in[1].gl_Position; EmitVertex();
			gl_Position = gl_in[2].gl_Position; EmitVertex();
			EndPrimitive();
		}
	};
	fragment : 
	{
		in vec2 texCoord
		layout (location = 0) out vec4 fragColor;
		
		void main()
		{
			fragColor = vec4(1.0);//texture2D(tex,texCoord);
		}
	};
}
