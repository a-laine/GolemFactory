#version 330

//	uniform
uniform sampler2D texture0;
uniform vec4 color;
uniform int useTexture;

//	input
in vec2 textureCoord0;

//	output
layout (location = 0) out vec4 fragColor;

//	program
void main()
{
	if(useTexture != 0)
		fragColor = color * texture2D(texture0, textureCoord0);
	else fragColor = color;
}


