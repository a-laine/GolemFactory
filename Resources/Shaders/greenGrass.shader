greenGrass {	
	uniform :
	{
		model : "mat4";
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

		uniform mat4 model; 	// model matrix (has to be present at this location)

		// program
		void main()
		{
			gl_Position = projection * view * model * position;
		}
	};
	fragment :
	{
		// output
		layout (location = 0) out vec3 fragColor;


		// program
		void main()
		{
			fragColor = 3.0 * vec3(0.0, 0.146154, 0.001127);
		}
	};
}