#version 440

in vec2 texCoord;

layout (location = 0) out vec4 fragColor;

//layout (location = 0) uniform sampler2D tex;

void main()
{
	fragColor = vec4(1.0);//texture2D(tex,texCoord);
}


