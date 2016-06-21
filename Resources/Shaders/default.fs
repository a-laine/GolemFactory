#version 330

// input
uniform vec3 lightDirection_cameraSpace;
uniform sampler2D tex;

in vec2 textureCoord;
in vec4 fragmentColor;
in vec3 fragmentNormal;

// output
layout (location = 0) out vec4 fragColor;


// program
void main()
{
    fragColor = vec3(1.0);
	fragColor = fragmentColor * dot(fragmentNormal,lightDirection_cameraSpace) * texture2D(tex,textureCoord);
}


