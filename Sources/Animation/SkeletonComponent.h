#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>

#include <EntityComponent/Component.hpp>
#include <Resources/Joint.h>
#include <Utiles/Mutex.h>


class Skeleton;
class Mesh;

class SkeletonComponent : public Component
{
	GF_DECLARE_COMPONENT_CLASS(SkeletonComponent, Component)
	public:
		explicit SkeletonComponent(const std::string& skeletonName);
		virtual ~SkeletonComponent() override;

		void setSkeleton(std::string skeletonName);
		void setSkeleton(Skeleton* skeleton);

		Skeleton* getSkeleton() const;
		unsigned int getNbJoints() const;
		const std::vector<mat4f>& getPose() const;
        std::vector<mat4f> getInverseBindPose() const;
		vec4f getJointPosition(const std::string& jointName);
		//const std::vector<float>& getCapsules() const;
		const std::vector<vec2i>& getSegmentsIndex() const;
		const std::vector<float>& getSegmentsRadius() const;

        bool isValid() const;
		
		void initToBindPose();
		void computePose(const std::vector<JointPose>& input);
		void computeCapsules(Mesh* mesh);
		void initializeVBOVAO();

		void drawBB();
		const GLuint getCapsuleVAO() const;


		void onAddToEntity(Entity* entity) override;

	private:
		void computePose(std::vector<mat4f>& result, const std::vector<JointPose>& input, const mat4f& parentPose, unsigned int joint);

		Skeleton* m_skeleton;

		Mutex locker;
		std::vector<mat4f> pose;
		std::vector<float> capsules;

		std::vector<vec2i> segmentIndex;
		std::vector<float> segmentRadius;
		GLuint  vao, segIndexBuffer, segRadiusBuffer;
};