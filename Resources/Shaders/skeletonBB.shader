SkeletonBB
{
	vertex :   "skeletonBB.vs";
	fragment : "skeletonBB.fs";
	geometry : "skeletonBB.gs";
		
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		skeletonPose : "mat4 array32";
	};
}