#pragma once

#include <Math/TMath.h>
#include <string>

struct AnimationParameter
{
	static const char* g_ParameterTypeCombo;

	enum ParameterType { TRIGGER = 0, BOOL, INT, FLOAT };
	ParameterType m_type;

	typedef union
	{
		bool Bool;
		int Int;
		float Float;
	} Param;
	Param m_value;
	std::string m_name;
};

struct BoneCurvesState
{
	std::string m_boneName;
	int m_skeletonBoneIndex;

	int m_scaleKey;
	int m_rotKey;
	int m_posKey;

	float m_scale;
	vec4f m_position;
	quatf m_rotation;
};

struct SubGraph
{
	enum SubGraphType { STATE_MACHINE, BLEND_TREE };
	SubGraphType m_type;

	void* m_graph;
	void* m_graphData;
	void* m_graphRuntime;
};