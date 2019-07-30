#version 330

// input
in vec2 barycentricCoord;

// output
layout (location = 0) out vec3 fragColor;


// program
float edgeFactor()
{
    vec2 d = fwidth(barycentricCoord);
    vec2 a3 = smoothstep(vec2(0.0), d * 0.9 , barycentricCoord);
    return min(a3.x, a3.y);
}

void main()
{
	if(edgeFactor() < 1.0) fragColor = vec3(0.5 , 0.0 , 0.0);
	else discard;
}


