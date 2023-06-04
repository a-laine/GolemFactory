NormalViewer
{
	uniform :
	{
		matrixArray : "struct array32"
		
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
		layout(std140, binding = 3) uniform DebugShaderUniform
		{
			vec4 vertexNormalColor;
			vec4 faceNormalColor;
			float wireframeEdgeFactor;
			float occlusionResultDrawAlpha;
			float occlusionResultCuttoff;
		};
	};
	vertex : 
	{
		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 vertexcolor;
		
		#ifdef INSTANCING
			#define MAX_INSTANCE 32
			#define MAX_MATRIX (2 * MAX_INSTANCE)
			uniform mat4 matrixArray[MAX_MATRIX];
		#else
			uniform mat4 matrixArray[2];
		#endif

		// output
		out vec4 delta_gs;

		// program
		void main()
		{
			mat4 model = matrixArray[2 * gl_InstanceID];
			mat4 normalMatrix = matrixArray[2 * gl_InstanceID + 1];
			mat4 MVP = projection * view * model;
			
			gl_Position = MVP * position;
			float mag = vertexNormalColor.w;
			delta_gs = MVP * (position + 1.0 * normal);
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

		void drawNormal(int p1, int p2)
		{
			gl_Position = gl_in[p1].gl_Position;
			fragmentColor_fs = vertexNormalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = mix(gl_in[p1].gl_Position, delta_gs[p1], vertexNormalColor.w);
			fragmentColor_fs = vertexNormalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[p2].gl_Position;
			fragmentColor_fs = vertexNormalColor;
			barycentricCoord = vec3(0.0 , 1.0 , 0.0);
			EmitVertex();
			
			EndPrimitive();
		}

		void drawFaceNormal()
		{
			vec4 p = 0.33 *  (gl_in[0].gl_Position + gl_in[1].gl_Position + gl_in[2].gl_Position);
			vec4 pn = 0.33 * (delta_gs[0] + delta_gs[1] + delta_gs[2]);
			
			gl_Position = p;
			fragmentColor_fs = faceNormalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = mix(p, pn, faceNormalColor.w);
			fragmentColor_fs = faceNormalColor;
			barycentricCoord = vec3(1.0 , 0.0 , 0.0);
			EmitVertex();
			
			gl_Position = gl_in[0].gl_Position;
			fragmentColor_fs = faceNormalColor;
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