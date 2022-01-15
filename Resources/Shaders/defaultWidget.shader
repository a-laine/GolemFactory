DefaultWidget
{
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		color : "vec4";
		useTexture : "int"
	};
	
	vertex : 
	{
		#version 330

		// input
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec2 textures;

		//	uniform
		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

		//	output
		out vec2 textureCoord0;

		// program
		void main()
		{
			gl_Position = projection * view * model * vec4(position, 1.0);
			textureCoord0 = textures;
		}
	};
	
	fragment : 
	{
		#version 330

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