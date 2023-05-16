SkeletonBB
{	
	uniform :
	{
		model : "mat4";
		skeletonPose : "mat4 array32";
	};
	
	includes :
	{
		#version 420
		
		layout(std140, binding = 0) uniform GlobalMatrices
		{
			mat4 view;
			mat4 projection;
			vec4 cameraPosition;
		};
	};
	vertex :
	{
		#define MAX_SKELETON_BONE 32

		// input
		layout(location = 0) in ivec2 segmentIndex;
		layout(location = 1) in float segmentRadius;

		uniform mat4 model;			// model matrix (has to be present at this location)
		uniform mat4 skeletonPose[MAX_SKELETON_BONE]; 	// bone matrix

		// output
		out vec4 segmentEnd;
		out float radius;
		out mat4 PVM;

		// program
		void main()
		{
			if(segmentIndex.x == -1)
			{
				segmentEnd = vec4(0,0,0,0);
				gl_Position = vec4(0,0,0,0);
				radius = 0;
				PVM = projection * view * model;
			}
			else 
			{
				PVM = projection * view * model;
				radius = segmentRadius;
				segmentEnd = skeletonPose[segmentIndex.y][3];
				gl_Position = skeletonPose[segmentIndex.x][3];
			}
		}
	};
	
	geometry :
	{
		#define M_PI 		3.1415926535897932384626433832795
		#define QUADRATURE 	7
		#define QUADRATURE_S 2
		#define RADIUS_FACTOR 1.07

		layout(points) in;
		layout(triangle_strip, max_vertices = 200) out;

		in vec4 segmentEnd[];
		in float radius[];
		in mat4 PVM[];

		out vec2 barycentricCoord;

		//	draw simple primitives
		void drawFace(vec3 center, vec3 axis1, vec3 axis2, float size1, float size2)
		{	
			gl_Position = PVM[0] * vec4(center - (size1/2.f)*axis1 - (size2/2.f)*axis2 , 1.0);
			barycentricCoord = vec2(0.0 , 0.0);
			EmitVertex();
			
			gl_Position = PVM[0] * vec4(center + (size1/2.f)*axis1 - (size2/2.f)*axis2 , 1.0);
			barycentricCoord = vec2(0.0 , 1.0);
			EmitVertex();
			
			gl_Position = PVM[0] * vec4(center - (size1/2.f)*axis1 + (size2/2.f)*axis2 , 1.0);
			barycentricCoord = vec2(1.0 , 0.0);
			EmitVertex();
			
			gl_Position = PVM[0] * vec4(center + (size1/2.f)*axis1 + (size2/2.f)*axis2 , 1.0);
			barycentricCoord = vec2(0.0 , 0.0);
			EmitVertex();
			
			EndPrimitive();
		}
		void drawQuad(vec3 v1, vec3 v2, vec3 v3, vec3 v4)
		{	
			gl_Position = PVM[0] * vec4(v1 , 1.0);
			barycentricCoord = vec2(0.0 , 0.0);
			EmitVertex();
			
			gl_Position = PVM[0] * vec4(v2 , 1.0);
			barycentricCoord = vec2(1.0 , 0.0);
			EmitVertex();
			
			gl_Position = PVM[0] * vec4(v3 , 1.0);
			barycentricCoord = vec2(0.0 , 1.0);
			EmitVertex();
			
			gl_Position = PVM[0] * vec4(v4 , 1.0);
			barycentricCoord = vec2(0.0 , 0.0);
			EmitVertex();
			
			EndPrimitive();
		}

		//	complex prilmitive draw
		void drawCylinder(vec3 begin, vec3 end)
		{
			vec3 center = 0.5 * (begin + end);
			vec3 axisRevolution = normalize(begin - end);
			vec3 normal = normalize(cross(vec3(0.0 , 0.0 , 1.0), axisRevolution));
			vec3 tangent = normalize(cross(normal, axisRevolution));
			float height = length(end - begin);
			vec3 tmp;
			
			vec3 previous = center + RADIUS_FACTOR * radius[0] * normal;
			for(int i = 1; i < QUADRATURE + 1; i++)
			{
				tmp = center + RADIUS_FACTOR * radius[0] * cos(2 * M_PI * i / QUADRATURE) * normal + RADIUS_FACTOR * radius[0] * sin(2 * M_PI * i / QUADRATURE) * tangent;
				drawFace(0.5 * (tmp + previous), axisRevolution, normalize(tmp - previous), height, length(tmp - previous));
				previous = tmp;
			}
		}
		void drawHemisphere(vec3 center, vec3 direction, vec3 normal)
		{
			vec3 tangent = normalize(cross(normal, direction));
			
			vec3 v1, v2, v3, v4;
			for(int j = 0; j < QUADRATURE_S; j++)
			{
				v4 = center + 
					RADIUS_FACTOR * radius[0] * cos(M_PI / 2 * j / QUADRATURE_S) * normal +
					RADIUS_FACTOR * radius[0] * sin(M_PI / 2 * j / QUADRATURE_S) * direction;
					
				v3 = center + 
					RADIUS_FACTOR * radius[0] * cos(M_PI / 2 * (j + 1) / QUADRATURE_S) * normal +
					RADIUS_FACTOR * radius[0] * sin(M_PI / 2 * (j + 1) / QUADRATURE_S) * direction;
						
				for(int i = 1; i < QUADRATURE+1; i++)
				{
					// compute new vectors
					v1 = center + 
						RADIUS_FACTOR * radius[0] * cos(2 * M_PI * i / QUADRATURE) * cos(M_PI / 2 * j / QUADRATURE_S) * normal + 
						RADIUS_FACTOR * radius[0] * sin(2 * M_PI * i / QUADRATURE) * cos(M_PI / 2 * j / QUADRATURE_S) * tangent + 
						RADIUS_FACTOR * radius[0] * sin(M_PI / 2 * j / QUADRATURE_S) * direction;
						
					v2 = center + 
						RADIUS_FACTOR * radius[0] * cos(2 * M_PI * i / QUADRATURE) * cos(M_PI / 2 * (j + 1) / QUADRATURE_S) * normal + 
						RADIUS_FACTOR * radius[0] * sin(2 * M_PI * i / QUADRATURE) * cos(M_PI / 2 * (j + 1) / QUADRATURE_S) * tangent + 
						RADIUS_FACTOR * radius[0] * sin(M_PI / 2 * (j + 1) / QUADRATURE_S) * direction;
						
					drawQuad(v2, v1, v3, v4);

					//	vector shift
					v4 = v1;
					v3 = v2;
				}
			}
		}

		//	entry point
		void main()
		{
			if(gl_in[0].gl_Position.xyz != segmentEnd[0].xyz)
			{
				drawCylinder(gl_in[0].gl_Position.xyz, segmentEnd[0].xyz);
				drawHemisphere(gl_in[0].gl_Position.xyz, normalize(gl_in[0].gl_Position.xyz - segmentEnd[0].xyz), normalize(cross(vec3(0.0 , 0.0 , 1.0), gl_in[0].gl_Position.xyz - segmentEnd[0].xyz)));
				drawHemisphere(segmentEnd[0].xyz, normalize(segmentEnd[0].xyz - gl_in[0].gl_Position.xyz), normalize(cross(vec3(0.0 , 0.0 , 1.0), gl_in[0].gl_Position.xyz - segmentEnd[0].xyz)));	
			}
			else
			{
				drawHemisphere(gl_in[0].gl_Position.xyz, vec3(0.0 , 0.0 ,  1.0), vec3(0.0 , 1.0 , 0.0));
				drawHemisphere(gl_in[0].gl_Position.xyz, vec3(0.0 , 0.0 , -1.0), vec3(0.0 , 1.0 , 0.0));
			} 
		}
	};
	
	fragment :
	{
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
	};
}