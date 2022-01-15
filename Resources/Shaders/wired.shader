Wired
{
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		overrideColor : "vec3";
	};
	
	vertex :
	{
		#version 330

		// input
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec3 normal;
		layout(location = 2) in vec3 vertexcolor;

		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

		// output
		out vec3 lightDirectionCameraSpace_gs;
		out vec3 fragmentNormal_gs;
		out vec3 fragmentColor_gs;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			gl_Position = projection * view * model * vec4(position, 1.0);
			fragmentNormal_gs = (view * model * vec4(normal, 0.0)).xyz;
			fragmentColor_gs = 1.0 * vertexcolor;
			
			vec3 eyeDirectionCameraSpace = - ( view * model * vec4(position, 1.0)).xyz;
			vec3 lightPositionCameraSpace = (view * lightCoordinateWorldSpace).xyz;
			lightDirectionCameraSpace_gs = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	
	geometry : "wired.gs";
	fragment : "wired.fs";
	
}
