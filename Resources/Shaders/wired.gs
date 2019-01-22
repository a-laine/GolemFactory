#version 330

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

// input
in vec3 lightDirectionCameraSpace_gs[];
in vec3 fragmentNormal_gs[];
in vec3 fragmentColor_gs[];

// output
out vec3 lightDirectionCameraSpace_fs;
out vec3 fragmentNormal_fs;
out vec3 fragmentColor_fs;
out vec3 barycentricCoord;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	fragmentNormal_fs = fragmentNormal_gs[0];
	fragmentColor_fs = fragmentColor_gs[0];
	lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[0];
	barycentricCoord = vec3(1.0 , 0.0 , 0.0);
	EmitVertex();
	
	gl_Position = gl_in[1].gl_Position;
	fragmentNormal_fs = fragmentNormal_gs[1];
	fragmentColor_fs = fragmentColor_gs[1];
	lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[1];
	barycentricCoord = vec3(0.0 , 1.0 , 0.0);
	EmitVertex();
	
	gl_Position = gl_in[2].gl_Position;
	fragmentNormal_fs = fragmentNormal_gs[2];
	fragmentColor_fs = fragmentColor_gs[2];
	lightDirectionCameraSpace_fs = lightDirectionCameraSpace_gs[2];
	barycentricCoord = vec3(0.0 , 0.0 , 1.0);
	EmitVertex();
	
	EndPrimitive();
}