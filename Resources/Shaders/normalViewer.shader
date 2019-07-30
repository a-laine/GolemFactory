NormalViewer{
	vertex :   "normalViewer.vs";
	geometry : "normalViewer.gs";
	fragment : "normalViewer.fs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		overrideColor : "vec3";
	};
}