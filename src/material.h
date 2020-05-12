/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_josu.cubero_2
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/15/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "math_utils.h"

struct Material
{
	vec3	diffuse_color;
	float	specular_reflection;
	float	specular_exponent;
	vec3	attenuation;
	float	electric_permittivity;
	float	magnetic_permeability;
	float	roughness;
};