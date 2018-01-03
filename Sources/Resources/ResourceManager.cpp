#include "ResourceManager.h"
#include "Utiles/ToolBox.h"

//  Default
ResourceManager::ResourceManager(const std::string& path)
	: repository(path), defaultTexture("10points.png"), defaultFont("Comic Sans MS"), defaultShader("default"), defaultMesh("cube2.obj"), defaultSkeleton("human"), defaultAnimation("human")
{
    
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

	for (auto element : animationList)
		delete element.second;
	animationList.clear();

    clearGarbage();
}
//

//  Public functions
void ResourceManager::setRepository(const std::string& path)
{
    repository = path;
}
std::string ResourceManager::getRepository() const
{
	return repository;
}
void ResourceManager::release(ResourceVirtual* resource)
{
	//	prevent fail & decrement resources user counter
	if (!resource) return;
	resource->count--;

	if (resource->count <= 0)
    {
		//	remove resource from avalaible ressources lists
        mutexList.lock();
		switch (resource->type)
        {
            case ResourceVirtual::TEXTURE:		textureList.erase(resource->name);		break;
			case ResourceVirtual::SHADER:		shaderList.erase(resource->name);		break;
            case ResourceVirtual::MESH:			meshList.erase(resource->name);			break;
			case ResourceVirtual::ANIMATION:	animationList.erase(resource->name);	break;
			case ResourceVirtual::FONT:			fontList.erase(resource->name);			break;
			case ResourceVirtual::SKELETON:     skeletonList.erase(resource->name);		break;
            default: break;
        }
        mutexList.unlock();

		//	add resources to garbage for delayed deletion
        mutexGarbage.lock();
		garbage.insert(garbage.end(), resource);
        mutexGarbage.unlock();
    }
}
void ResourceManager::clearGarbage()
{
	//	instanciate the dummy list
    std::vector<ResourceVirtual*> garbageCopy;

	//	swap dummy list with garbage
    mutexGarbage.lock();
	garbage.swap(garbageCopy);
    mutexGarbage.unlock();

	//	delete element in garbage list
	for (unsigned int i = 0; i < garbageCopy.size(); i++)
		if (garbageCopy[i]) delete garbageCopy[i];
    garbageCopy.clear();
}


