#include <iostream>
#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Utiles/Singleton.h"
#include "Utiles/Mutex.h"
#include "NodeVirtual.h"




class SceneManager : public Singleton<ResourceManager>
{
	friend class Singleton<ResourceManager>;

	public:
		//  Public functions

		//

		//  Set/get functions
		void getInstanceList(std::vector<std::pair<int, InstanceVirtual*> >& list);
		//

	private:
		//  Default
		SceneManager();		//!< Default constructor.
		~SceneManager();	//!< Default destructor.
		//

		//  Attributes
		std::vector<NodeVirtual*> world;
		//
};
