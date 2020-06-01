#version 330

// input
in vec3 lightDirectionCameraSpace_fs;
in vec3 normal_fs;
in vec3 color_fs;
in float valid_fs;

// uniform
uniform vec3 overrideColor = vec3(-1.0 , 0.0 , 0.0);

// output
layout (location = 0) out vec3 fragColor;

// program
void main()
{
	if(valid_fs < 0) discard;
	vec3 color = color_fs;
	
	if (overrideColor.x >= 0.0)
		color = overrideColor;

	float costeta = clamp( dot(normalize(normal_fs), normalize(lightDirectionCameraSpace_fs)), 0,1 );
	fragColor = color * (0.4*costeta + 0.6);
}


