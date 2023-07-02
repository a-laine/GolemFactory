#pragma once

#include <vector>

#include "ResourceVirtual.h"
#include "Joint.h"


class Skeleton : public ResourceVirtual
{
	friend class AnimationComponent;
	friend class SkeletonComponent;
	friend class AnimationGraph;

	public:
        static char const * const directory;
        static char const * const extension;

        static std::string getIdentifier(const std::string& resourceName);
        static const std::string& getDefaultName();
        static void setDefaultName(const std::string& name);

		//
		struct Bone
		{
			Bone* parent;
			std::vector<Bone*> sons;
			unsigned int id;
			mat4f relativeBindTransform;
			std::string name;
		};
		struct BonePose
		{
			vec4f position;
			quatf rotation;
			vec4f scale;
		};
		//

        //	Default
		explicit Skeleton(const std::string& skeletonName);
		~Skeleton();
		//

        //void initialize(const std::vector<unsigned int>& rootsList, const std::vector<Joint>& jointsList);
        //void initialize(std::vector<unsigned int>&& rootsList, std::vector<Joint>&& jointsList);
		void initialize(std::vector<Bone>& boneList);
		const std::vector<mat4f>& getInverseBindPose() const;
		const std::vector<mat4f>& getBindPose() const;
		const std::vector<Bone>& getBones() const;
		const std::vector<Bone*>& getRoots() const;
        std::string getIdentifier() const override;
        std::string getLoaderId(const std::string& resourceName) const;

		int getBoneId(const std::string& name) const;
		//

		// Debug
		void onDrawImGui() override;
		//

	protected:
        static std::string defaultName;

		//	Protected functions
		//void computeBindPose(const mat4f& parentPose, unsigned int joint);
		//

		//	Attributes
		std::vector<Bone*> m_roots;
		std::vector<Bone> m_bones;


		std::vector<mat4f> m_inverseBindPose;
		std::vector<mat4f> m_bindPose;
		//
};
