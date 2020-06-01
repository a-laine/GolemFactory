#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// input
in vec3 lightDirectionCameraSpace_gs[];
in vec3 normal_gs[];
in vec3 color_gs[];

// output
out vec3 lightDirectionCameraSpace_fs;
out vec3 normal_fs;
out vec3 color_fs;
out float valid_fs;

uniform int listsize = 0;
uniform int list[256];

float valid()
{
	for(int i = 0; i < min(listsize, 256); i++)
	{
		if(gl_PrimitiveIDIn == list[i])
			return -1;
	}
	return 0;
}

void main()
{
	valid_fs = valid();
	
	gl_Position = gl_in[0].gl_Position;
	normal_fs = normal_gs[0];
	color_fs = color_gs[0];
	lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[0];
	EmitVertex();
	
	gl_Position = gl_in[1].gl_Position;
	normal_fs = normal_gs[1];
	color_fs = color_gs[1];
	lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[1];
	EmitVertex();
	
	gl_Position = gl_in[2].gl_Position;
	normal_fs = normal_gs[2];
	color_fs = color_gs[2];
	lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[2];
	EmitVertex();
	
	EndPrimitive();
}