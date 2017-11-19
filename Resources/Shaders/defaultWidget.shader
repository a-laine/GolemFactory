Default{
	vertex :   "defaultWidget.vs";
	fragment : "defaultWidget.fs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		color : "vec4";
		useTexture : "int"
	};
}