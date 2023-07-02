#include "AnimationClip.h"
#include "Loader/AnimationLoader.h"

#include <Utiles/Assert.hpp>

//#include "imgui_plot.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

//  Static attributes
char const * const AnimationClip::directory = "Animations/";
char const * const AnimationClip::extension = ".animation";
std::string AnimationClip::defaultName;
//

//  Default
AnimationClip::AnimationClip(const std::string& AnimationClipName) : ResourceVirtual(AnimationClipName, ResourceVirtual::ResourceType::ANIMATION) 
{

}
AnimationClip::~AnimationClip() {}
//

//	Public functions
/*void AnimationClip::initialize(const std::vector<KeyFrame>& AnimationClips)
{
    GF_ASSERT(state == INVALID);
    if(!AnimationClips.empty())
    {
        //state = LOADING;
        timeLine = AnimationClips;
        state = VALID;
    }
}

void AnimationClip::initialize(std::vector<KeyFrame>&& AnimationClips)
{
    GF_ASSERT(state == INVALID);
    if(!AnimationClips.empty())
    {
        //state = LOADING;
        timeLine = std::move(AnimationClips);
        state = VALID;
    }
}

void AnimationClip::initialize(const std::vector<KeyFrame>& AnimationClips, const std::map<std::string, KeyLabel>& names)
{
    GF_ASSERT(state == INVALID);
    if(!AnimationClips.empty())
    {
        //state = LOADING;
        timeLine = AnimationClips;
        labels = names;
        state = VALID;
    }
}

void AnimationClip::initialize(std::vector<KeyFrame>&& AnimationClips, std::map<std::string, KeyLabel>&& names)
{
    GF_ASSERT(state == INVALID);
    if(!AnimationClips.empty())
    {
        //state = LOADING;
        timeLine = std::move(AnimationClips);
        labels = std::move(names);
        state = VALID;
    }
}*/

void AnimationClip::initialize(std::vector<BoneCurves>& _boneCurves, float _duration)
{
    m_boneCurves.swap(_boneCurves);
    m_duration = _duration;
    state = VALID;
}

void AnimationClip::clear()
{
    if(state == VALID)
    {
        //state = LOADING;
        //labels.clear();
        //timeLine.clear();
        state = INVALID;
    }
}

/*std::vector<mat4f> AnimationClip::getKeyPose(const unsigned int& keyFrame, const std::vector<unsigned int>& roots, const std::vector<Joint>& hierarchy) const
{
	std::vector<mat4f> pose(timeLine[0].poses.size(), mat4f(1.f));
	for (unsigned int i = 0; i < roots.size() && keyFrame < timeLine.size(); i++)
		computePose(keyFrame, pose, mat4f(1.f), roots[i], hierarchy);
	return pose;
}
std::pair<int, int> AnimationClip::getBoundingKeyFrameIndex(float time) const
{
	int previous = -1;
	int next = -1;
	for (unsigned int i = 0; i < timeLine.size(); i++)
	{
		if (timeLine[i].time >= time)
		{
			previous = i - 1;
			next = i;
			break;
		}
	}
	return std::pair<int, int>(previous, next);
}
const std::vector<KeyFrame>& AnimationClip::getTimeLine() const { return timeLine; }
const std::map<std::string, KeyLabel>& AnimationClip::getLabels() const { return labels; }*/

std::string AnimationClip::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}

std::string AnimationClip::getIdentifier() const
{
    return getIdentifier(name);
}

std::string AnimationClip::getLoaderId(const std::string& resourceName) const
{
    return extension;
}

float AnimationClip::getDuration() const { return m_duration; }

const std::string& AnimationClip::getDefaultName() { return defaultName; }
void AnimationClip::setDefaultName(const std::string& name) { defaultName = name; }
//

//	Protected functions
/*void AnimationClip::computePose(const unsigned int& keyFrame, std::vector<mat4f>& pose, const mat4f& parentPose, unsigned int joint, const std::vector<Joint>& hierarchy) const
{
	mat4f t = mat4f::translate(mat4f::identity, timeLine[keyFrame].poses[joint].position);
    mat4f r = mat4f(timeLine[keyFrame].poses[joint].rotation);
    mat4f s = mat4f::scale(mat4f::identity, timeLine[keyFrame].poses[joint].scale);

	pose[joint] = parentPose * t * r * s;
	for (unsigned int i = 0; i < hierarchy[joint].sons.size(); i++)
		computePose(keyFrame, pose, pose[joint], hierarchy[joint].sons[i], hierarchy);
}*/
//

