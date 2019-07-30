#version 330

// input
in vec3 barycentricCoord;
in vec3 fragmentColor;

uniform int wired = 0;
uniform vec3 overrideColor = vec3(-1.0 , 0.0 , 0.0);

// output
layout (location = 0) out vec3 fragColor;


// program
float edgeFactor()
{
    vec3 d = fwidth(barycentricCoord);
    vec3 a3 = smoothstep(vec3(0.0), d * 0.9 , barycentricCoord);
    return min(min(a3.x, a3.y), a3.z);
}

void main()
{
	vec3 color = fragmentColor;
	if (overrideColor.x >= 0.0)
		color = overrideColor;
	if(wired != 0)
	{
		if(edgeFactor() < 1.0) fragColor = color;
		else discard;	
	}
	else fragColor = color;
}


