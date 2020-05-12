/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: intersection.h
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/07/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "material.h"
#include "math_utils.h"

namespace Intersection
{

	struct Contact
	{
		float		time;
		vec3		point;
		vec3		normal;
		Material	material;


		Contact() { time = -1.0f; }
		Contact( const float time, const vec3& point, const vec3& normal, const Material& material )
			: time( time ), point( point ), normal( normal ), material( material ) {}
	};
}