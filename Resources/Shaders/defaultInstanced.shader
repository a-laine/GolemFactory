Default
{
	uniform :
	{
		model : "mat4 array32";
		view : "mat4";
		projection : "mat4";
	};
	
	vertex :
	{
		#version 330
		#define MAX_INSTANCE 32

		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 vertexcolor;

		//in vec2 texture;
		//in vec3 weight;

		uniform mat4 model[MAX_INSTANCE]; 	// model matrix
		uniform mat4 view; 					// view matrix
		uniform mat4 projection;			// projection matrix

		// output
		out vec4 lightDirectionCameraSpace;
		out vec4 fragmentNormal;
		out vec4 fragmentColor;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			gl_Position = projection * view * model[gl_InstanceID] * position;
			fragmentNormal = view * model[gl_InstanceID] * normal;
			fragmentColor = 2.0 * vertexcolor;
			
			vec4 eyeDirectionCameraSpace = - (view * model[gl_InstanceID] * position);
			vec4 lightPositionCameraSpace = view * lightCoordinateWorldSpace;
			lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	
	fragment : "default.fs";
}