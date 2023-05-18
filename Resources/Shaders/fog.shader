Fog
{	
	renderQueue : 3000;//transparent
	transparent : true;
	faceCulling : false;

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
		layout(location = 1) in vec4 normal;

		uniform mat4 model; 	// model matrix (has to be present at this location)
		
		// output
		out vec4 fragmentNormal;

		// program
		void main()
		{
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