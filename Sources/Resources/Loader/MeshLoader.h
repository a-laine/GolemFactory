#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <set>
#include <limits>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class MeshLoader
{
    public:
        //  Miscellaneous
        union vertexAttributes {
            float asFloat;
            unsigned int asUInt;
        };
        //

        //  Public functions
        static int loadMesh(std::string file,
							std::vector<glm::vec3>& vertices,
							std::vector<glm::vec3>& normales,
							std::vector<glm::vec3>& color,
							std::vector<unsigned int>& faces);
        //

    private:
        //  Miscellaneous
        enum Extension
        {
            UNKNOWN,
            OBJ
        };

        struct Vertex
        {
            unsigned int pos;
            unsigned int nor;
            unsigned int tex;
            unsigned int weight;
            unsigned int color;

            bool operator<(const Vertex& b) const
            {
                if(pos<b.pos) return true;
                else if(pos>b.pos) return false;
                if(nor<b.nor) return true;
                else if(nor>b.nor) return false;
                if(tex<b.tex) return true;
                else if(tex>b.tex) return false;
                if(weight<b.weight) return true;
                else if(weight>b.weight) return false;
                if(color<b.color) return true;
                else return false;
            };

            unsigned int vertexIndex;
        };
        //

        //  Private functions
        static Extension getExtension(std::string file);
        static void optimizeMesh(std::vector<glm::vec3>& pos,
                                 std::vector<glm::vec3>& nor,
                                 std::vector<glm::vec2>& tex,
                                 std::vector<glm::vec3>& weight,
                                 std::vector<glm::u8vec3>& color,
                                 std::vector<Vertex>& vertex,
                                 std::vector<vertexAttributes>& data,
                                 std::vector<unsigned int>& faces,
                                 uint8_t& vertexSize,
                                 uint8_t& offNor,uint8_t& offTex,uint8_t& offWeight,uint8_t& offColor);
        static int loadObj(std::string file,
                           std::vector<glm::vec3>& pos,
                           std::vector<glm::vec3>& nor,
                           std::vector<glm::vec2>& tex,
                           std::vector<Vertex>& vertex);
        //
};
