   
01000100 01101111 00100000 01101001 01110100 00100000 01110110 01100101
01110010 01111001 00100000 01110111 01100101 01101100 01101100 00100000
01101111 01110010 00100000 01100100 01101111 01101110 00100111 01110100
00100000 01100100 01101111 00100000 01101001 01110100 00100000 00100001

**LITTLE TODO**

*Micselenious :*
- Shader, font passer c++11
- change parsing gfmesh line from "iss<<" to sscanf_s 
	- for more error file detection

*Font generator :*
 - load a font from ttf file and generate texture atlas


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


























