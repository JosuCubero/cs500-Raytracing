/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: camera.h
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/09/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "math_utils.h"
#include "shapes.h"

#include <vector>

struct Camera
{
	vec3	pos;
	vec3	center;
	vec3	u; // up vector
	vec3	v; // right vector
	vec3	w;	// forward vector
	float	r;

	float	aperture;
	float	focal_point;
	float	refraction_index;
	float	r1;
	float	r2;

	std::vector<Shapes::LenseTriangle>	lense_triangles;

	Camera() = default;
	Camera( const vec3& center, const vec3& u, const vec3& v, const float r );
	vec2 get_rand_lense_point() const;


};