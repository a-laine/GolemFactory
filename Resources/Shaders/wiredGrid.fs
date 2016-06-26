#version 330

// input
//uniform vec3 lightDirection_cameraSpace;
//uniform sampler2D tex;

//in vec2 textureCoord;
//in vec4 fragmentColor;
//in vec3 fragmentNormal;

// output
layout (location = 0) out vec4 fragColor;

in vec3 barycentricCoord;

// program
void main()
{
	if(barycentricCoord.x < 0.1)
		fragColor = vec4(0.937,0.894,0.690,1.0);
	else discard;
		//fragColor = 0.3*vec4(0.937,0.894,0.690,1.0);
	//fragColor = fragmentColor * dot(fragmentNormal,lightDirection_cameraSpace) * texture2D(tex,textureCoord);
}


