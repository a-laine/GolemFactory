Tree
{
	vertex :   "tree.vs";
	fragment : "tree.fs";
	
	//  Geometry shader
	//  Tessellation evaluation shader
	//  Tessellation control shader
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		wind : "vec4";
	};
}