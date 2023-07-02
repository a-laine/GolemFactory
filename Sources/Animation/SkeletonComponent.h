#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

#include <EntityComponent/Component.hpp>
#include <Resources/Joint.h>
#include <Utiles/Mutex.h>
#include <Physics/BoundingVolume.h>


class Skeleton;
class Mesh;

class SkeletonComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(SkeletonComponent, Component)
	public:
		explicit SkeletonComponent();
		explicit SkeletonComponent(const std::string& skeletonName);
		virtual ~SkeletonComponent() override;

		void setSkeleton(std::string skeletonName);
		void setSkeleton(Skeleton* skeleton);

		Skeleton* getSkeleton() const;
		unsigned int getNbBones() const;
		const std::vector<mat4f>& getPose() const;
		void setPose(std::vector<mat4f> _pose);
		const std::vector<mat4f>& getInverseBindPose() const;
		vec4f getBonePosition(const std::string& jointName);

        bool isValid() const;

		bool load(Variant& jsonObject, const std::string& objectName) override;
		void save(Variant& jsonObject) override;

		void onAddToEntity(Entity* entity) override;
		void onDrawImGui() override;

		void recomputeBoundingBox();
		const AxisAlignedBox& getBoundingBox() const;

	private:
		Skeleton* m_skeleton;

		Mutex locker;
		std::vector<mat4f> pose;
		AxisAlignedBox boundingBox;
		float skinnedBoundingRadius;

#ifdef USE_IMGUI
		bool m_drawSkeleton = false;
		bool m_drawBoundingBox = false;
#endif
};