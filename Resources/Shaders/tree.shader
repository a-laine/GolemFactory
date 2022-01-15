Tree
{
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		wind : "vec4";
	};
	
	vertex : 
	{
		#version 330

		// input
		layout(location = 0) in vec3 position;
		layout(location = 1) in vec3 normal;
		layout(location = 2) in vec3 vertexcolor;

		//in vec2 texture;
		//in vec3 weight;

		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

		uniform vec4 wind;

		// output
		out vec3 lightDirectionCameraSpace;
		out vec3 fragmentNormal;
		out vec3 fragmentColor;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			vec4 tmp = model * vec4(position, 1.0) ;
			
			gl_Position = projection * view * (tmp + position.z * wind);
			fragmentNormal = (view * model * vec4(normal,0.0)).xyz;
			fragmentColor = 2.0 * vertexcolor;
			
			vec3 eyeDirectionCameraSpace = - ( view * tmp).xyz;
			vec3 lightPositionCameraSpace = (view * lightCoordinateWorldSpace).xyz;
			lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	
	fragment : 
	{
		#version 330

		// input
		in vec3 lightDirectionCameraSpace;
		in vec3 fragmentNormal;
		in vec3 fragmentColor;

		// output
		layout (location = 0) out vec3 fragColor;


		// program
		void main()
		{
			float costeta = clamp( dot(normalize(fragmentNormal), normalize(lightDirectionCameraSpace)), 0,1 );
			fragColor = fragmentColor * (0.5*costeta +0.9);
		}
	};
}