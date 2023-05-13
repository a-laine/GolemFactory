Wired
{
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		overrideColor : "vec4";
	};
	
	vertex :
	{
		#version 330

		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 vertexcolor;

		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

		// output
		out vec4 lightDirectionCameraSpace_gs;
		out vec4 fragmentNormal_gs;
		out vec4 fragmentColor_gs;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			gl_Position = projection * view * model * position;
			fragmentNormal_gs = view * model * normal;
			fragmentColor_gs = vertexcolor;
			
			vec4 eyeDirectionCameraSpace = - (view * model * position);
			vec4 lightPositionCameraSpace = view * lightCoordinateWorldSpace;
			lightDirectionCameraSpace_gs = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	
	geometry : "wired.gs";
	fragment : "wired.fs";
	
}
