grassGrid {
	vertex :   "grassGrid.vs";
	fragment : "grassGrid.fs";
	geometry : "grassGrid.gs";
	evaluation : "grassGrid.tes";
	control : "grassGrid.tcs";
	
	uniform :
	{
		model : "mat4";
		view : "mat4";
		projection : "mat4";
	};
}