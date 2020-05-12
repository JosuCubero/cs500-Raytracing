/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: shapes.h
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/07/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "intersection.h"
#include "material.h"
#include "math_utils.h"
#include <array>
#include <vector>

namespace Shapes
{
	struct Ray
	{
		vec3	pos;
		vec3	dir;

		Ray() = default;
		Ray( const vec3& pos, const vec3& dir );
	};

	struct Shape
	{
		virtual Intersection::Contact intersect( const Ray& ray ) const = 0;
		Material material;
	};

	struct Plane
	{
		vec3 point;
		vec3 normal;

		Plane() = default;
		Plane( const vec3& point, const vec3& normal );

		float intersect( const Ray& ray ) const;
	};

	struct Triangle
	{
		vec3 a, b, c;
		vec3 normal;

		Triangle() = default;
		Triangle( const vec3& a, const vec3& b, const vec3& c );
		
		float intersect( const Ray& ray, vec3& point  ) const;
		bool is_point_inside( const vec3& point ) const;
	};

	struct LenseTriangle
	{
		vec2 a, b, c;
		float area_euristic;

		vec2 get_rand_point() const;
	};

	struct Sphere : public Shape
	{
		vec3	pos;
		float	radius;

		Intersection::Contact intersect( const Ray& ray ) const;
	};

	struct Box : public Shape
	{
		vec3 pos;
		vec3 length, width, height;
		std::array<Plane, 6> planes;

		enum class plane { front, back, left, right, bottom, top };

		void generate_planes();

		Intersection::Contact intersect( const Ray& ray ) const;
	};

	struct Polygon : public Shape
	{
		std::vector<vec3> vertices;
		vec3 normal;

		Intersection::Contact intersect( const Ray& ray ) const;
	};

	struct Ellipsoid : public Shape
	{
		vec3 pos;
		vec3 u, v, w;
		mat3 inv_model;

		Intersection::Contact intersect( const Ray& ray ) const;
	};

	struct Mesh : public Shape
	{
		std::vector<vec3>	vertices;
		std::vector<ivec3>	indices;


		Box bounding_volume;

		void compute_bv();
		Intersection::Contact intersect( const Ray& ray ) const;
	};
}
