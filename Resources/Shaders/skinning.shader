Skinning
{
	vertex :   "skinning.vs";
	fragment : "skinning.fs";
	
	//  Geometry shader
	//  Tessellation evaluation shader
	//  Tessellation control shader
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		skeletonPose : "mat4 array32";
		inverseBindPose : "mat4 array32";
	};
}