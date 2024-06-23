skyboxRendering
{	
	renderQueue : 0;//background
	transparent : false;
	faceCulling : false;
	
	uniform :
	{
		matrixArray : "struct array32";
		skybox : "_globalSkybox";
	};
	
	includes : 
	{
		#version 430
	};
	vertex :
	{
		#include "UniformBuffers.cginc"
	
		// input
		layout(location = 0) in vec4 position;
		
		uniform mat4 matrixArray[2];
		
		// output
		out vec4 fragmentPosition;
		
		// program
		void main()
		{
			mat4 model = matrixArray[0];
			fragmentPosition = model * position;
			gl_Position = projection * view * fragmentPosition;
		}
	};
	fragment :
	{
		#include "UniformBuffers.cginc"
		
		// textures
		uniform samplerCube skybox;
		
		// images
		
		// input
		in vec4 fragmentPosition;
			
		// output
		layout (location = 0) out vec4 fragColor;
		
		// usefull
		//vec4 fragmentColor= vec4(0.0);
				
		// program
		void main()
		{
			vec4 viewDir = normalize(cameraPosition - fragmentPosition);
			//viewDir.y = -viewDir.y;
			//fragColor = viewDir;
			fragColor = texture(skybox, -viewDir.xyz);
			//fragColor.w = 1.0;
		}
	};
}