greenGrass {	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
	};
	
	vertex :
	{
		#version 330

		// input
		layout(location = 0) in vec3 position;

		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

		// program
		void main()
		{
			gl_Position = projection * view * model * vec4(position, 1.0);
		}
	};
	
	fragment :
	{
		#version 330

		// output
		layout (location = 0) out vec3 fragColor;


		// program
		void main()
		{
			fragColor = 3.0 * vec3(0.0, 0.146154, 0.001127);
		}
	};
}