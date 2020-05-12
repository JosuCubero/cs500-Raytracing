/* Start Header -------------------------------------------------------
Copyright (C) 20xx DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written consent of
DigiPen Institute of Technology is prohibited.

File Name: opengl.cpp
Purpose: Detect any error when calling an OpenGl function
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 09/20/2019
- End Header --------------------------------------------------------*/

#include "opengl.h"
#include <glad\glad.h>

#include <iostream>

namespace{
	
	
	/**
	 * OpenGL callback for debugging
	 * @param source
	 * @param type
	 * @param id
	 * @param severity
	 * @param length
	 * @param message
	 * @param userParam
	 */
	void APIENTRY openglCallbackFunction(GLenum source,
										 GLenum type,
										 GLuint id,
										 GLenum severity,
										 GLsizei length,
										 const GLchar* message,
										 const void* userParam)
	{
		( void )source;
		( void )length;
		( void )userParam;

		std::cout << "{\n";
		std::cout << "\tmessage: " << message << "\n";
		std::cout << "\ttype: ";
		switch (type){
			case GL_DEBUG_TYPE_ERROR: std::cout << "ERROR";
				break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "DEPRECATED_BEHAVIOR";
				break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: std::cout << "UNDEFINED_BEHAVIOR";
				break;
			case GL_DEBUG_TYPE_PORTABILITY: std::cout << "PORTABILITY";
				break;
			case GL_DEBUG_TYPE_PERFORMANCE: std::cout << "PERFORMANCE";
				break;
			case GL_DEBUG_TYPE_OTHER: std::cout << "OTHER";
				break;
			default:break;
		}
		std::cout << "\n";

		std::cout << "\tid: " << id << "\n";
		std::cout << "\tseverity: ";
		switch (severity){
			case GL_DEBUG_SEVERITY_LOW: std::cout << "LOW";
				break;
			case GL_DEBUG_SEVERITY_MEDIUM: std::cout << "MEDIUM";
				break;
			case GL_DEBUG_SEVERITY_HIGH: std::cout << "HIGH";
				break;
			case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "NOTIFICATION";
				break;
			default:break;
		}
		std::cout << "\n}\n";
		XASSERT(type != GL_DEBUG_TYPE_ERROR);
	}
}

/**
 *
 */
void setup_gl_debug()
{

	// Debug
	GL_CALL(glEnable(GL_DEBUG_OUTPUT));
	GL_CALL(glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS));
	GL_CALL(glDebugMessageCallback(openglCallbackFunction, nullptr));
	GLuint unusedIds = 0;
	GL_CALL(glDebugMessageControl(GL_DONT_CARE,
								  GL_DONT_CARE,
								  GL_DONT_CARE,
								  0,
								  &unusedIds,
								  GL_TRUE));
}

/**
 * Takes an screenshot
 * @param width
 * @param height
 * @return
 */
std::vector<glm::vec<4, uint8_t>> take_screenshoot(uint32_t width, uint32_t height)
{
	std::vector<glm::vec<4, uint8_t>> result;
	result.resize(width * height, {});
	GL_CALL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
	GL_CALL(glReadBuffer(GL_FRONT));
	GL_CALL(glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, result.data()));
	return result;
}

/**
 * Saves an screenshoot to a BMP
 * @param width
 * @param height
 * @param filename
 * @return
 */
std::vector<glm::vec<4, uint8_t>> save_screenshoot(uint32_t width, uint32_t height, const char* filename)
{
	auto pixels = take_screenshoot(width, height);
	FILE* file; 
	fopen_s( &file, filename, "wb" );

	//#ifndef _WIN32
	typedef struct                       /**** BMP file header structure ****/
	{
		unsigned short bfType;           /* Magic number for file */
		unsigned int   bfSize;           /* Size of file */
		unsigned short bfReserved1;      /* Reserved */
		unsigned short bfReserved2;      /* ... */
		unsigned int   bfOffBits;        /* Offset to bitmap data */
	}   BITMAPFILEHEADER;

	typedef struct                       /**** BMP file info structure ****/
	{
		unsigned int   biSize;           /* Size of info header */
		int            biWidth;          /* Width of image */
		int            biHeight;         /* Height of image */
		unsigned short biPlanes;         /* Number of color planes */
		unsigned short biBitCount;       /* Number of bits per pixel */
		unsigned int   biCompression;    /* Type of compression to use */
		unsigned int   biSizeImage;      /* Size of image data */
		int            biXPelsPerMeter;  /* X pixels per meter */
		int            biYPelsPerMeter;  /* Y pixels per meter */
		unsigned int   biClrUsed;        /* Number of colors used */
		unsigned int   biClrImportant;   /* Number of important colors */
	}   BITMAPINFOHEADER;
	//#endif

	// Info header
	BITMAPINFOHEADER bitmap_info_header = {};
	bitmap_info_header.biSize        = sizeof(BITMAPINFOHEADER);
	bitmap_info_header.biSizeImage   = width * height * 4;
	bitmap_info_header.biWidth       = width;
	bitmap_info_header.biHeight      = height;
	bitmap_info_header.biPlanes      = 1;
	bitmap_info_header.biBitCount    = 32;
	bitmap_info_header.biCompression = 0;
	bitmap_info_header.biSizeImage   = width * height * 4;

	int32_t bits_offset = sizeof(BITMAPFILEHEADER) + bitmap_info_header.biSize;
	int32_t image_size  = bitmap_info_header.biSizeImage;
	int32_t file_size   = bits_offset + image_size;

	// File header
	BITMAPFILEHEADER bitmap_file_header = {};
	bitmap_file_header.bfType      = 'B' + ('M' << 8);
	bitmap_file_header.bfOffBits   = static_cast<uint32_t>(bits_offset);
	bitmap_file_header.bfSize      = static_cast<uint32_t>(file_size);
	bitmap_file_header.bfReserved1 = bitmap_file_header.bfReserved2 = 0;

	// Data
	fwrite(&bitmap_file_header, 1, sizeof(BITMAPFILEHEADER), file);
	fwrite(&bitmap_info_header, 1, sizeof(BITMAPINFOHEADER), file);
	fwrite(pixels.data(), 1, static_cast<size_t>(image_size), file);

	fclose(file);
	return pixels;
}
