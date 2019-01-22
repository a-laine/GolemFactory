#version 330

// input
in vec3 lightDirectionCameraSpace_fs;
in vec3 fragmentNormal_fs;
in vec3 fragmentColor_fs;
in vec3 barycentricCoord;
 
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
	vec3 color = fragmentColor_fs;
	if (overrideColor.x >= 0.0)
		color = overrideColor;
		
	float costeta = clamp( dot(normalize(fragmentNormal_fs), normalize(lightDirectionCameraSpace_fs)), 0,1 );
	vec3 expectedColor = color * (0.7*costeta + 0.3);	

	if(edgeFactor() < 1.0) fragColor = expectedColor;
	else discard;
}


