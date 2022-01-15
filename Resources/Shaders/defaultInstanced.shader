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
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec3 normal;
		layout(location = 2) in vec3 vertexcolor;

		//in vec2 texture;
		//in vec3 weight;

		uniform mat4 model[MAX_INSTANCE]; 	// model matrix
		uniform mat4 view; 					// view matrix
		uniform mat4 projection;			// projection matrix

		// output
		out vec3 lightDirectionCameraSpace;
		out vec3 fragmentNormal;
		out vec3 fragmentColor;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			gl_Position = projection * view * model[gl_InstanceID] * vec4(position, 1.0);
			fragmentNormal = (view * model[gl_InstanceID] * vec4(normal, 0.0)).xyz;
			fragmentColor = 2.0 * vertexcolor;
			
			vec3 eyeDirectionCameraSpace = - ( view * model[gl_InstanceID] * vec4(position, 1.0)).xyz;
			vec3 lightPositionCameraSpace = (view * lightCoordinateWorldSpace).xyz;
			lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	
	fragment : "default.fs";
}