Mesh* ResourceManager::getMesh(std::string name)
{
	//	initialization
	if (name == "default") name = defaultMesh;
    Mesh* resource = nullptr;
    mutexList.lock();

	//	search mesh in resources containers
	auto it = meshList.find(name);
	if (it != meshList.end())
    {
        resource = it->second;
        resource->count++;
    }
    mutexList.unlock();
	if (resource) return resource;

	//	detect extention
	size_t ext = name.find_last_of('.');
	if(ext == std::string::npos || name.substr(ext) == Mesh::extension)
		resource = loadFromGolemFactoryFormat(name, ext != std::string::npos);
	else resource = loadFromAssimp(name);

	//	verify if mesh is successfully loaded
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
	//	initialization
    if(name == "default") name = defaultTexture;
    Texture* texture = nullptr;

	//	search texture in resources containers
    mutexList.lock();
    auto it=textureList.find(name);
    if(it!=textureList.end())
    {
		texture = it->second;
		texture->count++;
    }
    mutexList.unlock();
    if(texture) return texture;

	//	load texture
	texture = new Texture(repository + "Textures/", name, conf);

	//	return if successfully loaded
    if(texture && texture->isValid())
    {
		mutexList.lock();
		textureList[name] = texture;
		texture->count++;
		mutexList.unlock();
    }

	//	an error occur : try return the default one
    else if(name != defaultTexture)
    {
        if(texture) delete texture;
		texture = getTexture(defaultTexture);
    }

	//	an error occur on default texture loading : return nullptr
    else if(texture)
    {
        delete texture;
		texture = nullptr;
    }

	//	end
    return texture;
}
Texture* ResourceManager::getTexture2D(const std::string& name, uint8_t conf) { return getTexture(name, conf | Texture::TEXTURE_2D); }
Shader* ResourceManager::getShader(std::string name)
{
	//	initialization
    if(name == "default") name = defaultShader;
    Shader* shader = nullptr;

	//	search shader in resources containers
    mutexList.lock();
    auto it=shaderList.find(name);
    if(it!=shaderList.end())
    {
		shader = it->second;
		shader->count++;
    }
    mutexList.unlock();
    if(shader) return shader;

	//	load shader
	shader = new Shader(repository+"Shaders/",name);

	//	return if successfully loaded
    if(shader && shader->isValid())
    {
        mutexList.lock();
        shaderList[name] = shader;
		shader->count++;
        mutexList.unlock();
    }

	//	an error occur : try return the default one
    else if(name != defaultShader)
    {
        if(shader) delete shader;
		shader = getShader(defaultShader);
    }

	//	an error occur on default shader loading : return nullptr
    else if(shader)
    {
        delete shader;
		shader = nullptr;
    }

	//	end
    return shader;
}
Font* ResourceManager::getFont(std::string name)
{
	//	initialization
    if(name == "default") name = defaultFont;
    Font* font = nullptr;

	//	search font in resources containers
    mutexList.lock();
    auto it=fontList.find(name);
    if(it!=fontList.end())
    {
		font = it->second;
		font->count++;
    }
    mutexList.unlock();
    if(font) return font;

	//	load font
	font = new Font(repository+"Font/",name);

	//	return if successfully loaded
    if(font && font->isValid())
    {
		mutexList.lock();
        fontList[name] = font;
		font->count++;
		mutexList.unlock();
    }

	//	an error occur : try return the default one
    else if(name != defaultFont)
    {
        if(font) delete font;
		font = getFont(defaultFont);
    }

	//	an error occur on default font loading : return nullptr
    else if(font)
    {
        delete font;
		font = nullptr;
    }

	//	end
    return font;
}
Skeleton* ResourceManager::getSkeleton(std::string name)
{
	//	initialization
	if (name == "default") name = defaultSkeleton;
	Skeleton* skeleton = nullptr;
	
	//	search skeleton in resources containers
	mutexList.lock();
	auto it = skeletonList.find(name);
	if (it != skeletonList.end())
	{
		skeleton = it->second;
		skeleton->count++;
	}
	mutexList.unlock();
	if (skeleton) return skeleton;

	//	load skeleton
	skeleton = new Skeleton(repository + "Skeletons/", name);

	//	return if successfully loaded
	if (skeleton && skeleton->isValid())
	{
		mutexList.lock();
		skeletonList[name] = skeleton;
		skeleton->count++;
		mutexList.unlock();
	}

	//	an error occur : try return the default one
	else if (name != defaultSkeleton)
	{
		if (skeleton) delete skeleton;
		skeleton = getSkeleton(defaultSkeleton);
	}

	//	an error occur on default skeleton loading : return nullptr
	else if (skeleton)
	{
		delete skeleton;
		skeleton = nullptr;
	}

	//	end
	return skeleton;
}
Animation* ResourceManager::getAnimation(std::string name)
{
	//	initialization
	if (name == "default") name = defaultAnimation;
	Animation* animation = nullptr;

	//	search animation in resources containers
	mutexList.lock();
	auto it = animationList.find(name);
	if (it != animationList.end())
	{
		animation = it->second;
		animation->count++;
	}
	mutexList.unlock();
	if (animation) return animation;

	//	load animation
	animation = new Animation(repository + "Animations/", name);

	//	return if successfully loaded
	if (animation && animation->isValid())
	{
		mutexList.lock();
		animationList[name] = animation;
		animation->count++;
		mutexList.unlock();
	}

	//	an error occur : try return the default one
	else if (name != defaultAnimation)
	{
		if (animation) delete animation;
		animation = getAnimation(defaultAnimation);
	}

	//	an error occur on default animation loading : return nullptr
	else if (animation)
	{
		delete animation;
		animation = nullptr;
	}

	//	end
	return animation;
}


void ResourceManager::addMesh(Mesh* mesh)
{
	//	search mesh in resources containers
	auto it = meshList.find(mesh->name);
	if (it != meshList.end())
	{
		mesh->count++;
	}

	//	add to containers if not founded
	else
	{
		mutexList.lock();
		meshList[mesh->name] = mesh;
		mutexList.unlock();
		mesh->count++;
	}
}
void ResourceManager::addTexture(Texture* texture)
{
	//	search texture in resources containers
	auto it = textureList.find(texture->name);
	if (it != textureList.end())
	{
		texture->count++;
	}

	//	add to containers if not founded
	else
	{
		mutexList.lock();
		textureList[texture->name] = texture;
		mutexList.unlock();
		texture->count++;
	}
}
void ResourceManager::addShader(Shader* shader)
{
	//	search shader in resources containers
	auto it = shaderList.find(shader->name);
	if (it != shaderList.end())
	{
		shader->count++;
	}

	//	add to containers if not founded
	else
	{
		mutexList.lock();
		shaderList[shader->name] = shader;
		mutexList.unlock();
		shader->count++;
	}
}
void ResourceManager::addFont(Font* font)
{
	//	search font in resources containers
	auto it = fontList.find(font->name);
	if (it != fontList.end())
	{
		font->count++;
	}

	//	add to containers if not founded
	else
	{
		mutexList.lock();
		fontList[font->name] = font;
		mutexList.unlock();
		font->count++;
	}
}
void ResourceManager::addSkeleton(Skeleton* skeleton)
{
	//	search skeleton in resources containers
	auto it = skeletonList.find(skeleton->name);
	if (it != skeletonList.end())
	{
		skeleton->count++;
	}

	//	add to containers if not founded
	else
	{
		mutexList.lock();
		skeletonList[skeleton->name] = skeleton;
		mutexList.unlock();
		skeleton->count++;

		std::cout << skeleton->name << " added" << std::endl;
	}
}
void ResourceManager::addAnimation(Animation* animation)
{
	//	search animation in resources containers
	auto it = animationList.find(animation->name);
	if (it != animationList.end())
	{
		animation->count++;
	}

	//	add to containers if not founded
	else
	{
		mutexList.lock();
		animationList[animation->name] = animation;
		mutexList.unlock();
		animation->count++;
	}
}
//

