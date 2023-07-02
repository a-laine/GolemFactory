Debug
{	
	uniform :
	{
		matrixArray : "struct array32";
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
		layout(location = 1) in vec4 vertexcolor;

		// output
		out vec4 fragmentColor;

		uniform mat4 matrixArray[2];

		// program
		void main()
		{
			mat4 model = matrixArray[0];
			
			gl_Position = projection * view * model * position;
			fragmentColor = vertexcolor;
		}
	};
	fragment : 
	{
		// input
		in vec4 fragmentColor;

		// output
		layout (location = 0) out vec4 fragColor;

		// program
		void main()
		{
			fragColor = fragmentColor;
		}
	};
}