//
void AnimationClip::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Skeleton infos");
    ImGui::Text("Curve count : %d", m_boneCurves.size());
    ImGui::Text("Duration : %f", m_duration);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Overview");

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    ImU32 graphColor[4] = { ImGui::ColorConvertFloat4ToU32(ImVec4(1, 0, 0, 1)), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 1, 0, 1)),
        ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 1, 1)), ImGui::ColorConvertFloat4ToU32(ImVec4(0.5, 0.5, 0.5, 1)) };

    const float ptsSize = 2.f;
    const auto Plot4Dcurve = [&](std::vector<AnimationClip::Curve4DData>& curve, int dimension = 4, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImU32 bgColor = 0xFF404040, ImVec2 graph_size = ImVec2(0, 100))
    {
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        if (graph_size.x == 0.0f)
            graph_size.x = ImGui::CalcItemWidth();
        if (graph_size.y == 0.0f)
            graph_size.y = style.FramePadding.y * 2;

        const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
        const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(0.0f, 0));
        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, NULL))
            return;


        vec4f scaleMin, scaleMax, scaleRange;
        if (scale_min == FLT_MAX || scale_max == FLT_MAX)
        {
            scaleMin = vec4f(-0.3f);
            scaleMax = vec4f(0.3f);
            for (auto& v : curve)
            {
                scaleMin = vec4f::min(scaleMin, v.m_value);
                scaleMax = vec4f::max(scaleMax, v.m_value);
            }
            scaleMin -= vec4f(0.03f);
            scaleMax += vec4f(0.03f);
        }
        else
        {
            scaleMin = vec4f(scale_min);
            scaleMax = vec4f(scale_max);
        }
        scaleRange = scaleMax - scaleMin;


        ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bgColor, true, style.FrameRounding);

        int item_count = curve.size() - 1;
        int res_w = std::min((int)graph_size.x, item_count) -1;
        for (int i = 0; i < item_count; i++)
        {
            vec4f a = curve[i].m_value;
            vec4f b = curve[i + 1].m_value;

            float t0 = lerp(inner_bb.Min.x, inner_bb.Max.x, curve[i].m_time / m_duration);
            float t1 = lerp(inner_bb.Min.x, inner_bb.Max.x, curve[i + 1].m_time / m_duration);

            for (int j = 0; j < dimension; j++)
            {
                float v0 = lerp(inner_bb.Min.y, inner_bb.Max.y, (a[j] - scaleMin[j]) / scaleRange[j]);
                float v1 = lerp(inner_bb.Min.y, inner_bb.Max.y, (b[j] - scaleMin[j]) / scaleRange[j]);
                window->DrawList->AddLine(ImVec2(t0, v0), ImVec2(t1, v1), graphColor[j]);
                window->DrawList->AddTriangle(ImVec2(t0 - ptsSize, v0 - ptsSize), ImVec2(t0 + ptsSize, v0 - ptsSize), ImVec2(t0, v0 + ptsSize), graphColor[j]);
                if (i == item_count - 1)
                    window->DrawList->AddTriangle(ImVec2(t1 - ptsSize, v1 - ptsSize), ImVec2(t1 + ptsSize, v1 - ptsSize), ImVec2(t1, v1 + ptsSize), graphColor[j]);
            }
        }
    };
    const auto Plot1Dcurve = [&](std::vector<AnimationClip::Curve1DData>& curve, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImU32 bgColor = 0xFF404040, ImVec2 graph_size = ImVec2(0, 100))
    {
        if (window->SkipItems)
            return;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        if (graph_size.x == 0.0f)
            graph_size.x = ImGui::CalcItemWidth();
        if (graph_size.y == 0.0f)
            graph_size.y = style.FramePadding.y * 2;

        const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(graph_size.x, graph_size.y));
        const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
        const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(0.0f, 0));
        ImGui::ItemSize(total_bb, style.FramePadding.y);
        if (!ImGui::ItemAdd(total_bb, NULL))
            return;

        float scaleMin, scaleMax, scaleRange;
        if (scale_min == FLT_MAX || scale_max == FLT_MAX)
        {
            scaleMin = -1.f;
            scaleMax = 1.f;
            for (auto& v : curve)
            {
                scaleMin = std::min(scaleMin, v.m_value);
                scaleMax = std::max(scaleMax, v.m_value);
            }
        }
        else
        {
            scaleMin = scale_min;
            scaleMax = scale_max;
        }
        scaleRange = scaleMax - scaleMin;

        ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, bgColor, true, style.FrameRounding);

        int item_count = curve.size() - 1;
        int res_w = std::min((int)graph_size.x, item_count) -1;

        for (int i = 0; i < item_count; i++)
        {
            float a = curve[i].m_value;
            float b = curve[i + 1].m_value;

            float t0 = lerp(inner_bb.Min.x, inner_bb.Max.x, curve[i].m_time / m_duration);
            float t1 = lerp(inner_bb.Min.x, inner_bb.Max.x, curve[i + 1].m_time / m_duration);
            float v0 = lerp(inner_bb.Min.y, inner_bb.Max.y, (a - scaleMin) / scaleRange);
            float v1 = lerp(inner_bb.Min.y, inner_bb.Max.y, (b - scaleMin) / scaleRange);
            window->DrawList->AddLine(ImVec2(t0, v0), ImVec2(t1, v1), graphColor[0]);
            window->DrawList->AddTriangle(ImVec2(t0 - ptsSize, v0 - ptsSize), ImVec2(t0 + ptsSize, v0 - ptsSize), ImVec2(t0, v0 + ptsSize), graphColor[0]);
            if (i == item_count - 1)
                window->DrawList->AddTriangle(ImVec2(t1 - ptsSize, v1 - ptsSize), ImVec2(t1 + ptsSize, v1 - ptsSize), ImVec2(t1, v1 + ptsSize), graphColor[0]);
        }
    };


    for (int i = 0; i < m_boneCurves.size(); i++)
    {
        if (window->SkipItems)
            break;

        if (ImGui::TreeNode(m_boneCurves[i].m_boneName.c_str()))
        {
            ImGui::Text("Position");
            Plot4Dcurve(m_boneCurves[i].m_positionCurve, 3);
            ImGui::Spacing();

            ImGui::Text("Rotation");
            Plot4Dcurve(m_boneCurves[i].m_rotationCurve, 4, -1.1f, 1.1f);
            ImGui::Spacing();

            ImGui::Text("Scale");
            Plot1Dcurve(m_boneCurves[i].m_scaleCurve);
            ImGui::Spacing();

            ImGui::TreePop();
        }
    }
#endif // USE_IMGUI
}
//