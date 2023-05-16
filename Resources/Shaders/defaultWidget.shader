DefaultWidget
{
	uniform :
	{
		model : "mat4";
		
		color : "vec4";
		useTexture : "int"
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
		layout(location = 1) in vec2 textures;

		//	uniform
		uniform mat4 model; 	// model matrix (has to be present at this location)

		//	output
		out vec2 textureCoord0;

		// program
		void main()
		{
			gl_Position = projection * view * model * position;
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