DefaultWidget
{
	uniform :
	{
		matrixArray : "mat4";
		
		color : "vec4";
		useTexture : "int"
	};
	
	includes : 
	{
		#version 430
	};
	vertex : 
	{
		#include "UniformBuffers.cginc"
		
		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec2 textures;

		//	uniform
		uniform mat4 matrixArray[2];

		//	output
		out vec2 textureCoord0;

		// program
		void main()
		{
			gl_Position = projection * view * matrixArray[0] * position;
			textureCoord0 = textures;
		}
	};
	
	fragment : 
	{
		//	uniform
		uniform sampler2D texture0;
		uniform vec4 color;
		uniform int useTexture;

		//	input
		in vec2 textureCoord0;

		//	output
		layout (location = 0) out vec4 fragColor;

		//	program
		void main()
		{
			if(useTexture != 0)
				fragColor = color * texture(texture0, textureCoord0);
			else fragColor = color;
		}
	};	
}