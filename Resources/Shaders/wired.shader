Wired{
	vertex :   "wired.vs";
	fragment : "wired.fs";
	geometry : "wired.gs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		overrideColor : "vec3";
	};
}