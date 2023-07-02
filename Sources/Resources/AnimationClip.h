#pragma once

#include <vector>
#include <map>
#include <GL/glew.h>

#include "ResourceVirtual.h"
#include "Joint.h"


class AnimationClip : public ResourceVirtual
{
	friend class AnimationComponent;
    friend class AnimationGraph;
    public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

        //
        struct Curve1DData
        {
            float m_time;
            float m_value;
        };
        struct Curve2DData
        {
            float m_time;
            vec2f m_value;
        };
        struct Curve3DData
        {
            float m_time;
            vec3f m_value;
        };
        struct Curve4DData
        {
            float m_time;
            vec4f m_value;
        };
        struct BoneCurves
        {
            std::string m_boneName;
            std::vector<Curve1DData> m_scaleCurve;
            std::vector<Curve4DData> m_positionCurve; // ->faster if we have w
            std::vector<Curve4DData> m_rotationCurve; // ->vector will be casted as Quat
        };
        //

        //  Default
		AnimationClip(const std::string& animationName = "unknown");
        ~AnimationClip();
		//

		//	Public functions
        void initialize(std::vector<BoneCurves>& _boneCurves, float _duration);
        void clear();
		//

		//	Set/get functions
        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;
        float getDuration() const;
        //

        //
        void onDrawImGui() override;
        //

    protected:
        //	Attributes
        static std::string defaultName;
        float m_duration;
        std::vector<BoneCurves> m_boneCurves;
		//
};
