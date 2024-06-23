atmosphericScattering
{
	computeShader : true;
	
	compute :
	{
		#version 430
		#include "UniformBuffers.cginc"
		
		layout(rgba8, binding = 0) uniform imageCube skybox;
		layout (local_size_x = 1 , local_size_y = 1 , local_size_z = 1) in;
		
		// Defines
		#define EPS                 1e-6
		#define PI                  3.14159265359
		#define INFINITY            1.0 / 0.0
		#define PLANET_RADIUS       6371000
		#define PLANET_CENTER       vec3(0, -PLANET_RADIUS, 0)
		#define ATMOSPHERE_HEIGHT   100000
		#define RAYLEIGH_HEIGHT     (ATMOSPHERE_HEIGHT * 0.08)
		#define MIE_HEIGHT          (ATMOSPHERE_HEIGHT * 0.012)

		// Coefficients
		#define C_RAYLEIGH          (vec3(5.802, 13.558, 33.100) * 1e-6)
		#define C_MIE               (vec3(3.996,  3.996,  3.996) * 1e-6)
		#define C_OZONE             (vec3(0.650,  1.881,  0.085) * 1e-6)

		#define ATMOSPHERE_DENSITY  1
		#define EXPOSURE            20
		
		vec4 facecolors[6] = vec4[]( vec4(1,0,0,1), vec4(0,1,0,1), vec4(0,0,1,1), vec4(1,1,0,1), vec4(0,1,1,1), vec4(1,0,1,1));
		vec3 transmittance = vec3(0,0,0);
		
		vec2 SphereIntersection (vec3 rayStart, vec3 rayDir, vec3 sphereCenter, float sphereRadius)
		{
			rayStart -= sphereCenter;
			float a = dot(rayDir, rayDir);
			float b = 2.0 * dot(rayStart, rayDir);
			float c = dot(rayStart, rayStart) - (sphereRadius * sphereRadius);
			float d = b * b - 4 * a * c;
			if (d < 0)
			{
				return vec2(-1);
			}
			else
			{
				d = sqrt(d);
				return vec2(-b - d, -b + d) / (2 * a);
			}
		}
		vec2 PlanetIntersection (vec3 rayStart, vec3 rayDir)
		{
			return SphereIntersection(rayStart, rayDir, PLANET_CENTER, PLANET_RADIUS);
		}
		vec2 AtmosphereIntersection (vec3 rayStart, vec3 rayDir)
		{
			return SphereIntersection(rayStart, rayDir, PLANET_CENTER, PLANET_RADIUS + ATMOSPHERE_HEIGHT);
		}
		float PhaseMie (float costh, float g = 0.85)
		{
			g = min(g, 0.9381);
			float k = 1.55 * g - 0.55 * g * g * g;
			float kcosth = k * costh;
			return (1 - k * k) / ((4 * PI) * (1 - kcosth) * (1 - kcosth));
		}
		float PhaseRayleigh (float costh) { return 3 * (1 + costh*costh) / (16 * PI); }
		float AtmosphereHeight (vec3 position) { return distance(position, PLANET_CENTER) - PLANET_RADIUS; }
		float DensityRayleigh (float h) { return exp(-max(0, h / RAYLEIGH_HEIGHT)); }
		float DensityMie (float h) { return exp(-max(0, h / MIE_HEIGHT)); }
		float DensityOzone (float h) { return max(0, 1 - abs(h - 25000.0) / 15000.0); }
		vec3 AtmosphereDensity (float h) { return vec3(DensityRayleigh(h), DensityMie(h), DensityOzone(h)); }
		
		vec3 IntegrateOpticalDepth (vec3 rayStart, vec3 rayDir)
		{
			//line 141
			vec2 intersection = AtmosphereIntersection(rayStart, rayDir);
			float rayLength = intersection.y;
			int sampleCount = 8;
			float stepSize = rayLength / sampleCount;
			vec3 opticalDepth = vec3(0);

			for (int i = 0; i < sampleCount; i++)
			{
				vec3 localPosition = rayStart + rayDir * ((i + 0.5) * stepSize);
				float localHeight  = AtmosphereHeight(localPosition);
				vec3 localDensity  = AtmosphereDensity(localHeight);
				opticalDepth += localDensity * stepSize;
			}
			return opticalDepth;
		}
		vec3 Absorb(vec3 opticalDepth)
		{
			// Note that Mie results in slightly more light absorption than scattering, about 10%
			return exp(-(opticalDepth.x * C_RAYLEIGH + opticalDepth.y * C_MIE * 1.1 + opticalDepth.z * C_OZONE) * ATMOSPHERE_DENSITY);
		}
		vec3 IntegrateScattering (vec3 rayStart, vec3 rayDir, float rayLength, vec3 lightDir, vec3 lightColor)
		{
			float rayHeight = AtmosphereHeight(rayStart);
			float sampleDistributionExponent = 1 + clamp(1 - rayHeight / ATMOSPHERE_HEIGHT, 0.0, 1.0) * 8;
			vec2 intersection = AtmosphereIntersection(rayStart, rayDir);
			rayLength = min(rayLength, intersection.y);
			if (intersection.x > 0)
			{
				// Advance ray to the atmosphere entry point
				rayStart += rayDir * intersection.x;
				rayLength -= intersection.x;
			}

			float  costh    = dot(rayDir, lightDir);
			float  phaseR   = PhaseRayleigh(costh);
			float  phaseM   = PhaseMie(costh);
			int sampleCount  = 64;
			vec3 opticalDepth = vec3(0);
			vec3 rayleigh     = vec3(0);
			vec3 mie          = vec3(0);
			float  prevRayTime  = 0;

			for (int i = 0; i < sampleCount; i++)
			{
				float rayTime = pow(float(i) / sampleCount, sampleDistributionExponent) * rayLength;
				float stepSize = (rayTime - prevRayTime);
				vec3 localPosition = rayStart + rayDir * rayTime;
				float localHeight = AtmosphereHeight(localPosition);
				vec3 localDensity = AtmosphereDensity(localHeight);

				opticalDepth += localDensity * stepSize;
				vec3 viewTransmittance = Absorb(opticalDepth);
				vec3 opticalDepthlight  = IntegrateOpticalDepth(localPosition, lightDir);
				vec3 lightTransmittance = Absorb(opticalDepthlight);

				rayleigh += viewTransmittance * lightTransmittance * phaseR * localDensity.x * stepSize;
				mie      += viewTransmittance * lightTransmittance * phaseM * localDensity.y * stepSize;
				prevRayTime = rayTime;
			}

			transmittance = Absorb(opticalDepth);
			return (rayleigh * C_RAYLEIGH + mie * C_MIE) * lightColor * EXPOSURE;
		}
		void main()
		{
			ivec3 clusterIndex = ivec3(gl_GlobalInvocationID.xyz);
			ivec2 skyboxSize = imageSize(skybox);
			float invImageSize = 1.0 / skyboxSize.x;
			
			// compute ray direction
			vec3 ray = vec3(1,1,1);
			if (clusterIndex.z == 0)
				ray = vec3( 1.0,1.0-2.0 * clusterIndex.y * invImageSize, 1.0 - 2.0 * clusterIndex.x * invImageSize);
			else if (clusterIndex.z == 1)
				ray = vec3(-1.0,1.0-2.0 * clusterIndex.y * invImageSize, 2.0 * clusterIndex.x * invImageSize - 1.0);
			else if (clusterIndex.z == 2)
				ray = vec3( 2.0 * clusterIndex.x * invImageSize - 1.0 ,  1.0 , 2.0 * clusterIndex.y * invImageSize - 1.0);
			else if (clusterIndex.z == 3)
				ray = vec3( 2.0 * clusterIndex.x * invImageSize - 1.0 , -1.0 ,1.0 - 2.0 * clusterIndex.y * invImageSize);
			else if (clusterIndex.z == 4)
				ray = vec3( 2.0 * clusterIndex.x * invImageSize - 1.0 ,1.0-2.0 * clusterIndex.y * invImageSize,  1.0);
			else
				ray = vec3(1.0 - 2.0 * clusterIndex.x * invImageSize,1.0-2.0 * clusterIndex.y * invImageSize, -1.0);
			ray = normalize(ray);
			vec4 lightDir = -normalize(m_directionalLightDirection);
			
			//vec3 color = mix(ray.xxx, facecolors[clusterIndex.z].xyz, 0.5);
			vec3 color = IntegrateScattering(vec3(0, cameraPosition.y, 0), ray, 100000.0 , lightDir.xyz, m_directionalLightColor.xyz);
			
			imageStore(skybox, clusterIndex, vec4(color, 1.0));
		}
		
	};
}