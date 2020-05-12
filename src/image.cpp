/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: image.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/13/2020
----------------------------------------------------------------------------------------------------------*/

#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "image\stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "image\stb_image_write.h"

#include <iostream>

namespace Image
{
	/**
	* @brief save the color buffer into an image file
	* @param filepath	path to the output file
	* @param width		width of the image
	* @param height		height of the image
	* @param data		color data
	*/
	void save_image( const char* filepath, const int width, const int height, std::vector<unsigned char>& data )
	{
		std::vector<char> img_data;

		// write into the file
		stbi_write_png( filepath, width, height, 3, static_cast<void*>( &data[0] ), width * 3 );
	}
}
