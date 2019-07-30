#include "SkeletonComponent.h"

#include <Resources/ResourceManager.h>
#include <Resources/Skeleton.h>
#include <Resources/Mesh.h>



SkeletonComponent::SkeletonComponent(const std::string& skeletonName)
{
	m_skeleton = ResourceManager::getInstance()->getResource<Skeleton>(skeletonName);
	if(m_skeleton)
		pose = m_skeleton->getBindPose();
}

SkeletonComponent::~SkeletonComponent()
{
	ResourceManager::getInstance()->release(m_skeleton);

	glDeleteBuffers(1, &segIndexBuffer);
	glDeleteBuffers(1, &segRadiusBuffer);
	glDeleteVertexArrays(1, &vao);
}

void SkeletonComponent::setSkeleton(std::string skeletonName)
{
	ResourceManager::getInstance()->release(m_skeleton);
	m_skeleton = ResourceManager::getInstance()->getResource<Skeleton>(skeletonName);

	locker.lock();
	pose.clear();
	capsules.clear();
	locker.lock();
}

void SkeletonComponent::setSkeleton(Skeleton* skeleton)
{
	ResourceManager::getInstance()->release(m_skeleton);
	if(skeleton) m_skeleton = ResourceManager::getInstance()->getResource<Skeleton>(skeleton);
	else m_skeleton = nullptr;

	locker.lock();
	pose.clear();
	capsules.clear();
	locker.lock();
}

Skeleton* SkeletonComponent::getSkeleton() const
{
	return m_skeleton;
}

unsigned int SkeletonComponent::getNbJoints() const
{
    GF_ASSERT(isValid());
	return (unsigned int) m_skeleton->joints.size();
}

const std::vector<glm::mat4>& SkeletonComponent::getPose() const
{
	return pose;
}

std::vector<glm::mat4x4> SkeletonComponent::getInverseBindPose() const
{
    GF_ASSERT(isValid());
    return m_skeleton->getInverseBindPose();
}

glm::vec3 SkeletonComponent::getJointPosition(const std::string& jointName)
{
    GF_ASSERT(isValid());

	locker.lock();
	bool emptyPose = pose.empty();
	locker.lock();

	if(emptyPose)
		return glm::vec3(0.f);
	int index = -1;
	for(unsigned int i = 0; i < m_skeleton->joints.size(); i++)
	{
		if(m_skeleton->joints[i].name == jointName)
		{
			index = i;
			break;
		}
	}
	if(index < 0)
		return glm::vec3(0.f);

	locker.lock();
	glm::vec3 p = glm::vec3(pose[index][3][0], pose[index][3][1], pose[index][3][2]);
	locker.lock();
	return p;
}

/*const std::vector<float>& SkeletonComponent::getCapsules() const
{
	return capsules;
}*/

const std::vector<glm::ivec2>& SkeletonComponent::getSegmentsIndex() const
{
	return segmentIndex;
}

const std::vector<float>& SkeletonComponent::getSegmentsRadius() const
{
	return segmentRadius;
}

bool SkeletonComponent::isValid() const
{
    return m_skeleton && m_skeleton->isValid();
}

void SkeletonComponent::initToBindPose()
{
    GF_ASSERT(isValid());
	locker.lock();
	pose = m_skeleton->getBindPose();
	locker.unlock();
}

void SkeletonComponent::computePose(const std::vector<JointPose>& input)
{
    GF_ASSERT(isValid());
	std::vector<glm::mat4> blendMatrix(input.size(), glm::mat4(1.f));
	for(unsigned int i = 0; i < m_skeleton->roots.size(); i++)
		computePose(blendMatrix, input, glm::mat4(1.f), m_skeleton->roots[i]);

	locker.lock();
	pose = blendMatrix;
	locker.unlock();
}

void SkeletonComponent::computeCapsules(Mesh* mesh)
{
    GF_ASSERT(isValid());
	if(!capsules.empty()) capsules.clear();
	if(!mesh || !mesh->getBones() || !mesh->getWeights())
		return;

	//	get all lists
	std::vector<glm::mat4> ibind = m_skeleton->getInverseBindPose();
	const std::vector<glm::vec3>& vertices = *mesh->getVertices();
	const std::vector<glm::ivec3>* bones = mesh->getBones();
	const std::vector<glm::vec3>* weights = mesh->getWeights();
	std::vector<Joint> joints = m_skeleton->getJoints();
	if(ibind.empty() || pose.empty() || !bones || !weights || joints.empty())
		return;
	capsules.assign(m_skeleton->getJoints().size(), 0.f);

	//	inflating bones
	for(unsigned int i = 0; i < vertices.size(); i++)
	{
		float maxDistance = 0.f;
		for(int j = 0; j < 3; j++)
		{
			int bone = (*bones)[i][j];
			glm::vec3 v = glm::vec3(ibind[bone] * glm::vec4(vertices[i], 1.f));
			if(joints[bone].sons.size() == 1)
			{
				glm::vec3 end = glm::vec3(joints[joints[bone].sons[0]].relativeBindTransform[3]);
				float d = glm::dot(v, end) / glm::length(end);
				if(d < 0.f) maxDistance += (*weights)[i][j] * glm::length(v);
				else if(d > 1.f) maxDistance += (*weights)[i][j] * glm::length(v - end);
				else maxDistance += (*weights)[i][j] * glm::length(v - d * glm::normalize(end));
			}
			else maxDistance += (*weights)[i][j] * glm::length(v);
		}
		for(int j = 0; j < 3; j++)
			capsules[(*bones)[i][j]] = std::max(capsules[(*bones)[i][j]], maxDistance);
	}

	//	contruct capsule drawing buffers
	for(unsigned int i = 0; i < joints.size(); i++)
	{
		if(joints[i].sons.empty())
		{
			segmentIndex.push_back(glm::ivec2(i, i));
			segmentRadius.push_back(capsules[i]);
		}
		else
		{
			for(unsigned int j = 0; j < joints[i].sons.size(); j++)
			{
				segmentIndex.push_back(glm::ivec2(i, joints[i].sons[j]));
				segmentRadius.push_back(capsules[i]);
			}
		}
	}
}

void SkeletonComponent::initializeVBOVAO()
{
    GF_ASSERT(isValid());

	//	generate vbo
	glGenBuffers(1, &segIndexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, segIndexBuffer);
	glBufferData(GL_ARRAY_BUFFER, segmentIndex.size() * sizeof(glm::ivec2), segmentIndex.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &segRadiusBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, segRadiusBuffer);
	glBufferData(GL_ARRAY_BUFFER, segmentRadius.size() * sizeof(float), segmentRadius.data(), GL_STATIC_DRAW);

	//	generate vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, segIndexBuffer);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, segRadiusBuffer);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, NULL);

	glBindVertexArray(0);
}

void SkeletonComponent::drawBB()
{
    GF_ASSERT(isValid());
	glBindVertexArray(vao);
	glDrawArrays(GL_POINTS, 0, (int)segmentIndex.size());
}

const GLuint SkeletonComponent::getCapsuleVAO() const { return vao; }

void SkeletonComponent::computePose(std::vector<glm::mat4>& result, const std::vector<JointPose>& input, const glm::mat4& parentPose, unsigned int joint)
{
    GF_ASSERT(isValid());
	result[joint] = parentPose * glm::translate(input[joint].position) * glm::toMat4(input[joint].rotation) * glm::scale(input[joint].scale);
	for(unsigned int i = 0; i < m_skeleton->joints[joint].sons.size(); i++)
		computePose(result, input, result[joint], m_skeleton->joints[joint].sons[i]);
}

