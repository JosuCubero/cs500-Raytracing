/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: raytracer.h
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/13/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "scene.h"

#include "intersection.h"
#include "math_utils.h"
#include <vector>

struct Configuration
{
	int		depth;					// maximum ray reflections / refractions
	int		width, height;			// screen resolution
	int		antialiasing_samples;	// total samples for antialiasing
	bool	adaptive_antialiasing;	// flag for adaptive sampling
	int		shadow_samples;			// total samples for shadows
	bool	dof;					// flag for dof
	int		dof_samples;			// total samples for depth of field
	int		reflection_samples;		// total samples for reflection roughness
	bool	window;					// flag for window preview

	float epsilon;				// epsilon value
};

namespace Raytracer
{
	bool trace_scene( std::vector<unsigned char>& color_buffer, const Scene& scene, const Configuration& config );
}