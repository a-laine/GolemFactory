#pragma once

#include <vector>
//#include <glm/glm.hpp>

#include "Math/TMath.h"
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


        struct gfvertex { int v, vn, uv; };
        struct gfvertex_extended { int v, vn, uv, w, b; };

        std::vector<vec4f> vertices;
        std::vector<vec4f> normals;
        std::vector<vec4f> uvs;
        std::vector<unsigned short> faces;
        std::vector<vec4i> bones;
        std::vector<vec4f> weights;
};

