		
//  UniformBuffers.cginc
layout(std140, binding = 0) uniform GlobalMatrices
{
	mat4 view;
	mat4 projection;
	vec4 cameraPosition;
};
layout(std140, binding = 1) uniform EnvironementLighting
{
	vec4 m_backgroundColor;
	vec4 m_ambientColor;
	vec4 m_directionalLightDirection;
	vec4 m_directionalLightColor;
	
	mat4 shadowCascadeProjections[4];
	vec4 shadowFarPlanes;
	float m_shadowBlendMargin;
};

struct Light
{
	vec4 m_position;
	vec4 m_direction;
	vec4 m_color;
	float m_range;
	float m_intensity;
	float m_inCutOff;
	float m_outCutOff;
};
layout(std140, binding = 2) uniform Lights
{
	int lightCount;
	uint shadingConfiguration;
	float clusterDepthScale;
	float clusterDepthBias;
	float near;
	float far;
	float tanFovX;
	float tanFovY;
	Light lights[254];
};

layout(std140, binding = 3) uniform DebugShaderUniform
{
	vec4 vertexNormalColor;
	vec4 faceNormalColor;
	float wireframeEdgeFactor;
	float occlusionResultDrawAlpha;
	float occlusionResultCuttoff;
	float animatedTime;
};
layout(std140, binding = 4) uniform OmniShadows
{
	ivec4 omniShadowIndexLow;
	ivec4 omniShadowIndexHigh;
	vec4 omniShadowNearLow;
	vec4 omniShadowNearHigh;
	mat4 omniShadowProjections[48];
};

struct TerrainMaterial
{
	int m_albedo;
	int m_normal;
	float m_tiling;
	float m_metalic;
};
layout(std140, binding = 5) uniform TerrainMaterialCollections
{
	TerrainMaterial terrainMaterials[128];
};
//