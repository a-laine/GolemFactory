#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>
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
		SkeletonComponent(const std::string& skeletonName);
		virtual ~SkeletonComponent() override;

		void setSkeleton(std::string skeletonName);
		void setSkeleton(Skeleton* skeleton);

		Skeleton* getSkeleton() const;
		unsigned int getNbJoints() const;
		const std::vector<glm::mat4>& getPose() const;
        std::vector<glm::mat4x4> getInverseBindPose() const;
		glm::vec3 getJointPosition(const std::string& jointName);
		//const std::vector<float>& getCapsules() const;
		const std::vector<glm::ivec2>& getSegmentsIndex() const;
		const std::vector<float>& getSegmentsRadius() const;

        bool isValid() const;
		
		void initToBindPose();
		void computePose(const std::vector<JointPose>& input);
		void computeCapsules(Mesh* mesh);
		void initializeVBOVAO();

		void drawBB();
		const GLuint getCapsuleVAO() const;

	private:
		void computePose(std::vector<glm::mat4>& result, const std::vector<JointPose>& input, const glm::mat4& parentPose, unsigned int joint);

		Skeleton* m_skeleton;

		Mutex locker;
		std::vector<glm::mat4> pose;
		std::vector<float> capsules;

		std::vector<glm::ivec2> segmentIndex;
		std::vector<float> segmentRadius;
		GLuint  vao, segIndexBuffer, segRadiusBuffer;
};