/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: camera.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/09/2020
----------------------------------------------------------------------------------------------------------*/

#include "camera.h"

/**
* @brief camera constructor
* @param center		center of projection plane
* @param u			horizontal vector of projection plane
* @param v			vertical vector of projection plane
* @param r			distance from projection plane to camera
*/
Camera::Camera( const vec3 & center, const vec3 & u, const vec3 & v, const float r ) :
	center( center ), u( u ), v( v ), r( r )
{
	pos = center + normalize( cross( v, u ) ) * r;
}

/**
* @brief get a random offset in the lens using Alvaro's method
* @return random offset
*/
vec2 Camera::get_rand_lense_point() const
{
	// lense has defined shape
	if ( lense_triangles.empty() == false )
	{
		// area euristic [0,1]
		float area = rand() / static_cast<float>( RAND_MAX );

		float total_area = 0.0f;

		// get random point in the triangle with matching euristic
		for ( const auto& triangle : lense_triangles )
		{
			total_area += triangle.area_euristic;
			if ( total_area >= area )
				return triangle.get_rand_point();
		}
	}

	// lense has no shape

	// random point in a circular lense
	float r_angle = rand() / static_cast< float >( RAND_MAX ) * 2 * glm::pi<float>();
	float r_radius = aperture * sqrt( rand() / static_cast< float >( RAND_MAX ) );
	return { r_radius * cos( r_angle ), r_radius * sin( r_angle ) };
}
