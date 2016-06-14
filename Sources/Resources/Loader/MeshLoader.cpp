#include "MeshLoader.h"

//  Public functions
int MeshLoader::loadMesh(std::string file,
                         std::vector<vertexAttributes>& data,
                         std::vector<unsigned int>& faces,
                         uint8_t& vertexSize,
                         uint8_t& offNor,uint8_t& offTex,uint8_t& offWeight,uint8_t& offColor)
{
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> nor;
    std::vector<glm::vec2> tex;
    std::vector<glm::vec3> weight;
    std::vector<glm::u8vec3> color;
    std::vector<Vertex> vertex;
    int i = 0;

    switch(getExtension(file))
    {
        case OBJ:
            i = loadObj(file,pos,nor,tex,vertex);
            if(i == 0) optimizeMesh(pos,nor,tex,weight,color,vertex,data,faces,vertexSize,offNor,offTex,offWeight,offColor);
            break;
        default:
            std::cerr<<"MeshLoader : Unable to load mesh."<<std::endl;
            std::cerr<<"             File extension not supported."<<std::endl;
            return 1;
    }
    return i;
}
//

//  Private functions
MeshLoader::Extension MeshLoader::getExtension(std::string file)
{
    int index = file.find_last_of(".") + 1;
    if(file.substr(index) == "obj") return OBJ;
    else return UNKNOWN;
}
void MeshLoader::optimizeMesh(std::vector<glm::vec3>& pos,
                              std::vector<glm::vec3>& nor,
                              std::vector<glm::vec2>& tex,
                              std::vector<glm::vec3>& weight,
                              std::vector<glm::u8vec3>& color,
                              std::vector<Vertex>& vertex,
                              std::vector<vertexAttributes>& data,
                              std::vector<unsigned int>& faces,
                              uint8_t& vertexSize,
                              uint8_t& offNor,uint8_t& offTex,uint8_t& offWeight,uint8_t& offColor)
{
    //  initialization
    std::set<Vertex> vertexPool;
    std::set<Vertex>::iterator found;
    vertexAttributes tmp;
    Vertex curent_vert;
        curent_vert.pos = vertex.front().pos;
        tmp.asFloat = pos[vertex.front().pos].x; data.insert(data.end(),tmp);
        tmp.asFloat = pos[vertex.front().pos].y; data.insert(data.end(),tmp);
        tmp.asFloat = pos[vertex.front().pos].z; data.insert(data.end(),tmp);
        vertexSize = 3*sizeof(vertexAttributes);

        if(!nor.empty())   {
            curent_vert.nor = vertex.front().nor;
            offNor = vertexSize;
            tmp.asFloat = nor[vertex.front().nor].x; data.insert(data.end(),tmp);
            tmp.asFloat = nor[vertex.front().nor].y; data.insert(data.end(),tmp);
            tmp.asFloat = nor[vertex.front().nor].z; data.insert(data.end(),tmp);
            vertexSize += 3*sizeof(vertexAttributes); }
        else { curent_vert.nor = std::numeric_limits<unsigned int>::max();
               offNor = std::numeric_limits<uint8_t>::max(); }

        if(!tex.empty())   {
            curent_vert.tex = vertex.front().tex;
            offTex = vertexSize;
            tmp.asFloat = tex[vertex.front().tex].x; data.insert(data.end(),tmp);
            tmp.asFloat = 1-tex[vertex.front().tex].y; data.insert(data.end(),tmp);
            vertexSize += 2*sizeof(vertexAttributes); }
        else { curent_vert.tex = std::numeric_limits<unsigned int>::max();
               offTex = std::numeric_limits<uint8_t>::max(); }

        if(!weight.empty()) {
            curent_vert.weight = vertex.front().weight;
            offWeight = vertexSize;
            tmp.asFloat = weight[vertex.front().weight].x; data.insert(data.end(),tmp);
            tmp.asFloat = weight[vertex.front().weight].y; data.insert(data.end(),tmp);
            tmp.asFloat = weight[vertex.front().weight].z; data.insert(data.end(),tmp);
            vertexSize += 3*sizeof(vertexAttributes);  }
        else { curent_vert.weight = std::numeric_limits<unsigned int>::max();
               offWeight = std::numeric_limits<uint8_t>::max(); }

        if(!color.empty()) {
            curent_vert.color = vertex.front().color;
            offColor = vertexSize;
            uint32_t r = color[vertex.front().color].x;
            uint32_t g = color[vertex.front().color].y;
            uint32_t b = color[vertex.front().color].z;
            tmp.asUInt = (r<<24)|(g<<16)|(b<<24)|(255);
            data.insert(data.end(),tmp);
            vertexSize += sizeof(vertexAttributes);}
        else { curent_vert.color = std::numeric_limits<unsigned int>::max();
               offColor = std::numeric_limits<uint8_t>::max(); }

        curent_vert.vertexIndex = 0;
        faces.insert(faces.end(),0);
        vertexPool.insert(curent_vert);

/*
    std::cout<<(int)vertexSize<<std::endl;
    std::cout<<'\t'<<(int)offNor<<std::endl;
    std::cout<<'\t'<<(int)offTex<<std::endl;
    std::cout<<'\t'<<(int)offWeight<<std::endl;
    std::cout<<'\t'<<(int)offColor<<std::endl;
    std::cout<<std::endl;
*/

    //  keep unique vertex and discard doubloon
    for(unsigned int i=1;i<vertex.size();i++)
    {
        curent_vert.pos = vertex[i].pos;
        if(!nor.empty()) {curent_vert.nor = vertex[i].nor;}
        else curent_vert.nor = std::numeric_limits<unsigned int>::max();
        if(!tex.empty()) {curent_vert.tex = vertex[i].tex;}
        else curent_vert.tex = std::numeric_limits<unsigned int>::max();
        if(!weight.empty()) {curent_vert.weight = vertex[i].weight;}
        else curent_vert.weight = std::numeric_limits<unsigned int>::max();
        if(!color.empty()) {curent_vert.color = vertex[i].color;}
        else curent_vert.color = std::numeric_limits<unsigned int>::max();
        curent_vert.vertexIndex = vertexPool.size();

        found = vertexPool.find(curent_vert);
        if(found != vertexPool.end())  faces.insert(faces.end(),found->vertexIndex);
        else
        {
            tmp.asFloat = pos[vertex[i].pos].x; data.insert(data.end(),tmp);
            tmp.asFloat = pos[vertex[i].pos].y; data.insert(data.end(),tmp);
            tmp.asFloat = pos[vertex[i].pos].z; data.insert(data.end(),tmp);
            if(!nor.empty())
            {
                tmp.asFloat = nor[vertex[i].nor].x; data.insert(data.end(),tmp);
                tmp.asFloat = nor[vertex[i].nor].y; data.insert(data.end(),tmp);
                tmp.asFloat = nor[vertex[i].nor].z; data.insert(data.end(),tmp);
            }
            if(!tex.empty())
            {
                tmp.asFloat = tex[vertex[i].tex].x; data.insert(data.end(),tmp);
                tmp.asFloat = 1-tex[vertex[i].tex].y; data.insert(data.end(),tmp);
            }
            if(!weight.empty())
            {
                tmp.asFloat = weight[vertex[i].weight].x; data.insert(data.end(),tmp);
                tmp.asFloat = weight[vertex[i].weight].y; data.insert(data.end(),tmp);
                tmp.asFloat = weight[vertex[i].weight].z; data.insert(data.end(),tmp);
            }
            if(!color.empty())
            {
                uint32_t r = color[vertex[i].color].x;
                uint32_t g = color[vertex[i].color].y;
                uint32_t b = color[vertex[i].color].z;
                tmp.asUInt = (r<<24)|(g<<16)|(b<<24)|(255);
                data.insert(data.end(),tmp);
            }

            faces.insert(faces.end(),vertexPool.size());
            vertexPool.insert(curent_vert);
        }
    }

    std::cout<<data.size()/vertexSize<<std::endl;
    std::cout<<faces.size()/3<<std::endl<<std::endl;

}
int MeshLoader::loadObj(std::string file,
                        std::vector<glm::vec3>& pos,
                        std::vector<glm::vec3>& nor,
                        std::vector<glm::vec2>& tex,
                        std::vector<MeshLoader::Vertex>& vertex)
{
	glm::vec3 tmpV3;
    glm::vec2 tmpV2;
    glm::u8vec3 tmpV3ui;
    Vertex tmpVert1,tmpVert2,tmpVert3;

    std::ifstream meshFile(file);
    if(!meshFile.good())
    {
        std::cerr<<"MeshLoader : Unable to load mesh:"<<std::endl;
        std::cerr<<"             Fail to open mesh file"<<std::endl;
        return 1;
    }

    unsigned int lineNb = 0;
    std::string line, vert, attr;
    while(!meshFile.eof())
    {
        std::getline(meshFile,line);  lineNb++;
        if(line.size() == 0) continue;          //empty line
        else if(line.find("#") == 0) continue;  //comment line
        std::stringstream lineStream(line);

        if(line.find("vn") != std::string::npos)
        {
            lineStream.ignore(2);
            lineStream >> tmpV3.x;
            lineStream >> tmpV3.y;
            lineStream >> tmpV3.z;
            nor.insert(nor.end(),tmpV3);
        }
        else if(line.find("vt") != std::string::npos)
        {
            lineStream.ignore(2);
            lineStream >> tmpV2.x;
            lineStream >> tmpV2.y;
            tex.insert(tex.end(),tmpV2);
        }
        else if(line.find("vp") != std::string::npos){}
        else if(line.find('v') == 0)
        {
            lineStream.ignore(1);
            lineStream >> tmpV3.x;
            lineStream >> tmpV3.y;
            lineStream >> tmpV3.z;
            pos.insert(pos.end(),tmpV3);
        }
        else if(line.find('f') == 0)
        {
            lineStream.ignore(2);
            int i=0;

            while(!lineStream.eof())
            {
                std::getline(lineStream,vert,' ');
                i++;
                std::stringstream vertStream(vert);
                switch(i)
                {
                    case 1:
                        std::getline(vertStream,attr,'/'); tmpVert1.pos = atoi(attr.c_str());
                        std::getline(vertStream,attr,'/'); tmpVert1.tex = atoi(attr.c_str());
                        std::getline(vertStream,attr,'/'); tmpVert1.nor = atoi(attr.c_str());
                        tmpVert1.weight = 0; tmpVert1.color = 0;
                        break;
                    case 2:
                        std::getline(vertStream,attr,'/'); tmpVert2.pos = atoi(attr.c_str());
                        std::getline(vertStream,attr,'/'); tmpVert2.tex = atoi(attr.c_str());
                        std::getline(vertStream,attr,'/'); tmpVert2.nor = atoi(attr.c_str());
                        tmpVert2.weight = 0; tmpVert2.color = 0;
                        break;
                    case 3:
                        std::getline(vertStream,attr,'/'); tmpVert3.pos = atoi(attr.c_str());
                        std::getline(vertStream,attr,'/'); tmpVert3.tex = atoi(attr.c_str());
                        std::getline(vertStream,attr,'/'); tmpVert3.nor = atoi(attr.c_str());
                        tmpVert3.weight = 0; tmpVert3.color = 0;
                        break;
                    default: break;
                }
            }

            /*if(i!=3)
            {
                std::cerr<<"MeshLoader : Unable to load mesh:"<<std::endl;
                std::cerr<<"             Invalid number of vertices for face definition at line "<<lineNb<<std::endl;
                std::cerr<<"             All faces must be triangles"<<std::endl;
                return 1;
            }
            else*/
            {
                tmpVert1.pos--;        tmpVert2.pos--;      tmpVert3.pos--;
                tmpVert1.nor--;        tmpVert2.nor--;      tmpVert3.nor--;
                tmpVert1.tex--;        tmpVert2.tex--;      tmpVert3.tex--;
                vertex.insert(vertex.end(),tmpVert1);
                vertex.insert(vertex.end(),tmpVert2);
                vertex.insert(vertex.end(),tmpVert3);
            }
        }
    }
    if(pos.empty() || vertex.empty()) return 1;
    else return 0;
}
//
