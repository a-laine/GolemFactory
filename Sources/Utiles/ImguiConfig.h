#pragma once


#define USE_IMGUI


#ifdef USE_IMGUI
	#define IMGUI_DEFINE_MATH_OPERATORS
	#include "imgui.h"
	#include "backends/imgui_impl_glfw.h"
	#include "backends/imgui_impl_opengl3.h"

#endif // USE_IMGUI

