#include "ResourceManager.h"


//  Default
ResourceManager::ResourceManager(std::string path)
{
    repository = path;

    defaultTexture = "10points.png";
    defaultFont = "Comic Sans MS";
    defaultShader = "default";
    defaultMesh = "cube2.obj";
	defaultSkeleton = "test";
}
ResourceManager::~ResourceManager()
{
    for(auto element : textureList)
        delete element.second;
    textureList.clear();

    for(auto element : fontList)
        delete element.second;
    fontList.clear();

    for(auto element : shaderList)
        delete element.second;
    shaderList.clear();

    for(auto element : meshList)
        delete element.second;
    meshList.clear();

	for (auto element : skeletonList)
		delete element.second;
	skeletonList.clear();

    clearGarbage();
}
//

//  Public functions
void ResourceManager::setRepository(std::string path)
{
    repository = path;
}
void ResourceManager::release(ResourceVirtual* resource)
{
    if(!resource) return;
    resource->count--;

    if(resource->count<=0)
    {
        mutexList.lock();
        switch(resource->type)
        {
			case ResourceVirtual::FONT:     fontList.erase(resource->name);		break;
            case ResourceVirtual::SHADER:   shaderList.erase(resource->name);	break;
            case ResourceVirtual::TEXTURE:  textureList.erase(resource->name);	break;
            case ResourceVirtual::MESH:     meshList.erase(resource->name);		break;
			case ResourceVirtual::SKELETON:	skeletonList.erase(resource->name);	break;
            default: break;
        }
        mutexList.unlock();

        mutexGarbage.lock();
        garbage.insert(garbage.end(), resource);
        mutexGarbage.unlock();
    }
}
void ResourceManager::clearGarbage()
{
    std::vector<ResourceVirtual*> garbageCopy;

    mutexGarbage.lock();
    garbage.swap(garbageCopy);
    mutexGarbage.unlock();

	for (unsigned int i = 0; i < garbageCopy.size(); i++)
		if (garbageCopy[i]) delete garbageCopy[i];
    garbageCopy.clear();
}


Mesh* ResourceManager::getMesh(std::string name)
{
	if (name == "default") name = defaultMesh;
    Mesh* resource = nullptr;
    mutexList.lock();

	auto it = meshList.find(name);
	if (it != meshList.end())
    {
        resource = it->second;
        resource->count++;
    }
    mutexList.unlock();
	if (resource) return resource;

	resource = new Mesh(repository + "Meshes/", name);
	if (resource && resource->isValid())
    {
        mutexList.lock();
        meshList[name] = resource;
        mutexList.unlock();
        resource->count++;
    }
	else if (name != defaultMesh)
    {
        if(resource) delete resource;
		resource = getMesh(defaultMesh);
    }
	else if (resource)
    {
        delete resource;
        resource = nullptr;
    }
    return resource;
}
Texture* ResourceManager::getTexture(std::string name,uint8_t conf)
{
    if(name == "default") name = defaultTexture;
    Texture* resource = nullptr;
    mutexList.lock();

    auto it=textureList.find(name);
    if(it!=textureList.end())
    {
        resource = it->second;
        resource->count++;
    }
    mutexList.unlock();
    if(resource) return resource;

    resource = new Texture(repository+"Textures/",name,conf);

    if(resource && resource->isValid())
    {
    mutexList.lock();
        textureList[name] = resource;
        resource->count++;
    mutexList.unlock();
    }
    else if(name != defaultTexture)
    {
        if(resource) delete resource;
        resource = getTexture(defaultTexture);
    }
    else if(resource)
    {
        delete resource;
        resource = nullptr;
    }

    return resource;
}
Texture* ResourceManager::getTexture2D(std::string name,uint8_t conf) {return getTexture(name,conf|Texture::TEXTURE_2D);}
Shader* ResourceManager::getShader(std::string name)
{
    if(name == "default") name = defaultShader;
    Shader* resource = nullptr;
    mutexList.lock();

    auto it=shaderList.find(name);
    if(it!=shaderList.end())
    {
        resource = it->second;
        resource->count++;
    }
    mutexList.unlock();
    if(resource) return resource;

    resource = new Shader(repository+"Shaders/",name);
    if(resource && resource->isValid())
    {
        mutexList.lock();
        shaderList[name] = resource;
        resource->count++;
        mutexList.unlock();
    }
    else if(name != defaultShader)
    {
        std::cerr<<"fail load shader : "<<name<<std::endl;
        if(resource) delete resource;
        resource = getShader(defaultShader);
    }
    else if(resource)
    {
        delete resource;
        resource = nullptr;
    }
    return resource;
}
Font* ResourceManager::getFont(std::string name)
{
    if(name == "default") name = defaultFont;
    Font* resource = nullptr;

    mutexList.lock();
    auto it=fontList.find(name);
    if(it!=fontList.end())
    {
        resource = it->second;
        resource->count++;
    }
    mutexList.unlock();
    if(resource) return resource;

    resource = new Font(repository+"Font/",name);
    if(resource && resource->isValid())
    {
    mutexList.lock();
        fontList[name] = resource;
        resource->count++;
    mutexList.unlock();
    }
    else if(name != defaultFont)
    {
        if(resource) delete resource;
        resource = getFont(defaultFont);
    }
    else if(resource)
    {
        delete resource;
        resource = nullptr;
    }
    return resource;
}
Skeleton* ResourceManager::getSkeleton(std::string name)
{
	if (name == "default") name = defaultSkeleton;
	Skeleton* resource = nullptr;

	mutexList.lock();
	auto it = skeletonList.find(name);
	if (it != skeletonList.end())
	{
		resource = it->second;
		resource->count++;
	}
	mutexList.unlock();
	if (resource) return resource;

	resource = new Skeleton(repository + "Skeletons/", name);
	if (resource && resource->isValid())
	{
		mutexList.lock();
		skeletonList[name] = resource;
		resource->count++;
		mutexList.unlock();
	}
	else if (name != defaultSkeleton)
	{
		if (resource) delete resource;
		resource = getSkeleton(defaultSkeleton);
	}
	else if (resource)
	{
		delete resource;
		resource = nullptr;
	}
	return resource;
}


