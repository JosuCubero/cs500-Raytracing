/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: light.h
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 02/03/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "math_utils.h"

namespace Lights
{
	struct Point
	{
		vec3	pos;
		vec3	color;
		float	radius;
	};

	struct Ambient
	{
		vec3	color;
	};

	struct Air
	{
		float	electric_permitivity{ 1.0f };
		float	magnetic_permeability{ 1.0f };
		vec3	attenuation{1.0f, 1.0f, 1.0f};
	};
}