//  Set/get functions
unsigned int ResourceManager::getNumberOfRessources(const ResourceVirtual::ResourceType& type) const
{
    switch(type)
    {
        case ResourceVirtual::FONT:			return fontList.size();
        case ResourceVirtual::MESH:			return meshList.size();
        case ResourceVirtual::SHADER:		return shaderList.size();
        case ResourceVirtual::TEXTURE:		return textureList.size();
		case ResourceVirtual::ANIMATION:	return animationList.size();
		case ResourceVirtual::SKELETON:		return skeletonList.size();
		default: return fontList.size() + shaderList.size() + textureList.size() + meshList.size() + animationList.size() + skeletonList.size();
    }
}
std::string ResourceManager::getDefaultName(const ResourceVirtual::ResourceType& type) const
{
    switch(type)
    {
        case ResourceVirtual::FONT:			return defaultFont;
        case ResourceVirtual::SHADER:		return defaultShader;
        case ResourceVirtual::TEXTURE:		return defaultTexture;
        case ResourceVirtual::MESH:			return defaultMesh;
		case ResourceVirtual::ANIMATION:	return defaultAnimation;
		case ResourceVirtual::SKELETON:		return defaultSkeleton;
        default: return "";
    }
}
void ResourceManager::setDefaultName(const ResourceVirtual::ResourceType& type, const std::string& name)
{
    switch(type)
    {
		case ResourceVirtual::FONT:			defaultFont = name;			break;
        case ResourceVirtual::SHADER:		defaultShader = name;		break;
        case ResourceVirtual::TEXTURE:		defaultTexture = name;		break;
		case ResourceVirtual::MESH:			defaultMesh = name;			break;
		case ResourceVirtual::ANIMATION:	defaultAnimation = name;	break;
		case ResourceVirtual::SKELETON:		defaultSkeleton = name;		break;
        default: break;
    }
}
//

//  Private functions
Mesh* ResourceManager::loadFromAssimp(const std::string& fileName)
{
	//	load mesh with mesh loader
	Mesh* m;
	MeshLoader ml;
	ml.loadMesh(repository + "Meshes/" + fileName);
	if (!ml.bones.empty() && !ml.weights.empty())
		m = new MeshAnimated(fileName, !ml.animations.empty(), ml.vertices, ml.normales, ml.colors, ml.bones, ml.weights, ml.faces);
	else m = new Mesh(fileName, ml.vertices, ml.normales, ml.colors, ml.faces);

	//	create skeleton if needed
	if (!ml.roots.empty() && skeletonList.find(fileName) == skeletonList.end())
	{
		Skeleton* skeleton = new Skeleton(fileName, ml.roots, ml.joints);
		if (skeleton->isValid())
		{
			mutexList.lock();
			skeletonList[fileName] = skeleton;
			mutexList.unlock();
		}
	}

	//	create animation if needed
	if (!ml.animations.empty() && animationList.find(fileName) == animationList.end())
	{
		Animation* animation = new Animation(fileName, ml.animations);
		if (animation->isValid())
		{
			mutexList.lock();
			animationList[fileName] = animation;
			mutexList.unlock();
		}
	}

	return m;
}
Mesh* ResourceManager::loadFromGolemFactoryFormat(const std::string& fileName, bool haveExtention) const
{
	bool animated = false;
	{
		std::ifstream file(repository + "Meshes/" + fileName + (haveExtention ? "" : Mesh::extension));
		if (!file.good()) return nullptr;
		std::string line;
		while (!file.eof())
		{
			std::getline(file, line);
			if (line.find("b ") != std::string::npos || line.find("w ") != std::string::npos) {
				animated = true;
				break; }
		}
		file.close();
	}
	if (animated) return new MeshAnimated(repository + "Meshes/", fileName);
	else return new Mesh(repository + "Meshes/", fileName);

	return nullptr;
}
//
