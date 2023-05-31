Default
{	
	renderQueue : 1000;//opaque
	uniform :
	{
		matrixArray : "mat4";
		overrideColor : "vec4";
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
		layout(location = 2) in vec4 vertexcolor;

		uniform mat4 matrixArray[2];

		// output
		out vec4 lightDirectionCameraSpace;
		out vec4 fragmentNormal;
		out vec4 fragmentColor;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			mat4 model = matrixArray[0];
			gl_Position = projection * view * model * position;
			fragmentNormal = view * model * normal;
			fragmentColor = vertexcolor;
			
			vec4 eyeDirectionCameraSpace = -(view * model * position);
			vec4 lightPositionCameraSpace = view * lightCoordinateWorldSpace;
			lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	fragment : 
	{
		// input
		in vec4 lightDirectionCameraSpace;
		in vec4 fragmentNormal;
		in vec4 fragmentColor;

		uniform vec4 overrideColor = vec4(-1.0 , 0.0 , 0.0 , 0.0);

		// output
		layout (location = 0) out vec4 fragColor;

		// program
		void main()
		{
			vec4 color = fragmentColor;
			if (overrideColor.x >= 0.0)
				color = overrideColor;

			float costeta = clamp( dot(normalize(fragmentNormal), normalize(lightDirectionCameraSpace)), 0,1 );
			fragColor = color * (0.4*costeta + 0.6);
		}
	};
} 
