#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 12) out;

// input
in vec4 delta_gs[];

// output
out vec3 fragmentColor_fs;
out vec3 barycentricCoord;

vec3 normalColor = vec3(1.0,1.0,1.0);


void drawNormal(int p1, int p2)
{
	gl_Position = gl_in[p1].gl_Position;
	fragmentColor_fs = normalColor;
	barycentricCoord = vec3(1.0 , 0.0 , 0.0);
	EmitVertex();
	
	gl_Position = delta_gs[p1];
	fragmentColor_fs = normalColor;
	barycentricCoord = vec3(1.0 , 0.0 , 0.0);
	EmitVertex();
	
	gl_Position = gl_in[p2].gl_Position;
	fragmentColor_fs = normalColor;
	barycentricCoord = vec3(0.0 , 1.0 , 0.0);
	EmitVertex();
	
	EndPrimitive();
}

void main()
{
	drawNormal(0 , 1);
	drawNormal(1 , 2);
	drawNormal(2 , 0);
}