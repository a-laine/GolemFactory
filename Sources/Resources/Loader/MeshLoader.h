#pragma once

#include <vector>
#include <glm/glm.hpp>

#include <Resources/IResourceLoader.h>
#include <Resources/Joint.h>



class MeshLoader : public IResourceLoader
{
    public:
        bool load(const std::string& resourceDirectory, const std::string& fileName) override;
        void initialize(ResourceVirtual* resource) override;
        void getResourcesToRegister(std::vector<ResourceVirtual*>& resourceList) override;

    private:
        std::string getFileName(const std::string& resourceDirectory, const std::string& fileName) const;
        bool loadFromFile_static(const std::string& resourceDirectory, const std::string& fileName);
        bool loadFromFile_animated(const std::string& resourceDirectory, const std::string& fileName);


        struct gfvertex { int v, vn, c; };
        struct gfvertex_extended { int v, vn, c, w, b; };

        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec3> colors;
        std::vector<unsigned short> faces;
        std::vector<glm::ivec3> bones;
        std::vector<glm::vec3> weights;
};

