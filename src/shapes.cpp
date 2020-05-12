/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: shapes.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/07/2020
----------------------------------------------------------------------------------------------------------*/

#include "shapes.h"

namespace Shapes
{
	//--------------- PLANE ------------------//

	/**
	* @brief plane constructor
	*/
	Plane::Plane( const vec3& point, const vec3& normal ) :
		point( point ), normal( normal )
	{}

	/**
	* @brief compute the intersection between a ray and a plane
	* @param ray	the ray
	* @return time of the intersection
	*/
	float Plane::intersect( const Ray& ray ) const 
	{
		float div = dot( ray.dir, normal );

		// no collision
		if ( div == 0.0f ) return -1.0f;

		// -(P-C)*n/(d*n)
		return -dot( ray.pos - point, normal ) / div;
	}




	//--------------- TRIANGLE ---------------//

	/**
	* @brief triangle constructor
	*/
	Triangle::Triangle( const vec3& a, const vec3& b, const vec3& c ) :
		a( a ), b( b ), c( c )
	{
		normal = normalize( cross( b - a, c - a ) );
	}

	/**
	* @brief compute the intersection between a ray and a triangle
	* @param ray	the ray
	* @param point		point of intersection (return)
	* @return time of the intersection
	*/
	float Triangle::intersect( const Ray& ray, vec3& point ) const
	{
		float div = dot( normal, ray.dir );

		// check
		if ( glm::abs( div ) < 0.01f )
			return -1.0f;

		// time of collision
		float time = ( dot( normal, a ) - dot( normal, ray.pos ) ) / div;

		// check
		if ( time < 0.0f )
			return -1.0f;

		point = ray.pos + time * ray.dir;

		// out of bounds
		if ( is_point_inside( point ) == false )
			return -1.0f;

		return time;
	}

	/**
	* @brief check if the point is inside the boundaries of the polygon
	* @param point
	* @return true if it is inside
	*/
	bool Triangle::is_point_inside( const vec3& point ) const
	{
		vec3 v0 = b - a;
		vec3 v1 = c - a;
		vec3 v2 = point - a;

		float v0v0 = dot( v0, v0 );
		float v0v1 = dot( v0, v1 );
		float v1v1 = dot( v1, v1 );
		float uv0 = dot( v2, v0 );
		float uv1 = dot( v2, v1 );

		float div = v0v0 * v1v1 - v0v1 * v0v1;

		// check
		if ( div == 0 )
			return false;

		vec3 coordinates;

		// set the coordinates result
		coordinates.y = ( v1v1 * uv0 - v0v1 * uv1 ) / div;
		coordinates.z = ( v0v0 * uv1 - v0v1 * uv0 ) / div;
		coordinates.x = 1 - coordinates.y - coordinates.z;

		// check boundaries
		if ( coordinates.x < 0 || coordinates.y < 0 || coordinates.z < 0 )
			return false;

		return true;
	}




	//--------------- LENSE TRIANGLE -----------------//

	/**
	* @brief get a random uniformly distributed point in the triangle
	*/
	vec2 LenseTriangle::get_rand_point() const
	{
		float r1 = rand() / static_cast<float>( RAND_MAX );
		float r2 = rand() / static_cast<float>( RAND_MAX );
		r1 = sqrt( r1 );

		vec2 result = ( 1.0f - r1 ) * a + r1 * ( 1.0f - r2 ) * b + r1 * r2 * c;

		return result;
	}




	//--------------- SPHERE -----------------//

