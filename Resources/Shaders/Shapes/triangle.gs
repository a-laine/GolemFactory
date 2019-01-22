#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

uniform mat4 model;
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix

uniform vec3 vector1 = vec3(0.0 , 0.0 , 0.0);
uniform vec3 vector2 = vec3(0.0 , 0.0 , 0.0);

// output
out vec3 barycentricCoord;


//	program
void main()
{
	//	create alias
	vec3 p1 = (model * gl_in[0].gl_Position).xyz;
	vec3 p2 = p1 + vector1;
	vec3 p3 = p1 + vector2;
	
	//	draw segment
	gl_Position = projection * view * vec4(p1 , 1.0);
	barycentricCoord = vec3(1.0 , 0.0 , 0.0);
	EmitVertex();
	gl_Position = projection * view * vec4(p2, 1.0);
	barycentricCoord = vec3(0.0 , 1.0 , 0.0);
	EmitVertex();
	gl_Position = projection * view * vec4(p3, 1.0);
	barycentricCoord = vec3(0.0 , 0.0 , 1.0);
	EmitVertex();
	EndPrimitive();
}

