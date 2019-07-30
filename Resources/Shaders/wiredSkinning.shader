WiredSkinning
{
	vertex :   "wiredSkinning.vs";
	fragment : "wiredSkinning.fs";
	geometry : "wiredSkinning.gs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
		skeletonPose : "mat4 array32";
		inverseBindPose : "mat4 array32";
	};
}