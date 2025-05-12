#include "MeshLoader.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include <Utiles/Parser/Reader.h>
#include <Utiles/ToolBox.h>
#include <Resources/Mesh.h>



bool MeshLoader::load(const std::string& resourceDirectory, const std::string& fileName)
{
    bool animated = false;
    {
        std::ifstream file(getFileName(resourceDirectory, fileName));
        if(!file.good())
        {
            if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
                std::cerr << "ERROR : MeshLoader : " << fileName << " : fail to open file" << std::endl;
            return false;
        }
        std::string line;
        while(!file.eof())
        {
            std::getline(file, line);
            if(line.find("b ") != std::string::npos || line.find("w ") != std::string::npos)
            {
                animated = true;
                break;
            }
        }
        file.close();
    }

    bool ok;
    if(animated)
        ok = loadFromFile_animated(resourceDirectory, fileName);
    else
        ok = loadFromFile_static(resourceDirectory, fileName);

    if(!ok)
    {
        vertices.clear();
        normals.clear();
        uvs.clear();
        faces.clear();
        weights.clear();
        bones.clear();
    }

    return ok;
}

void MeshLoader::initialize(ResourceVirtual* resource)
{
    Mesh* mesh = static_cast<Mesh*>(resource);
    mesh->initialize(std::move(vertices), std::move(normals), std::move(uvs), std::move(faces), std::move(bones), std::move(weights));
}


std::string MeshLoader::getFileName(const std::string& resourceDirectory, const std::string& fileName) const
{
    std::string str = resourceDirectory;
    str += Mesh::directory;
    str += fileName;
    str += (fileName.find_last_of('.') == std::string::npos) ? Mesh::extension : "";
    return str;
}

bool MeshLoader::loadFromFile_static(const std::string& resourceDirectory, const std::string& fileName)
{
    //	open file
    std::ifstream file(getFileName(resourceDirectory, fileName));
    if(!file.good())
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : MeshLoader : " << fileName << " : fail to open file" << std::endl;
        return false;
    }


    //	initialization
    int lineIndex = 0;
    std::string line;
    std::vector<vec4f> tmpv, tmpvn, tmpc;

    //	loading
    bool errorOccured = false;
    while(!file.eof())
    {
        std::getline(file, line);
        lineIndex++;

        if(line.empty()) continue;
        ToolBox::clearWhitespace(line);

        if(line.substr(0, 2) == "v ")
        {
            //	try to push a new vertex to vertex array
            std::istringstream iss(line.substr(2));
            vec4f v = vec4f::one;
            iss >> v.x; iss >> v.y; iss >> v.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpv.push_back(v);
        }
        else if(line.substr(0, 3) == "vn ")
        {
            //	try to push a new vertex normal to normales array
            std::istringstream iss(line.substr(2));
            vec4f vn = vec4f::zero;
            iss >> vn.x; iss >> vn.y; iss >> vn.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpvn.push_back(vn);
        }
        else if(line.substr(0, 2) == "c ")
        {
            //	try to push a new vertex color to colors array
            std::istringstream iss(line.substr(2));
            vec4f c = vec4f::one;
            iss >> c.x; iss >> c.y; iss >> c.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpc.push_back(c);
        }
        else if(line.substr(0, 2) == "f ")
        {
            gfvertex v1, v2, v3;
            if(sscanf_s(line.c_str(), "f %i//%i/%i %i//%i/%i %i//%i/%i",
                &v1.v, &v1.vn, &v1.uv,
                &v2.v, &v2.vn, &v2.uv,
                &v3.v, &v3.vn, &v3.uv) == 9)
            {
                //	check if requested indexes are present in arrays
                int outrange = 0;
                if(v1.v<0 || v1.v >= (int) tmpv.size()) outrange++;
                if(v2.v<0 || v2.v >= (int) tmpv.size()) outrange++;
                if(v3.v<0 || v3.v >= (int) tmpv.size()) outrange++;

                if(v1.vn<0 || v1.vn >= (int) tmpvn.size()) outrange++;
                if(v2.vn<0 || v2.vn >= (int) tmpvn.size()) outrange++;
                if(v3.vn<0 || v3.vn >= (int) tmpvn.size()) outrange++;

                if(v1.uv <0 || v1.uv >= (int) tmpc.size()) outrange++;
                if(v2.uv <0 || v2.uv >= (int) tmpc.size()) outrange++;
                if(v3.uv <0 || v3.uv >= (int) tmpc.size()) outrange++;

                //	push vertex attributes in arrays, print errors if not
                if(outrange) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
                else
                {
                    faces.push_back(vertices.size());
                    faces.push_back(vertices.size() + 1);
                    faces.push_back(vertices.size() + 2);

                    vertices.push_back(tmpv[v1.v]);		vertices.push_back(tmpv[v2.v]);		vertices.push_back(tmpv[v3.v]);
                    normals.push_back(tmpvn[v1.vn]);	normals.push_back(tmpvn[v2.vn]);	normals.push_back(tmpvn[v3.vn]);
                    uvs.push_back(tmpc[v1.uv]);		    uvs.push_back(tmpc[v2.uv]);		    uvs.push_back(tmpc[v3.uv]);
                }
            }
            else ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
        }
    }

    //	end
    file.close();
    return true;
}

