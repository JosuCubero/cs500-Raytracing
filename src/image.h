/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: image.h
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/13/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "math_utils.h"
#include <vector>

namespace Image
{
	void save_image( const char* filepath, const int width, const int height, std::vector<unsigned char>& data );
}
