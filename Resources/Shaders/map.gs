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

uniform ivec4 exclusion;

float valid()
{
	int height = exclusion.x;
	int size = exclusion.y;
	ivec2 minCorner = ivec2(exclusion.z - size, exclusion.w - size);
	ivec2 maxCorner = ivec2(exclusion.z + size, exclusion.w + size);
	int primitiveIDIn = gl_PrimitiveIDIn / 2;
	ivec2 face =  ivec2(primitiveIDIn % height, primitiveIDIn / height);
	
	if(face.x >= minCorner.x && face.x <= maxCorner.x && face.y >= minCorner.y && face.y <= maxCorner.y)
		return -1;
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