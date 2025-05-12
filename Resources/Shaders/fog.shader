Fog
{
	renderQueue : 3000;//transparent
	transparent : true;
	faceCulling : false;

	uniform :
	{
		matrixArray : "mat4";
		tintColor : "vec4";
	};
	
	textures : [
		{
			name : "density";
			resource : "WhiteTexture.png";
		}
	];
	
	properties : [
		{
			name : "tintColor",
			type : "color",
			default : [ 0.0 , 0.0 , 0.0 , 0.5 ]
		}
	];
	
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
		layout(location = 2) in vec4 uv;

		uniform mat4 matrixArray[2];
		
		// output
		//out vec4 fragmentNormal;
		out vec4 fragmentUv;

		// program
		void main()
		{
			mat4 model = matrixArray[0];
			gl_Position = (projection * view * model) * position;
			fragmentUv = uv;
		}
	};
	fragment :
	{
		// textures
		uniform sampler2D density;   //sampler unit 0
		
		// input
		in vec4 fragmentUv;
		
		// material constants
		uniform vec4 tintColor = vec4(0.0 , 0.0 , 0.0 , 0.5);
		
		// output
		layout (location = 0) out vec4 fragColor;

		// program
		void main()
		{
			fragColor = tintColor;
			fragColor.a *= texture(density, fragmentUv.xy).r;
		}
	};
} 