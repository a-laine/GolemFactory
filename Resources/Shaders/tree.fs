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


