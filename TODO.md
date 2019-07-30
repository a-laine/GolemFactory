   
01000100 01101111 00100000 01101001 01110100 00100000 01110110 01100101
01110010 01111001 00100000 01110111 01100101 01101100 01101100 00100000
01101111 01110010 00100000 01100100 01101111 01101110 00100111 01110100
00100000 01100100 01101111 00100000 01101001 01110100 00100000 00100001

**TODO**

*Micselenious :*
- change parsing gfmesh line from "iss<<" to sscanf_s 
	- for more error file detection
- add a pipeline for continous integration with :
	- compilation test with output with -Wall,
	- code coverage,
	- code metrics,
	- output of cppcheck and/or others static analizis,
	- output of several unitary test. Better if these test are builded using the engine (one exe each)
		- picking,
		- HUD picking,
		- collision test,
		- ...
		

*Widget system :*
- change renderer hud pass to fully iterative rendering on widgets to support hierarchy
- add Widget loader static class
	- choose between iterative pass for loading or recursive (recursive easier to understand but slower)
- add full hud loading from .gui files

*Font generator :*
 - load a font from ttf file and generate texture atlas
 
*SceneManager :*
- change NodeVirtual class for thread safe
- change SceneManager class for thread safe
- add the map structure to populate a huge map with several tree just around the camera.
- add support to change depth of these tree, depending on camera position
- add support of instance hierarchy. two way possible :
	- getInstance function only return parent (and the renderer has to get child by himself)
	- getInstance function return parent and children with all instance have an absolute position/orientation
		  (so the renderer don't have to compute them)

*Instance management :*
- add support of instance hierarchy (parent <-> child)
	- in instance manager or in the instance himself : supported by the entity component system
	- ex: class InstanceContainer : public Component
	- or simpler and easier :
		>class Entity {
		>	private: 
		>		Entity\* parent
		>}

*Physics::Collision*
- change unitary test function to accept a json file with all the test
	- descriptor of shape a
	- descriptor of shape b
	- expected result
- benchmark (statistics) all test in unitary test
- support compound and/or bounding volume hierarchy
hierarchy only example ex:
>std::vector<std::vector<Shape> > bvh
>
>bvh0					// first level of the hierarchy
>'-- Sphere
>bvh2					// second level
>'-- Hull
>bvh3					// last level
>|-- OrientedBox
>|-- OrientedBox
>|-- OrientedBox
>|-- OrientedBox
>|-- OrientedBox
>|-- OrientedBox
>|-- OrientedBox
>|-- OrientedBox
>'-- OrientedBox

*EntityFactory*
- change for entity description from a json game description file
- could be nice to have an Entity pool
- think if it's possible to have an understandable reflection system for script loading