	/**
	* @brief compute the intersection between a ray and a sphere
	* @param ray	the ray
	* @return contact information of the intersection
	*/
	Intersection::Contact Sphere::intersect( const Ray& ray ) const 
	{
		vec3 v = ray.pos - pos;

		float a = dot( ray.dir, ray.dir );
		float b = 2.0f * dot( ray.dir, v );
		float c = dot( v, v ) - radius * radius;

		float disc = b * b - 4.0f * a * c;

		if ( disc < 0.0f )
			return Intersection::Contact();

		float t;

		// compute the time values for the intersection
		float t1 = ( -b + sqrt( disc ) ) / ( 2 * a );
		float t2 = ( -b - sqrt( disc ) ) / ( 2 * a );

		// the sphere is behind
		if ( t1 < 0.0f )
			return Intersection::Contact();
		// the ray starts inside
		else if ( t2 < 0.0f )
			t = t1;
		// the sphere is in front
		else
			t = t2;

		// return the intersection value
		vec3 point = ray.pos + t * ray.dir;
		vec3 normal = normalize( point - pos );
		Intersection::Contact contact( t, point, normal, material );
		return contact;
	}




	//--------------- BOX --------------------//

	/**
	* @brief generate the planes of the boxel
	*/
	void Box::generate_planes()
	{
		planes[static_cast< unsigned >( plane::front )]		= Plane( pos,			normalize( cross( length, height ) ) );
		planes[static_cast< unsigned >( plane::back )]		= Plane( pos + width,	normalize( cross( height, length ) ) );
		planes[static_cast< unsigned >( plane::left )]		= Plane( pos,			normalize( cross( height, width  ) ) );
		planes[static_cast< unsigned >( plane::right )]		= Plane( pos + length,	normalize( cross( width,  height ) ) );
		planes[static_cast< unsigned >( plane::bottom )]	= Plane( pos,			normalize( cross( width,  length ) ) );
		planes[static_cast< unsigned >( plane::top )]		= Plane( pos + height,	normalize( cross( length, width  ) ) );
	}

	/**
	* @brief compute the intersection between a ray and a box
	* @param ray	the ray
	* @return contact information of the intersection
	*/
	Intersection::Contact Box::intersect( const Ray& ray ) const 
	{
		const float min = 0.0f;
		const float max = std::numeric_limits<float>::max();
		float t_min = min;
		float t_max = max;

		vec3 point_min;
		vec3 normal_min;
		vec3 point_max;
		vec3 normal_max;

		for ( unsigned i = 0u; i < 6u; i++ )
		{
			const auto& plane = planes[i];

			float dot_normal = dot( ray.dir, plane.normal );

			// check front face or backface and update the values of t
			if ( dot_normal < 0.0f )
			{
				float t = plane.intersect( ray );
				if ( t > t_min )
				{
					t_min = t;
					point_min = ray.pos + t * ray.dir;
					normal_min = plane.normal;
				}
			}
			else if ( dot_normal > 0.0f )
			{
				float t = plane.intersect( ray ); 
				if ( t < t_max )
				{
					t_max = t;
					point_max = ray.pos + t * ray.dir;
					normal_max = plane.normal;
				}
			}
			else if ( dot( ray.pos - plane.point, plane.normal ) > 0.0f )
				return Intersection::Contact();
		}

		if ( t_max >= t_min )
		{
			if ( t_min == min )
				return Intersection::Contact( t_max, point_max, normal_max, material );
			else
				return Intersection::Contact( t_min, point_min, normal_min, material );
		}

		return Intersection::Contact();
	}




	//--------------- POLYGON ----------------//

	/**
	* @brief compute the intersection between a ray and a polygon
	* @param ray	the ray
	* @return contact information of the intersection
	*/
	Intersection::Contact Polygon::intersect( const Ray& ray ) const
	{
		vec3 a = vertices[0u];

		for ( unsigned i = 1u; i < vertices.size() - 1u; i++ )
		{
			Triangle triangle( a, vertices[i], vertices[i + 1u] );
			vec3 point;
			float time = triangle.intersect( ray, point );

			if ( time != -1.0f )
				return Intersection::Contact( time, point, normal, material );
		}
			

		return Intersection::Contact();
	}




	//--------------- ELLIPSOID --------------//

