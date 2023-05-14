DefaultTextured
{	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
	};
	
	textures : [
		{
			name : "albedo";
			resource : "PolygonDungeon/Dungeons_Texture_01.png";
		},{
			name : "emmisive";
			resource : "PolygonDungeon/Emmisive_01.png";
		}
	];
	
	vertex :
	{
		#version 330

		// input
		layout(location = 0) in vec4 position;
		layout(location = 1) in vec4 normal;
		layout(location = 2) in vec4 uv;

		uniform mat4 model; 	// model matrix (has to be present at this location)
		uniform mat4 view; 		// view matrix
		uniform mat4 projection;// projection matrix

		// output
		out vec4 lightDirectionCameraSpace;
		out vec4 fragmentNormal;
		out vec4 fragmentUv;

		vec4 lightCoordinateWorldSpace = vec4(1000,200,1500,1);

		// program
		void main()
		{
			gl_Position = (projection * view * model) * position;
			fragmentNormal = view * model * normal;
			fragmentUv = uv;//vec4(uv.x, 1.0 - uv.y, uv.z, uv.w);
			
			vec4 eyeDirectionCameraSpace = -(view * model * position);
			vec4 lightPositionCameraSpace = view * lightCoordinateWorldSpace;
			lightDirectionCameraSpace = lightPositionCameraSpace + eyeDirectionCameraSpace;
		}
	};
	fragment :
	{
		#version 330
		
		//	uniform
		uniform sampler2D albedo;   //texture unit 0
		uniform sampler2D emmisive; //texture unit 1

		// input
		in vec4 lightDirectionCameraSpace;
		in vec4 fragmentNormal;
		in vec4 fragmentUv;

		// output
		layout (location = 0) out vec4 fragColor;

		// program
		void main()
		{
			vec4 albedoColor = texture(albedo, vec2(fragmentUv.x, fragmentUv.y));
			vec4 emmisiveColor = texture(emmisive, fragmentUv.xy);
			float costeta = clamp( dot(normalize(fragmentNormal), normalize(lightDirectionCameraSpace)), 0,1 );
			
			//fragColor = fragmentUv ;
			//if(fragmentUv.x > 1.0 ||fragmentUv.y > 1.0 || fragmentUv.x < 0.0 ||fragmentUv.y < 0.0) fragColor.z = 1.0;
			
			fragColor = albedoColor * (0.4*costeta + 0.6) + emmisiveColor;
		}
	};
} 