bool MeshLoader::loadFromFile_animated(const std::string& resourceDirectory, const std::string& fileName)
{
    //	open file
    std::ifstream file(getFileName(resourceDirectory, fileName));
    if(!file.good())
    {
        if(ResourceVirtual::logVerboseLevel >= ResourceVirtual::VerboseLevel::ERRORS)
            std::cerr << "ERROR : MeshLoader : " << fileName << " : fail to open file" << std::endl;
        return false;
    }

    //	initialization
    int lineIndex = 0;
    std::string line;
    std::vector<vec4f> tmpv, tmpvn, tmpc, tmpw;
    std::vector<vec4i> tmpb;

    //	loading
    bool errorOccured = false;
    while(!file.eof())
    {
        std::getline(file, line);
        lineIndex++;

        if(line.empty()) continue;
        ToolBox::clearWhitespace(line);

        if(line.substr(0, 2) == "v ")
        {
            //	try to push a new vertex to vertex array
            std::istringstream iss(line.substr(2));
            vec4f v = vec4f::one;
            iss >> v.x; iss >> v.y; iss >> v.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpv.push_back(v);
        }
        else if(line.substr(0, 3) == "vn ")
        {
            //	try to push a new vertex normales to normales array
            std::istringstream iss(line.substr(2));
            vec4f vn = vec4f::zero;
            iss >> vn.x; iss >> vn.y; iss >> vn.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpvn.push_back(vn);
        }
        else if(line.substr(0, 2) == "c ")
        {
            //	try to push a new vertex color to colors array
            std::istringstream iss(line.substr(2));
            vec4f c = vec4f::one;
            iss >> c.x; iss >> c.y; iss >> c.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpc.push_back(c);
        }
        else if(line.substr(0, 2) == "w ")
        {
            //	try to push a new vertex weight list to weights list array
            std::istringstream iss(line.substr(2));
            vec4f w = vec4f::zero;
            iss >> w.x; iss >> w.y; iss >> w.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpw.push_back(w);
        }
        else if(line.substr(0, 2) == "b ")
        {
            //	try to push a new vertex bones list to bones list array
            std::istringstream iss(line.substr(2));
            vec4i b = vec4i(1, 0, 0, 0);
            iss >> b.x; iss >> b.y; iss >> b.z;
            if(iss.fail()) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
            tmpb.push_back(b);
        }
        else if(line.substr(0, 2) == "f ")
        {
            gfvertex_extended v1, v2, v3;
            if(sscanf_s(line.c_str(), "f %i//%i/%i/%i/%i %i//%i/%i/%i/%i %i//%i/%i/%i/%i",
                &v1.v, &v1.vn, &v1.uv, &v1.w, &v1.b,
                &v2.v, &v2.vn, &v2.uv, &v2.w, &v2.b,
                &v3.v, &v3.vn, &v3.uv, &v3.w, &v3.b) == 15)
            {
                //	check if requested indexes are present in arrays
                int outrange = 0;
                if(v1.v<0 || v1.v >= (int) tmpv.size()) outrange++;
                if(v2.v<0 || v2.v >= (int) tmpv.size()) outrange++;
                if(v3.v<0 || v3.v >= (int) tmpv.size()) outrange++;

                if(v1.vn<0 || v1.vn >= (int) tmpvn.size()) outrange++;
                if(v2.vn<0 || v2.vn >= (int) tmpvn.size()) outrange++;
                if(v3.vn<0 || v3.vn >= (int) tmpvn.size()) outrange++;

                if(v1.uv <0 || v1.uv >= (int) tmpc.size()) outrange++;
                if(v2.uv <0 || v2.uv >= (int) tmpc.size()) outrange++;
                if(v3.uv <0 || v3.uv >= (int) tmpc.size()) outrange++;

                if(v1.w<0 || v1.w >= (int) tmpw.size()) outrange++;
                if(v2.w<0 || v2.w >= (int) tmpw.size()) outrange++;
                if(v3.w<0 || v3.w >= (int) tmpw.size()) outrange++;

                if(v1.b<0 || v1.b >= (int) tmpb.size()) outrange++;
                if(v2.b<0 || v2.b >= (int) tmpb.size()) outrange++;
                if(v3.b<0 || v3.b >= (int) tmpb.size()) outrange++;

                //	push vertex attributes in arrays, print errors if not
                if(outrange) ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
                else
                {
                    faces.push_back(vertices.size());
                    faces.push_back(vertices.size() + 1);
                    faces.push_back(vertices.size() + 2);

                    vertices.push_back(tmpv[v1.v]);		vertices.push_back(tmpv[v2.v]);		vertices.push_back(tmpv[v3.v]);
                    normals.push_back(tmpvn[v1.vn]);	normals.push_back(tmpvn[v2.vn]);	normals.push_back(tmpvn[v3.vn]);
                    uvs.push_back(tmpc[v1.uv]);		    uvs.push_back(tmpc[v2.uv]);		    uvs.push_back(tmpc[v3.uv]);
                    weights.push_back(tmpw[v1.w]);		weights.push_back(tmpw[v2.w]);		weights.push_back(tmpw[v3.w]);
                    bones.push_back(tmpb[v1.b]);		bones.push_back(tmpb[v2.b]);		bones.push_back(tmpb[v3.b]);
                }
            }
            else ResourceVirtual::printErrorLog(fileName, lineIndex, errorOccured);
        }
    }

    //	end
    file.close();
    return true;
}

