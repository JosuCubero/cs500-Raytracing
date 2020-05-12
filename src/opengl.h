/* Start Header -------------------------------------------------------
Copyright (C) 20xx DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.

File Name: opengl.h
Purpose: Detect any error when calling an OpenGl function
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 09/20/2019
- End Header --------------------------------------------------------*/

#pragma once
#include <glad\glad.h>
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>

#include <vector>
#include <iostream>


// macros
#if _WIN32
#define XBREAK() __debugbreak()
#else
#define XBREAK() __builtin_trap()
#endif

#define __STRINGIZE__1(x) #x
#define __STRINGIZE__2(x) __STRINGIZE__1(x)
#define __LOCATION__ __FILE__ " : " __STRINGIZE__2(__LINE__)
#define XASSERT(what) if(!(what)){ std::cerr << "Assert failed: "#what; XBREAK(); throw std::runtime_error("Assert failed at " __LOCATION__);}
#define XERROR(what) {std::cerr << "Error: " << what; XBREAK(); throw std::runtime_error("Error at " __LOCATION__); }

#define GL_CALL(what) {what; checkGlError();}



inline void checkGlError()
{
	const auto iErr = glGetError();
	if (iErr != GL_NO_ERROR){
		const char* pszError;
		switch (iErr){
			case GL_INVALID_ENUM:{
				pszError = "GL_INVALID_ENUM";
				break;
			}
			case GL_INVALID_VALUE:{
				pszError = "GL_INVALID_VALUE";
				break;
			};
			case GL_INVALID_OPERATION:{
				pszError = "GL_INVALID_OPERATION";
				break;
			};
			case GL_INVALID_FRAMEBUFFER_OPERATION:{
				pszError = "GL_INVALID_FRAMEBUFFER_OPERATION";
				break;
			};
			case GL_OUT_OF_MEMORY:{
				pszError = "GL_OUT_OF_MEMORY";
				break;
			};
			#if defined(GL_STACK_UNDERFLOW) && defined(GL_STACK_OVERFLOW)
			case GL_STACK_UNDERFLOW:{
				pszError = "GL_STACK_UNDERFLOW";
				break;
			};
			case GL_STACK_OVERFLOW:{
				pszError = "GL_STACK_OVERFLOW";
				break;
			};
			#endif
			default:{pszError = "Unknown";}
		}
		XERROR(pszError);
	}
}

std::vector<glm::vec<4, uint8_t>> take_screenshoot(uint32_t width, uint32_t height);
std::vector<glm::vec<4, uint8_t>> save_screenshoot(uint32_t width, uint32_t height, const char* filename);

