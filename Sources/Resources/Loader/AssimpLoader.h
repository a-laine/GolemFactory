#pragma once

#include <vector>
#include <map>
#include <list>
#include <tuple>
#include <string>

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include <Resources/Joint.h>
#include <Resources/IResourceLoader.h>


class AssimpLoader : public IResourceLoader
{
    public:
        //  Miscellaneous
        enum ResourceType
        {
            MESH,
            SKELETON,
            ANIMATION
        };
        //

        //  Default
        AssimpLoader(ResourceType resourceToLoad);
        //

        //  Public functions
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

        static bool isAssimpFileMesh(const std::string& fileName);
        //


    private:
        //	Temporary loading structures
        typedef std::tuple<float, std::string, glm::vec3>  tupleVec3;
        typedef std::tuple<float, std::string, glm::fquat> tupleQuat;
        typedef std::list<tupleVec3> BidirectionnalVectorMap;
        typedef std::list<tupleQuat> BidirectionnalQuaternionMap;
        //

        //	Private functions
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;
        void clear();
        void readSceneHierarchy(const aiNode* node, int depth = 0);
        //

        //	Debug
        void printJoint(unsigned int joint, int depth);
        //


        //	Attributes
        std::string resourceName;
        ResourceType firstResource;


        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normales;
        std::vector<glm::vec3> colors;
        std::vector<glm::ivec3> bones;
        std::vector<glm::vec3> weights;
        std::vector<unsigned short> faces;

        std::map<std::string, int> boneMap;

        glm::mat4 globalMatrix;
        std::vector<unsigned int> roots;
        std::vector<Joint> joints;

        std::vector<KeyFrame> animations;
};
