Map
{
	vertex :   "map.vs";
	geometry : "map.gs";
	fragment : "map.fs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		overrideColor : "vec3";
		exclusion : "ivec4";
	};
}