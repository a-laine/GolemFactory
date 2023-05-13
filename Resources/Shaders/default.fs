#version 330

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


