Map
{
	uniform :
	{
		matrixArray : "mat4";
		overrideColor : "vec4";
		exclusion : "ivec4";
	};
	
	includes :
	{
		#version 420
	};
	vertex :
	{
		#include "UniformBuffers.cginc"
		
		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 color;

		uniform mat4 matrixArray[2];
		uniform int listsize = 0;

		// output
		out vec4 lightDirectionCameraSpace_gs;
		out vec4 normal_gs;
		out vec4 color_gs;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			mat4 model = matrixArray[0];
			gl_Position = projection * view * model * position;
			normal_gs = view * model * normal;
			color_gs = color;
			
			vec4 eyeDirectionCameraSpace = - (view * model * position);
			vec4 lightPositionCameraSpace = view * lightCoordinateWorldSpace;
			lightDirectionCameraSpace_gs = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	geometry :
	{
		
		layout(triangles) in;
		layout(triangle_strip, max_vertices = 3) out;

		// input
		in vec4 lightDirectionCameraSpace_gs[];
		in vec4 normal_gs[];
		in vec4 color_gs[];

		// output
		out vec4 lightDirectionCameraSpace_fs;
		out vec4 normal_fs;
		out vec4 color_fs;
		out float valid_fs;

		uniform ivec4 exclusion = ivec4(-1 , 0 , 0 , 0);

		float valid()
		{
			int height = exclusion.x;
			int size = exclusion.y;
			ivec2 minCorner = ivec2(exclusion.z - size, exclusion.w - size);
			ivec2 maxCorner = ivec2(exclusion.z + size, exclusion.w + size);
			int primitiveIDIn = gl_PrimitiveIDIn / 2;
			ivec2 face =  ivec2(primitiveIDIn % height, primitiveIDIn / height);
			
			if(height < 0)
				return 0;
			if(face.x >= minCorner.x && face.x <= maxCorner.x && face.y >= minCorner.y && face.y <= maxCorner.y)
				return -1;
			return 0;
		}

		void main()
		{
			valid_fs = valid();
			
			gl_Position = gl_in[0].gl_Position;
			normal_fs = normal_gs[0];
			color_fs = color_gs[0];
			lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[0];
			EmitVertex();
			
			gl_Position = gl_in[1].gl_Position;
			normal_fs = normal_gs[1];
			color_fs = color_gs[1];
			lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[1];
			EmitVertex();
			
			gl_Position = gl_in[2].gl_Position;
			normal_fs = normal_gs[2];
			color_fs = color_gs[2];
			lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[2];
			EmitVertex();
			
			EndPrimitive();
		}
	};
	fragment :
	{
		// input
		in vec4 lightDirectionCameraSpace_fs;
		in vec4 normal_fs;
		in vec4 color_fs;
		in float valid_fs;

		// uniform
		uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);

		// output
		layout (location = 0) out vec4 fragColor;

		// program
		void main()
		{
			if(valid_fs < 0) discard;
			vec4 color = color_fs;
			
			if (overrideColor.x >= 0.0)
				color = overrideColor;

			float costeta = clamp( dot(normalize(normal_fs), normalize(lightDirectionCameraSpace_fs)), 0,1 );
			fragColor = color * (0.4*costeta + 0.6);
		}
	};
}