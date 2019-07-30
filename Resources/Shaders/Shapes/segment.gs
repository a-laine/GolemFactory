#version 330

layout(points) in;
layout(line_strip, max_vertices = 2) out;

uniform mat4 model;
uniform mat4 view; 		// view matrix
uniform mat4 projection;// projection matrix

uniform vec3 vector = vec3(0.0 , 0.0 , 0.0);

//	program
void main()
{
	//	create alias
	vec3 p1 = (model * gl_in[0].gl_Position).xyz;
	vec3 p2 = p1 + vector;
	
	//	draw segment
	gl_Position = projection * view * vec4(p1 , 1.0);
	EmitVertex();
	gl_Position = projection * view * vec4(p2, 1.0);
	EmitVertex();
	EndPrimitive();
}

