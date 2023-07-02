#include "Skeleton.h"
#include "Loader/SkeletonLoader.h"

#include <Utiles/Assert.hpp>
#include <functional>

//  Static attributes
char const * const Skeleton::directory = "Skeletons/";
char const * const Skeleton::extension = ".skeleton";
std::string Skeleton::defaultName;
//

//	Default
Skeleton::Skeleton(const std::string& skeletonName) : ResourceVirtual(skeletonName, ResourceVirtual::ResourceType::SKELETON) 
{}
Skeleton::~Skeleton() {}
//


/*void Skeleton::initialize(const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList)
{
    GF_ASSERT(state == INVALID);
    if(!rootsList.empty() && !jointsList.empty())
    {
        state = LOADING;

        roots = rootsList;
        joints = jointsList;

        //	compute bind pose and inverse bind pose matrix lists
        inverseBindPose.assign(joints.size(), mat4f::identity);
        bindPose.assign(joints.size(), mat4f::identity);
        for(unsigned int i = 0; i < roots.size(); i++)
            computeBindPose(mat4f::identity, roots[i]);

        state = VALID;
    }
}

void Skeleton::initialize(std::vector<unsigned int>&& rootsList, std::vector<Joint>&& jointsList)
{
    GF_ASSERT(state == INVALID);
    if(!rootsList.empty() && !jointsList.empty())
    {
        state = LOADING;

        roots = std::move(rootsList);
        joints = std::move(jointsList);

        //	compute bind pose and inverse bind pose matrix lists
        inverseBindPose.assign(joints.size(), mat4f::identity);
        bindPose.assign(joints.size(), mat4f::identity);
        for(unsigned int i = 0; i < roots.size(); i++)
            computeBindPose(mat4f::identity, roots[i]);

        state = VALID;
    }
}*/
void Skeleton::initialize(std::vector<Bone>& boneList)
{
    m_bones.swap(boneList);
    m_roots.clear();
    for (Bone& b : m_bones)
    {
        if (!b.parent)
            m_roots.push_back(&b);
    }

    for (Skeleton::Bone& bone : m_bones)
    {
        mat4f m = bone.parent ? m_bindPose[bone.parent->id] : mat4f::identity;
        m = m * bone.relativeBindTransform;
        m_bindPose.push_back(m);
        m_inverseBindPose.push_back(mat4f::inverse(m));
    }
    state = VALID;
}

std::string Skeleton::getIdentifier(const std::string& resourceName)
{
    return std::string(directory) + resourceName;
}
std::string Skeleton::getIdentifier() const
{
    return getIdentifier(name);
}

std::string Skeleton::getLoaderId(const std::string& resourceName) const
{
    size_t ext = resourceName.find_last_of('.');
    if (ext != std::string::npos && resourceName.substr(ext) != Skeleton::extension)
        return "assimpSkel";
    else
        return extension;
}

const std::string& Skeleton::getDefaultName() { return defaultName; }
void Skeleton::setDefaultName(const std::string& name) { defaultName = name; }

const std::vector<mat4f>& Skeleton::getInverseBindPose() const { return m_inverseBindPose; }
const std::vector<mat4f>& Skeleton::getBindPose() const { return m_bindPose; }
const std::vector<Skeleton::Bone>& Skeleton::getBones() const { return m_bones; }
const std::vector<Skeleton::Bone*>& Skeleton::getRoots() const { return m_roots; }

int Skeleton::getBoneId(const std::string& name) const
{
    for (const Bone& bone : m_bones)
    {
        if (bone.name == name)
            return bone.id;
    }
    return -1;
}
//

//	Private functions
/*void Skeleton::computeBindPose(const mat4f& parentPose, unsigned int joint)
{
	bindPose[joint] = parentPose * joints[joint].relativeBindTransform;
	inverseBindPose[joint] = mat4f::inverse(bindPose[joint]);
	
	for (unsigned int i = 0; i < joints[joint].sons.size(); i++)
		computeBindPose(bindPose[joint], joints[joint].sons[i]);
}*/
//

//
#ifdef USE_IMGUI
ImGuiTextFilter g_jointNameFilter;
#endif

void Skeleton::onDrawImGui()
{
#ifdef USE_IMGUI
    ResourceVirtual::onDrawImGui();

    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Type infos");
    ImGui::Text("Fallback resource name : %s", defaultName.c_str());
    ImGui::Text("Directory : %s", directory);
    ImGui::Text("File extension : %s", extension);

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Skeleton infos");
    ImGui::Text("Bone count : %d", m_bones.size());

    ImGui::Spacing();
    ImGui::TextColored(ResourceVirtual::titleColorDraw, "Overview");
    g_jointNameFilter.Draw();
    ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;

    std::function<bool(Bone*)> FilterTest = [&](Bone* bone)
    {
        if (g_jointNameFilter.PassFilter(bone->name.c_str()))
            return true;
        for (int i = 0; i < bone->sons.size(); i++)
            if (FilterTest(bone->sons[i]))
                return true;
        return false;
    };
    std::function<void(Bone*)> DrawBone = [&](Bone* bone)
    {
        if (!bone)
            return;
        if (!FilterTest(bone))
            return;

        if (!bone->sons.empty())
        {
            if (ImGui::TreeNodeEx(bone->name.c_str(), nodeFlags))
            {
                ImGui::PushID(bone);
                for (int i = 0; i < bone->sons.size(); i++)
                    DrawBone(bone->sons[i]);
                ImGui::PopID();
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::Text("  %s", bone->name.c_str());
        }
    };

    for (int j = 0; j < m_roots.size(); j++)
        DrawBone(m_roots[j]);
#endif
}
//