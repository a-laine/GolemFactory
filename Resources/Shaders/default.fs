#version 330

// input
in vec3 lightDirectionCameraSpace;
in vec3 fragmentNormal;
in vec3 fragmentColor;

uniform vec3 overrideColor = vec3(-1.0 , 0.0 , 0.0);

// output
layout (location = 0) out vec3 fragColor;

// program
void main()
{
	vec3 color = fragmentColor;
	if (overrideColor.x >= 0.0)
		color = overrideColor;

	float costeta = clamp( dot(normalize(fragmentNormal), normalize(lightDirectionCameraSpace)), 0,1 );
	fragColor = color * (0.4*costeta + 0.6);
}


