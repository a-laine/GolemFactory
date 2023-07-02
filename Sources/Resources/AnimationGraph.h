#pragma once

#include "ResourceVirtual.h"
#include <Resources/AnimationClip.h>
#include <Utiles/Parser/Variant.h>


// GRAPH STRUCTURE (all sharable by instances)
struct AnimationGraphTransition;

struct AnimationGraphState
{
	std::string m_name;
	unsigned int id; // id in vector<AnimationGraphState>
	std::vector<AnimationGraphTransition*> m_transitionOut;
};

struct AnimationGraphParameter
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
struct AnimationGraphCondition
{
	unsigned int parameterId; // in order to get the right param for evaluation
	enum ComparisonType { GREATER, LESS, EQUALS, NOT_EQUALS };
	ComparisonType m_comparisonType;
	AnimationGraphParameter m_parameter;

	bool evaluate(const AnimationGraphParameter& _comparisonParameter) const;
};

struct AnimationGraphTransition
{
	AnimationGraphState* m_stateFrom;
	AnimationGraphState* m_stateTo;

	float m_duration;
	std::vector<AnimationGraphCondition> m_conditions;
};





// OVERRIDE / VARIANTS (sharable per instance but permit some indirection, or can be copied to full override per instance)
struct AnimationGraphStateData
{
	AnimationClip* m_animation;
	float m_speed;
};

struct AnimationGraphData
{
	~AnimationGraphData();

	std::string m_name;
	std::vector<AnimationGraphStateData> m_statesData; // same size and order as graph.m_states
};


// RUNTIME DATA (not sharable per instances)
class Skeleton;
struct AnimationGraphRuntimeData
{
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

	std::vector<AnimationGraphParameter> m_parameters;

	float m_stateTime;
	unsigned int m_currentStateIndex;
	std::vector<BoneCurvesState> m_currentStateEvaluation;
	std::vector<int> m_bones2currentStateEvaluation;

	float m_transitionTime, m_targetStateTime;
	AnimationGraphTransition* m_currentTransition;
	std::vector<BoneCurvesState> m_currentTargetEvaluation;
	std::vector<int> m_bones2currentTargetEvaluation;

	Skeleton* m_skeleton;
	std::vector<mat4f> m_skeletonFinalPose;
};


class AnimationGraph : public ResourceVirtual
{
	public:
		static char const* const directory;
		static char const* const extension;

		AnimationGraph(const std::string& _graphName = "unknown");
		~AnimationGraph();

		// initialization
		void initialize(std::vector<AnimationGraphState>& _states, std::vector<AnimationGraphTransition>& _transitions, unsigned int _entryState);
		void setDefaultParameters(std::vector<AnimationGraphParameter>& _parameters);
		void setVariants(std::map<std::string, AnimationGraphData>& _variants);
		//void addVariant(const std::string& _variantName, AnimationGraphData* _variantData);

		// instance copy functions
		std::vector<AnimationGraphParameter> getParametersCopy() const;
		const AnimationGraphData* getVariant(const std::string& _name) const;
		unsigned int getEntryState() const;

		// evaluation of the graph
		const std::vector<AnimationGraphState>& getStates() const;
		bool setState(unsigned int _stateId, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const;
		bool setState(const std::string& _stateName, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const;
		void evaluate(float _elapsedTime, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const;

		// resource stuff
		static std::string getIdentifier(const std::string& resourceName);
		static const std::string& getDefaultName();
		static void setDefaultName(const std::string& name);
		std::string getIdentifier() const override;
		std::string getLoaderId(const std::string& resourceName) const;

		// debug
		void onDrawImGui() override;

	protected:
		void setTransition(AnimationGraphTransition* _transition, const AnimationGraphData& _data, AnimationGraphRuntimeData& _runtimeData) const;

		static std::string defaultName;

		unsigned int m_entryState;
		std::vector<AnimationGraphState> m_states;
		std::vector<AnimationGraphTransition> m_transitions;

		std::map<std::string, AnimationGraphData> m_dataVariant;
		std::vector<AnimationGraphParameter> m_defaultParameters;
};