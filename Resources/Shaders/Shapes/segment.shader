Segment
{
	vertex :   "Shapes/point.vs";
	geometry : "Shapes/segment.gs";
	fragment : "Shapes/point.fs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		
		vector : "vec3";
		
		overrideColor : "vec3";
	};
}