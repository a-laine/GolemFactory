Point
{
	vertex :   "Shapes/point.vs";
	geometry : "Shapes/point.gs";
	fragment : "Shapes/point.fs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		overrideColor : "vec3";
	};
}