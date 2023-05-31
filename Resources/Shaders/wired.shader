Wired
{
	renderQueue : 1000;//opaque
	transparent : true;
	faceCulling : false;
	
	uniform :
	{
		matrixArray : "mat4";
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
		layout(std140, binding = 1) uniform EnvironementLighting
		{
			vec4 m_backgroundColor;
			vec4 m_ambientColor;
			vec4 m_directionalLightDirection;
			vec4 m_directionalLightColor;
		};
	};
	vertex : 
	{
		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;

		uniform mat4 matrixArray[2];

		// output
		out vec4 fragmentNormal_gs;

		// program
		void main()
		{
			gl_Position = projection * view * matrixArray[0] * position;
			fragmentNormal_gs = normalize(matrixArray[1] * normal);
		}
	};
	geometry : 
	{
		layout(triangles) in;
		layout(triangle_strip, max_vertices = 3) out;

		// input
		in vec4 fragmentNormal_gs[];

		// output
		out vec4 fragmentNormal_fs;
		out vec4 fragmentPosition_fs;
		out vec3 barycentricCoord;

		void main()
		{
			gl_Position = gl_in[0].gl_Position;
			fragmentNormal_fs = fragmentNormal_gs[0];
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[1].gl_Position;
			fragmentNormal_fs = fragmentNormal_gs[1];
			barycentricCoord = vec3(0.0 , 1.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[2].gl_Position;
			fragmentNormal_fs = fragmentNormal_gs[2];
			barycentricCoord = vec3(0.0 , 0.0 , 1.0);
			EmitVertex();
			
			EndPrimitive();
		}
	};
	fragment : 
	{
		// input
		in vec4 fragmentNormal_fs;
		in vec3 barycentricCoord;

		uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);

		// output
		layout (location = 0) out vec4 fragColor;

		// program
		float edgeFactor()
		{
			vec3 d = fwidth(barycentricCoord);
			vec3 a3 = smoothstep(vec3(0.0), d * 0.9 , barycentricCoord);
			return min(min(a3.x, a3.y), a3.z);
		}
		void main()
		{
			vec4 color = overrideColor;
			float costeta = clamp( dot(normalize(fragmentNormal_fs), normalize(-m_directionalLightDirection)), 0, 1 );
			
			if(edgeFactor() < 1.0)
				fragColor = color * (0.4*costeta + 0.6);
			else discard;
		}
	};
}
