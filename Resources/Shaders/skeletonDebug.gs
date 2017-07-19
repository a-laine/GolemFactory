#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 72) out;

in int boneID[];
in mat4 PVM[];

out vec3 fragmentColor;

void drawFace(vec3 center, vec3 axis1, vec3 axis2, float size1, float size2)
{	
	gl_Position = PVM[0] * vec4(center - (size1/2.f)*axis1 - (size2/2.f)*axis2 , 1.0);
	EmitVertex();
	
	gl_Position = PVM[0] * vec4(center + (size1/2.f)*axis1 - (size2/2.f)*axis2 , 1.0);
	EmitVertex();
	
	gl_Position = PVM[0] * vec4(center - (size1/2.f)*axis1 + (size2/2.f)*axis2 , 1.0);
	EmitVertex();
	
	gl_Position = PVM[0] * vec4(center + (size1/2.f)*axis1 + (size2/2.f)*axis2 , 1.0);
	EmitVertex();
	
	EndPrimitive();
}
void drawCube(vec3 center, float size, vec3 color)
{
	fragmentColor = color;
	drawFace(center - vec3(0.0 , 0.0 , size/2) , vec3(1.0 , 0.0 , 0.0) , vec3(0.0 , 1.0 , 0.0) , size , size);
	drawFace(center + vec3(0.0 , 0.0 , size/2) , vec3(1.0 , 0.0 , 0.0) , vec3(0.0 , 1.0 , 0.0) , size , size);
	
	drawFace(center - vec3(0.0 , size/2 , 0.0) , vec3(1.0 , 0.0 , 0.0) , vec3(0.0 , 0.0 , 1.0) , size , size);
	drawFace(center + vec3(0.0 , size/2 , 0.0) , vec3(1.0 , 0.0 , 0.0) , vec3(0.0 , 0.0 , 1.0) , size , size);
	
	drawFace(center - vec3(size/2 , 0.0 , 0.0) , vec3(0.0 , 1.0 , 0.0) , vec3(0.0 , 0.0 , 1.0) , size , size);
	drawFace(center + vec3(size/2 , 0.0 , 0.0) , vec3(0.0 , 1.0 , 0.0) , vec3(0.0 , 0.0 , 1.0) , size , size);
}
void drawCross(vec3 p1, vec3 p2, float size, vec3 color)
{
	fragmentColor = color;
	vec3 axis1 = vec3(0.0 , 1.0 , 0.0);
	vec3 axis2 = normalize(p2 - p1);
	vec3 axis3 = cross(axis1,axis2);
	
	drawFace(0.5*(p1 + p2), axis1, axis2, size, length(p2 - p1));
	drawFace(0.5*(p1 + p2), axis3, axis2, size, length(p2 - p1));
}

void main()
{
	float s = 10;
	drawCube(gl_in[0].gl_Position.xyz, s , vec3(0.5 , 0.5 , 1.0));
	if(boneID[0] != boneID[1] && boneID[1] != boneID[2] && boneID[0] != boneID[2])
	{
		drawCross(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, 0.25*s, vec3(1.0 , 0.0 , 0.0));
		drawCross(gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz, 0.25*s, vec3(1.0 , 0.0 , 0.0));
		drawCross(gl_in[2].gl_Position.xyz, gl_in[1].gl_Position.xyz, 0.25*s, vec3(1.0 , 0.0 , 0.0));
	}
	else if(boneID[0] != boneID[1])
		drawCross(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, 0.25*s, vec3(0.5 , 1.0 , 0.5));
}

