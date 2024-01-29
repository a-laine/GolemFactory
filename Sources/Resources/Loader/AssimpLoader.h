#pragma once

#include <vector>
#include <map>
#include <list>
#include <tuple>
#include <string>

//#include <glm/glm.hpp>
#include "Math/TMath.h"
#include <assimp/scene.h>

#include "Resources/Skeleton.h"
#include "Resources/IResourceLoader.h"


class AssimpLoader : public IResourceLoader
{
    public:
        //  Miscellaneous
        enum class ResourceType
        {
            MESH,
            SKELETON,
            ANIMATION
        };
        //

        //  Default
		explicit AssimpLoader(ResourceType resourceToLoad);
        //

        //  Public functions
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

        void PrintError(const char* filename, const char* msg) override;
        void PrintWarning(const char* filename, const char* msg) override;

        static bool isAssimpFileMesh(const std::string& fileName);
        //


    private:
        //	Temporary loading structures
        typedef std::tuple<float, std::string, vec4f>  tupleVec3;
        typedef std::tuple<float, std::string, quatf> tupleQuat;
        typedef std::list<tupleVec3> BidirectionnalVectorMap;
        typedef std::list<tupleQuat> BidirectionnalQuaternionMap;
        //

        //	Private functions
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;
        void clear();
        void readSceneHierarchy(const aiNode* node, int depth = 0);
        //

        //	Debug
        void printJoint(Skeleton::Bone* bone, int depth);
        //


        //	Attributes
        std::string resourceName;
        ResourceType firstResource;


        std::vector<vec4f> vertices;
        std::vector<vec4f> normales;
        std::vector<vec4f> uvs;
        std::vector<vec4i> bones;
        std::vector<vec4f> weights;
        std::vector<unsigned int> faces;

        std::map<std::string, int> boneMap;

        mat4f globalMatrix;
        std::vector<unsigned int> roots;
        std::vector<Skeleton::Bone> joints;

        //std::vector<KeyFrame> animations;
};