	/**
	* @brief compute the intersection between a ray and an ellipsoid
	* @param ray	the ray
	* @return contact information of the intersection
	*/
	Intersection::Contact Ellipsoid::intersect( const Ray& ray ) const
	{
		vec3 p0 = inv_model * ( ray.pos - pos );
		vec3 ray_dir = inv_model * ray.dir;

		float a = dot( ray_dir, ray_dir );
		float b = 2.0f * dot( p0, ray_dir );
		float c = dot( p0, p0 ) - 1.0f;

		float disc = b * b - 4.0f * a * c;

		if ( disc < 0.0f )
			return Intersection::Contact();

		float t;

		// compute the time values for the intersection
		float t1 = ( -b + sqrt( disc ) ) / ( 2.0f * a );
		float t2 = ( -b - sqrt( disc ) ) / ( 2.0f * a );

		// the sphere is behind
		if ( t1 < 0.0f )
			return Intersection::Contact();
		// the ray starts inside
		else if ( t2 < 0.0f )
			t = t1;
		// the sphere is in front
		else
			t = t2;

		// intersection values
		vec3 point_unit = p0 + t * ray_dir;
		vec3 normal = normalize( transpose( inv_model ) * point_unit );
		vec3 point = ray.pos + t * ray.dir;

		Intersection::Contact contact( t, point, normal, material );
		return contact;
	}




	//--------------- MESH --------------------//

	/**
	* @brief generate the bounding volume of the mesh
	*/
	void Mesh::compute_bv()
	{
		vec3 min(  std::numeric_limits<float>::max() );
		vec3 max( -std::numeric_limits<float>::max() );

		for ( const auto vertex : vertices )
		{
			// min
			min.x = vertex.x < min.x ? vertex.x : min.x;
			min.y = vertex.y < min.y ? vertex.y : min.y;
			min.z = vertex.z < min.z ? vertex.z : min.z;

			// max
			max.x = vertex.x > max.x ? vertex.x : max.x;
			max.y = vertex.y > max.y ? vertex.y : max.y;
			max.z = vertex.z > max.z ? vertex.z : max.z;
		}

		bounding_volume.pos = vec3( min.x, min.y, min.z );

		bounding_volume.width	= vec3( max.x - min.x, 0.0f, 0.0f );
		bounding_volume.height	= vec3( 0.0f, max.y - min.y, 0.0f );
		bounding_volume.length	= vec3( 0.0f, 0.0f, max.z - min.z );
		bounding_volume.generate_planes();
	}

	/**
	* @brief compute the intersection between a ray and a mesh
	* @param ray	the ray
	* @return contact information of the intersection
	*/
	Intersection::Contact Mesh::intersect( const Ray& ray ) const
	{
		// check collision with bounding volume
		Intersection::Contact contact_bv = bounding_volume.intersect( ray );

		if ( contact_bv.time == -1.0f )
			return Intersection::Contact();
		//else
		//	return contact_bv;


		// check collision with mesh
		vec3 point;
		vec3 normal;
		float time = -1.0f;

		// check collision against each triangle in the mesh
		for ( unsigned i = 0u; i < indices.size(); i++ )
		{
			vec3 point_curr;
			vec3 normal_curr = normalize( cross( vertices[indices[i][1]] - vertices[indices[i][0]], vertices[indices[i][2]] - vertices[indices[i][0]] ) );

			Triangle triangle( vertices[indices[i][0]], vertices[indices[i][1]], vertices[indices[i][2]] );
			float time_curr = triangle.intersect( ray, point_curr );

			if ( time_curr != -1.0f && ( time == -1.0f || time_curr < time ) )
			{
				point = point_curr;
				normal = normal_curr;
				time = time_curr;
			}
		}
		return Intersection::Contact( time, point, normal, material );
	}




	//--------------- RAY --------------------//

	/**
	* @brief ray constructor
	* @param pos	start point of the ray
	* @param dir	direction of the ray
	*/
	Ray::Ray( const vec3& pos, const vec3& dir )  :
		pos( pos ), dir( dir )
	{}
	
}