bool ResourceManager::addMesh(Mesh* mesh)
{
	auto it = meshList.find(mesh->name);
	if (it != meshList.end())
	{
		mesh->count++;
	}
	else
	{
		mutexList.lock();
		meshList[mesh->name] = mesh;
		mutexList.unlock();
		mesh->count++;
	}
}
bool ResourceManager::addTexture(Texture* texture)
{
	auto it = textureList.find(texture->name);
	if (it != textureList.end())
	{
		texture->count++;
	}
	else
	{
		mutexList.lock();
		textureList[texture->name] = texture;
		mutexList.unlock();
		texture->count++;
	}
}
bool ResourceManager::addShader(Shader* shader)
{
	auto it = shaderList.find(shader->name);
	if (it != shaderList.end())
	{
		shader->count++;
	}
	else
	{
		mutexList.lock();
		shaderList[shader->name] = shader;
		mutexList.unlock();
		shader->count++;
	}
}
bool ResourceManager::addFont(Font* font)
{
	auto it = fontList.find(font->name);
	if (it != fontList.end())
	{
		font->count++;
	}
	else
	{
		mutexList.lock();
		fontList[font->name] = font;
		mutexList.unlock();
		font->count++;
	}
}
bool ResourceManager::addSkeleton(Skeleton* skeleton)
{
	auto it = skeletonList.find(skeleton->name);
	if (it != skeletonList.end())
	{
		skeleton->count++;
	}
	else
	{
		mutexList.lock();
		skeletonList[skeleton->name] = skeleton;
		mutexList.unlock();
		skeleton->count++;
	}
}
//

//  Set/get functions
unsigned int ResourceManager::getNumberOfRessources(ResourceVirtual::ResourceType type)
{
    switch(type)
    {
        case ResourceVirtual::FONT:     return fontList.size();
        case ResourceVirtual::MESH:     return meshList.size();
        case ResourceVirtual::SHADER:   return shaderList.size();
        case ResourceVirtual::TEXTURE:  return textureList.size();
		case ResourceVirtual::SKELETON:	return skeletonList.size();
		default: return fontList.size() + shaderList.size() + textureList.size() + meshList.size() + skeletonList.size();
    }
}
std::string ResourceManager::getDefaultName(ResourceVirtual::ResourceType type)
{
    switch(type)
    {
        case ResourceVirtual::FONT:     return defaultFont;
        case ResourceVirtual::SHADER:   return defaultShader;
        case ResourceVirtual::TEXTURE:  return defaultTexture;
        case ResourceVirtual::MESH:     return defaultMesh;
		case ResourceVirtual::SKELETON:	return defaultSkeleton;
        default: return "";
    }
}
void ResourceManager::setDefaultName(ResourceVirtual::ResourceType type,std::string name)
{
    switch(type)
    {
		case ResourceVirtual::FONT:     defaultFont = name;			break;
        case ResourceVirtual::SHADER:   defaultShader = name;		break;
        case ResourceVirtual::TEXTURE:  defaultTexture = name;		break;
		case ResourceVirtual::MESH:		defaultMesh = name;			break;
		case ResourceVirtual::SKELETON:	defaultSkeleton = name;		break;
        default: break;
    }
}
//
