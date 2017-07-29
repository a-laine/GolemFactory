   
01000100 01101111 00100000 01101001 01110100 00100000 01110110 01100101
01110010 01111001 00100000 01110111 01100101 01101100 01101100 00100000
01101111 01110010 00100000 01100100 01101111 01101110 00100111 01110100
00100000 01100100 01101111 00100000 01101001 01110100 00100000 00100001

**LITTLE TODO**

*Micselenious :*
- Shader, font passer c++11
- add static parameter in resourcesVirtual for verbose output levels
- change parsing gfmesh line from "iss<<" to sscanf_s 
	- for more error file detection

*Font generator :*
 - load a font from ttf file and generate texture atlas
 - 


*Scene manager :*
- change NodeVirtual* sons[] container list into a thread safe forward list.
- add an optional NodeVirtual* adoptedSons[] thread safe forward list for optionnal sub tree.
- add the map structure to populate a huge map with several tree just around the camera.
- add support to change depth of these tree, depending on camera position
- change this class to make it totaly thread safe !
- add support of instance hierarchy. two way possible :
	- getInstance function only return parent (and the renderer has to get child by himself)
	- getInstance function return parent and children with all instance have an absolute position/orientation
		  (so the renderer don't have to compute them)

*Instance manager :*
- add support of instance hierarchy (parent <-> child)

**ANIMATION MODULE (DESCRIPTION)**

 - this define the "Action state machine" described in game architecture (this holly bible book!) p604
   the blend part is performed later in InstanceAnimatable class.
   all state machine are simple string or string looped on themself.

Animation manager:

 - for more  information all function described here define the "animation pinepline" described in game architecture p604

design a base class for AnimationManager from a clearly defined task list
	- two function to add and remove InstanceAnimatable*
	- clasify all InstanceAnimatable* by group of refresh rate
	- one function for update all instances of a group
		+ example of behavour for this function:
			for each instance of the group to animate:
			{
				call the function animate(elapsedTime)
				call a function of the physics engine to correct the skeleton pose (depending on terrain, or environment for example)
				// in a ideal world it is in the physics engine that the instance position and speed are updated (depending on contact point with ground, etc...)
			}

 - for the "Animation controllers" described in book it's performed here by the InteligenceManager class (which regroup the behaviour of instances)

 - with this structure we can have:
	- standard animation such as dancing (all joints defined as revelant in the animation, animation defined as looped)
	- animation blending like shoot during running (because the target pose is defined via target joint and animation definition permit combination)
	- standard animation sequence such as seat down and just after drink a beer (seat down animation defined as flaged, when the flag is rised the InteligenceManager can launch a new animation: drink)
	- procedural animation such as climb a wall, or stairs (because the InteligenceManager can specify special pose directly: 
	  when a climbing hold is reached compute the next skeleton pose with a inverse kinematics module and blend it directly as flaged)
	- have crowd animation (each instance can have a refresh rate depending on its distance to camera)

















