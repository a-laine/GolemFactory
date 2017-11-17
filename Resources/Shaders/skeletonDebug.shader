SkeletonDebug
{
	vertex :   "skeletonDebug.vs";
	fragment : "skeletonDebug.fs";
	geometry : "skeletonDebug.gs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		skeletonPose : "mat4 array32";
	};
}