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
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 vertexcolor;

		//in vec2 texture;
		//in vec3 weight;

		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

		uniform vec4 wind;

		// output
		out vec4 lightDirectionCameraSpace;
		out vec4 fragmentNormal;
		out vec4 fragmentColor;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			vec4 tmp = model * position;
			
			gl_Position = projection * view * (tmp + position.z * wind);
			fragmentNormal = view * model * normal;
			fragmentColor = 2.0 * vertexcolor;
			
			vec4 eyeDirectionCameraSpace = - (view * tmp);
			vec4 lightPositionCameraSpace = view * lightCoordinateWorldSpace;
			lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	
	fragment : 
	{
		#version 330

		// input
		in vec4 lightDirectionCameraSpace;
		in vec4 fragmentNormal;
		in vec4 fragmentColor;

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