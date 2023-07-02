Fog
{	
	renderQueue : 3000;//transparent
	transparent : true;
	faceCulling : false;

	uniform :
	{
		matrixArray : "mat4";
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

		uniform mat4 matrixArray[2];
		
		// output
		out vec4 fragmentNormal;

		// program
		void main()
		{
			mat4 model = matrixArray[0];
			gl_Position = (projection * view * model) * position;
			fragmentNormal = view * model * normal;
		}
	};
	fragment :
	{
		// input
		in vec4 fragmentNormal;
		
		// output
		layout (location = 0) out vec4 fragColor;

		// program
		void main()
		{
			fragColor = vec4(0 , 0 , 0 , 0.5);
		}
	};
} 