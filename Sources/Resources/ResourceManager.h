#pragma once

#include <vector>
#include <map>
#include <string>

#include "Utiles/Singleton.h"
#include "Utiles/Mutex.h"

#include "Texture.h"
#include "Shader.h"
#include "Font.h"
#include "Mesh.h"
#include "MeshAnimated.h"
#include "Skeleton.h"
#include "Animation.h"

#include "Loader/MeshLoader.h"



class ResourceManager : public Singleton<ResourceManager>
{
    friend class Singleton<ResourceManager>;

    public:
        //  Public functions
        void setRepository(std::string path);
        void release(ResourceVirtual* resource);
        void clearGarbage();

        Mesh* getMesh(std::string name);
        Texture* getTexture(std::string name, uint8_t conf = 0x00);
        Texture* getTexture2D(const std::string& name, uint8_t conf = 0x00);
        Shader* getShader(std::string name);
        Font* getFont(std::string name);
		Skeleton* getSkeleton(std::string name);
		Animation* getAnimation(std::string name);

		void addMesh(Mesh* mesh);
		void addTexture(Texture* texture);
		void addShader(Shader* shader);
		void addFont(Font* font);
        //

        //  Set/get functions
        unsigned int getNumberOfRessources(ResourceVirtual::ResourceType type);
        std::string getDefaultName(ResourceVirtual::ResourceType type);
        void setDefaultName(ResourceVirtual::ResourceType type,std::string name);
        //


    private:
        //  Default
        ResourceManager(const std::string& path = "Resources/");	//!< Default constructor.
        ~ResourceManager();											//!< Default destructor.
        //

        //  Attributes
        Mutex mutexGarbage;								//!< A mutex to prevent garbage collision.
        std::vector<ResourceVirtual*> garbage;			//!< The list of resources to delete.

        Mutex mutexList;								//!< A mutex to prevent lists collisions.
        std::map<std::string, Texture*> textureList;	//!< The textures container.
        std::map<std::string, Font*> fontList;			//!< The fonts container.
        std::map<std::string, Shader*> shaderList;		//!< The shader container.
        std::map<std::string, Mesh*> meshList;			//!< The mesh container.
		std::map<std::string, Skeleton*> skeletonList;	//!< The skeleton container.
		std::map<std::string, Animation*> animationList;//!< The animation container.

        std::string repository;							//!< The repository path.
        std::string defaultTexture;						//!< The default texture name.
        std::string defaultFont;						//!< The default font name.
        std::string defaultShader;						//!< The default shader name.
        std::string defaultMesh;						//!< The default mesh name.
		std::string defaultSkeleton;					//!< The default skeleton name.
		std::string defaultAnimation;					//!< The default animation name.
        //
};
