Triangle
{
	vertex :   "Shapes/point.vs";
	geometry : "Shapes/triangle.gs";
	fragment : "Shapes/triangle.fs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		vector1 : "vec3";
		vector2 : "vec3";
		
		wired : "int";
		overrideColor : "vec3";
	};
}