Wired{
	vertex :   "wiredInstanced.vs";
	fragment : "wired.fs";
	geometry : "wired.gs";
	
	uniform :
	{
		model : "mat4 array32";
		view : "mat4";
		projection : "mat4";
	};
}