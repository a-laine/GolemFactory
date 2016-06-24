#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

out vec3 barycentricCoord;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	barycentricCoord = vec3(1.0,0.0,0.0);
	EmitVertex();
	
	gl_Position = gl_in[1].gl_Position;
	barycentricCoord = vec3(0.0,1.0,0.0);
	EmitVertex();
	
	gl_Position = gl_in[2].gl_Position;
	barycentricCoord = vec3(0.0,0.0,1.0);
	EmitVertex();
	
	EndPrimitive();
}