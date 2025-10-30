#pragma once

#include <GL/glew.h>

#include <string>
#include <iostream>
#include <Utiles/System.h>

//#define CHECK_GL_ERRORS

#ifdef CHECK_GL_ERRORS
inline bool __CheckGLError(const std::string& header, const std::string& label, const std::string& functionName, uint32_t line)
{
	constexpr bool verbose = false;
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::string code = "";
		switch (error)
		{
			case GL_INVALID_ENUM: code = "GL_INVALID_ENUM"; break;
			case GL_INVALID_VALUE: code = "GL_INVALID_VALUE"; break;
			case GL_INVALID_OPERATION: code = "GL_INVALID_OPERATION"; break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: code = "GL_INVALID_FRAMEBUFFER_OPERATION"; break;
			case GL_OUT_OF_MEMORY: code = "GL_OUT_OF_MEMORY"; break;
			case GL_STACK_UNDERFLOW: code = "GL_STACK_UNDERFLOW"; break;
			case GL_STACK_OVERFLOW: code = "GL_STACK_OVERFLOW"; break;
			default: break;
		}
		std::cout << header << " : " << label << std::endl;
		std::cout << functionName << "(line:" << line << ") : " << code << std::endl;
		DebugBreak();
		return true;
	}
	else if (verbose)
	{
		std::cout << "---" << header << " : " << label << std::endl;
	}
	return false;
}
#define CheckGLError(header,label) __CheckGLError(header,label,GF_FUNCTION,__LINE__)
#else
#define CheckGLError(header,label)
#endif // CHECK_GL_ERRORS
