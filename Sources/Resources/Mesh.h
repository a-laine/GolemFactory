#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "ResourceVirtual.h"
#include "Loader/MeshLoader.h"

class Mesh : public ResourceVirtual
{
    public:
        //  Miscellaneous
        enum MeshConfiguration
        {
            VALID = 1<<0            //!< The valid flag position
        };

        struct Face{
            unsigned int vertex1;
            unsigned int vertex2;
            unsigned int vertex3;
        };

        enum AttributeType
        {
            MATRIX_44,
            FLOAT,
            INT,
            TEXTURE
        };
        struct Attribute
        {
            AttributeType type;
            int location;
            int size;
            void* data;
        };
        //

        //  Default
        Mesh(std::string path,std::string meshName);
        ~Mesh();

        bool isValid() const;
        //

        //  Set/get functions
        void addAttribute(AttributeType t,int l,int s,void* d);

        unsigned int getNumberVertices() const;
        unsigned int getNumberFaces() const;
        //

        //  Attributes
        uint8_t configuration;
        std::vector<Attribute> attributeSummary;
        std::vector<MeshLoader::vertexAttributes> data;
        std::vector<unsigned int> faces;

        uint8_t sizeofVertex;
        uint8_t offsetNormal;
        uint8_t offsetTexture;
        uint8_t offsetWeight;
        uint8_t offsetColor;
        //
};
