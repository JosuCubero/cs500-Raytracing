/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: raytracer.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/13/2020
----------------------------------------------------------------------------------------------------------*/

#include "raytracer.h"
#include "window.h"

#include <glm/gtc/random.hpp>
#include <list>
#include <iostream>
#include <random>
#include <thread>

namespace Raytracer
{
	void					trace_chunk				( std::vector<unsigned char>& color_buffer, const Scene& scene, const Configuration& config, const int thread_count, const int thread_id, const bool* terminate );
	vec3					adaptive_sampling		( const Scene& scene, const Configuration& config, const vec3 center, const vec3 sample_offset_x, const vec3 sample_offset_y, const int depth = 0 );
	vec3					compute_pixel			( const Scene& scene, const Shapes::Ray& ray, const Configuration& config, const float e_permittivity, const float m_permeability, const int depth = 0 );

	Intersection::Contact	raycast_scene			( const Scene& scene, const Shapes::Ray& ray );
	vec3					raycast_lights			( const Scene& scene, const Shapes::Ray& ray, const int samples, const vec3 contact_point, const vec3 contact_normal, const Material& material );

	vec3					get_random_sample		( const vec3& pos, const float radius );
	float					compute_reflection_coeff( const float eps_i, const float nu_i, const float eps_t, const float nu_t, const float incident_angle );
	Shapes::Ray				compute_ray_dir_dof		( const Camera& camera, const vec3& pixel_pos, const float focal_point );
	float					compute_focal_point		( const Camera& camera, const float axis_offset );

	/**
	* @brief compute the color value of each pixel of the output buffer by throwing rays
	* @param color_buffer		result color buffer in chars
	* @param scene				scene to render
	* @param config				raytracer properties
	*/
	bool trace_scene( std::vector<unsigned char>& color_buffer, const Scene& scene, const Configuration& config )
	{

		// resize buffer
		color_buffer.resize( config.height * config.width * 3 );
		for ( unsigned i = 0; i < color_buffer.size(); i += 3 )
		{
			color_buffer[i] = 240;
			color_buffer[i + 1] = 145;
			color_buffer[i + 2] = 0;
		}

		// generate window if enabled
		Window window;
		if ( config.window == true )
			window.initialize( config.width, config.height, color_buffer );

		// color buffer
		std::vector<char> buffer( config.height * config.width );

		// threads
		unsigned thread_count = std::thread::hardware_concurrency() - 1u;
		if ( thread_count == 0u ) thread_count = 1u;

		std::vector<std::thread*>	threads;
		bool terminate = false;

		// raytrace
		for ( unsigned i = 0u; i < thread_count; i++ )
			threads.push_back( new std::thread( trace_chunk, std::ref( color_buffer ), std::ref( scene ), config, thread_count, i, &terminate ) );

		// rendering
		if ( config.window == true )
		{
			// wait for closing window
			while ( window.should_close() == false )
				window.render( color_buffer );
	
			// flag to end threads
			terminate = true;
		}

		// delete threads
		for ( auto thread = threads.begin(); thread != threads.end(); thread++ )
		{
			if ( ( *thread ) != nullptr )
			{
				( *thread )->join();
				delete ( *thread );
			}
		}
	
		// remove window
		if ( config.window == true )
			window.exit();

		return true;
	}

	/**
	* @brief compute the color value of the pixel assigned to the thread
	* @param color_buffer		result color buffer in chars
	* @param scene				scene to render
	* @param config				raytracer values
	* @param thread_count		maximum amount of threads
	* @param thread_id			id of the current thread
	*/
	void trace_chunk( std::vector<unsigned char>& color_buffer, const Scene& scene, const Configuration& config, const int thread_count, const int thread_id, const bool* terminate )
	{
		// get camera
		Camera camera = scene.camera();

		const float half_width  = static_cast<float>( config.width ) / 2.0f;
		const float half_height = static_cast<float>( config.height ) / 2.0f;

		const int pixel_size = static_cast<int>( sqrt( config.antialiasing_samples ) );
		const float half_pixel_size = static_cast<float>( pixel_size ) / 2.0f;
		const vec3  half_pixel_width  = camera.u / static_cast<float>( config.width ) / 2.0f;
		const vec3  half_pixel_height = camera.v / static_cast<float>( config.height ) / 2.0f;

		for ( int i = thread_id; i < config.height; i += thread_count )
		{
			// compute the y value for the current row
			vec3 y = ( static_cast<float>( i ) - half_height + 0.5f ) / half_height * camera.v;


			for ( int j = 0; j < config.width; j++ )
			{
				vec3 color( 0.0f );

				// compute the x value for the current column
				vec3 x = ( static_cast<float>( j ) - half_width + 0.5f ) / half_width * camera.u;


				// spherical aberration
				vec2 axis_offset = {
					( static_cast<float>( j ) - half_width  + 0.5f ) / half_width,
					( static_cast<float>( i ) - half_height + 0.5f ) / half_height
				};
				float axis_dist = sqrt( axis_offset.x * axis_offset.x + axis_offset.y * axis_offset.y ) * camera.aperture;
				float focal_point = compute_focal_point( camera, axis_dist );

				// adaptive antialiasing
				if ( config.adaptive_antialiasing == true )
				{
					vec3 pixel_pos = x - y + camera.center;
					color = adaptive_sampling( scene, config, pixel_pos, half_pixel_width / 2.0f, half_pixel_height / 2.0f );
				}
				else	// supersampling antialiasing
				{
					// add offset inside pixel
					for ( int k = 0; k < pixel_size; k++ )
					{
						vec3 pixel_y = ( static_cast<float>( k ) - half_pixel_size + 0.5f ) / half_pixel_size * half_pixel_height;

						for ( int l = 0; l < pixel_size; l++ )
						{
							vec3 pixel_x = ( static_cast<float>( l ) - half_pixel_size + 0.5f ) / half_pixel_size * half_pixel_width;

							// create the ray for the current pixel
							vec3 pixel_pos = x + pixel_x - y - pixel_y + camera.center;

							for ( int m = 0; m < config.dof_samples; m++ )
							{
								Shapes::Ray ray;
								if ( m == 0 )
								{
									ray.pos = camera.pos;
									ray.dir = normalize( pixel_pos - camera.pos );
								}
								else
								{
									ray = compute_ray_dir_dof( camera, pixel_pos, focal_point );
								}

								// compute the value of the pixel and set it to the buffer
								color += compute_pixel( scene, ray, config, scene.air().electric_permitivity, scene.air().magnetic_permeability );
							}
						}
					}
					// color average
					color /= ( config.antialiasing_samples * config.dof_samples );
				}

				color = clamp( color, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f } );

				// transform the color buffer from float to char
				color_buffer[( i * config.width + j ) * 3]		= static_cast<char>( color.x * 255.99f );
				color_buffer[( i * config.width + j ) * 3 + 1]	= static_cast<char>( color.y * 255.99f );
				color_buffer[( i * config.width + j ) * 3 + 2]	= static_cast<char>( color.z * 255.99f );

				if ( *terminate == true )
					return; 
			}
		}
	}

	/**
	* @brief compute the pixel color using adaptive antialiasing
	* @param scene
	* @param config
	* @param center				center of the color
	* @param sample_offset_x	offset in x for the subdivision
	* @param sample_offset_y	offset in y for the subdivision
	* @param depth
	*/
	vec3 adaptive_sampling( const Scene& scene, const Configuration& config, const vec3 center, const vec3 sample_offset_x, const vec3 sample_offset_y, const int depth )
	{
		const float tolerance = 0.05f;
		const Camera& camera = scene.camera();

		// get position of the samples
		const vec3 pos[4] = {
			center - sample_offset_x + sample_offset_y,
			center + sample_offset_x + sample_offset_y,
			center - sample_offset_x - sample_offset_y,
			center + sample_offset_x - sample_offset_y
		};

		Shapes::Ray rays[4];
		vec3 colors[4];
		vec3 final_color( 0.0f );

		// compute the rays and colors
		for ( int i = 0; i < 4; i++ )
		{
			rays[i]		= Shapes::Ray( camera.pos, normalize( pos[i] - camera.pos ) );
			colors[i]	= compute_pixel( scene, rays[i], config, scene.air().electric_permitivity, scene.air().magnetic_permeability );
			final_color += colors[i];
		}

		// get average of color
		final_color /= 4.0f;

		//return final_color;

		// if maximum depth was not reached
		if ( depth < config.antialiasing_samples )
		{
			// modify colors if they do not meet the tolerance
			for ( int i = 0; i < 4; i++ )
			{
				if ( std::abs( ( colors[i] - final_color ).length() ) > tolerance )
					colors[i] = adaptive_sampling( scene, config, pos[i], sample_offset_x / 2.0f, sample_offset_y / 2.0f, depth + 1 );
			}

			// recompute the final color
			final_color = vec3( 0.0f );
			for ( int i = 0; i < 4; i++ )
				final_color += colors[i];
			final_color /= 4.0f;
		}

		return final_color;
	}

	/**
	* @brief compute the color of a pixel in the image
	* @param scene	scene to trace
	* @param ray	ray from the camera equivalent for the current pixel
	* @param config	raytracer values
	* @param depth	current level of recursion
	*/
	vec3 compute_pixel( const Scene& scene, const Shapes::Ray& ray, const Configuration& config, const float e_permittivity, const float m_permeability, const int depth )
	{
		if ( config.depth <= depth )
			return vec3{ 0.0f, 0.0f, 0.0f };

		// get the contact of raycasting against the scene
		auto contact = raycast_scene( scene, ray );

		// if no contact return black
		if ( contact.time == -1.0f )
			return vec3( 0.0f, 0.0f, 0.0f );


		// electric permitivity and magnetic permeability
		float next_e_permittivity;
		float next_m_permeability;

		vec3 I = normalize( ray.dir );
		vec3 N = normalize( contact.normal );

		float cos_angle = -dot( I, N );

		// entering material
		if ( cos_angle > 0.0f )
		{
			next_e_permittivity = contact.material.electric_permittivity;
			next_m_permeability = contact.material.magnetic_permeability;
		}
		// exiting material
		else
		{
			next_e_permittivity = scene.air().electric_permitivity;
			next_m_permeability = scene.air().magnetic_permeability;
			N = -N;
			cos_angle = -cos_angle;
		}

		// small correction of contact position 
		vec3 contact_point_out = contact.point + config.epsilon * N;
		vec3 contact_point_in = contact.point - config.epsilon * N;



		// compute reflected, transmitted and absorved indices
		float reflection = compute_reflection_coeff( e_permittivity, m_permeability, next_e_permittivity, next_m_permeability, cos_angle );
		float transmission = 1.0f - reflection;
		float absortion = 1.0f - reflection * contact.material.specular_reflection - transmission * contact.material.specular_reflection;

		reflection		*= contact.material.specular_reflection;
		transmission	*= contact.material.specular_reflection;


		// lighting for absorbed light
		vec3 color = absortion * raycast_lights( scene, ray, config.shadow_samples, contact_point_out, contact.normal, contact.material );



		// refracted color
		if ( transmission > 0.0f )
		{
			// coefficients of refraction
			float n_i = sqrt( e_permittivity * m_permeability );
			float n_t = sqrt( next_e_permittivity * next_m_permeability );
			float ior = n_i / n_t;

			vec3 refr_dir = glm::refract( I, N, ior );

			Shapes::Ray refr_ray( contact_point_in, normalize( refr_dir ) );
			color += transmission * compute_pixel( scene, refr_ray, config, next_e_permittivity, next_m_permeability, depth + 1u );

		}



		// reflected color
		if ( reflection > 0.0f )
		{
			vec3 reflection_ray = glm::reflect( ray.dir, N );
			vec3 reflection_color(0.0f);

			// if roughness is zero all samples will go in the same direction
			int samples = contact.material.roughness == 0.0f ? 1 : config.reflection_samples;

			for ( int i = 0; i < samples; i++ )
			{
				// compute reflection direction
				vec3 reflection_dir;
				if ( i == 0 )
					reflection_dir = contact_point_out + reflection_ray;
				else
					reflection_dir = get_random_sample( contact_point_out + reflection_ray, contact.material.roughness );

				// compute reflection color
				Shapes::Ray new_ray( contact_point_out, normalize( reflection_dir - contact_point_out ) );
				reflection_color += reflection * compute_pixel( scene, new_ray, config, e_permittivity, m_permeability, depth + 1 );
			}

			// normalize reflection color and add it to the result
			reflection_color = reflection_color / static_cast<float>( samples );
			color += reflection_color;
		}



		// air attenuation
		auto& air = scene.air();
		float traversed_dist = length( contact.point - ray.pos );
		color *= glm::pow( air.attenuation, vec3( traversed_dist ) );

		return color;
	}

	/**
	* @brief raycast all the objects in the scene
	* @param scene	scene to raycast
	* @param ray
	* @return contact information of the raycast
	*/
	Intersection::Contact raycast_scene( const Scene& scene, const Shapes::Ray& ray )
	{
		Intersection::Contact result;
		result.time = -1.0f;

		// get the shapes of the scene
		const std::vector<Shapes::Shape*>& shapes = scene.shapes();

		// raycast shapes
		for ( unsigned i = 0; i < shapes.size(); i++ )
		{
			// compute the intersection time of the ray with the shape
			const Intersection::Contact contact = shapes[i]->intersect( ray );

			// if an intersection happens take the closest value
			if ( ( contact.time != -1.0f && contact.time < result.time ) || result.time == -1.0f )
				result = contact;
		}

		return result;
	}

	/**
	* @brief compute the color of the pixel based on the light
	* @param scene
	* @param ray
	* @param samples	shadow samples
	* @param contact	contact information
	*/
	vec3 raycast_lights( const Scene& scene, const Shapes::Ray& ray, const int samples, const vec3 contact_point, const vec3 contact_normal, const Material& material )
	{
		auto& lights = scene.lights();
		auto& ambient_light = scene.ambient();

		// compute lighting
		vec3 ambient{ 0.0f, 0.0f, 0.0f };
		vec3 diffuse{ 0.0f, 0.0f, 0.0f };
		vec3 specular{ 0.0f, 0.0f, 0.0f };

		ambient = ambient_light.color * material.diffuse_color;

		for ( auto& light : lights )
		{
			// check for shadows
			int oclusions = 0;

			// distance to the light
			float light_dist = dot( light.pos - contact_point, light.pos - contact_point );

			for ( int i = 0; i < samples; i++ )
			{
				// randomized point inside the light sphere
				vec3 pos;
				if ( i == 0 )
					pos = light.pos;
				else
					pos = get_random_sample( light.pos, light.radius );

				// create a ray towards the light
				Shapes::Ray light_ray( contact_point, normalize( pos - contact_point ) );

				// check for ocluder
				auto ocluder = raycast_scene( scene, light_ray );

				float ocluder_dist = dot( ocluder.point - contact_point, ocluder.point - contact_point );
				if ( ocluder.time != -1.0f && ocluder_dist < light_dist )
					oclusions++;
			}

			// shadow factor
			float shadow = 1.0f;
			if ( samples != 0.0f )
				shadow = 1.0f - ( oclusions ) / static_cast< float >( samples );
		


			// accumulate diffuse
			vec3 l = normalize( light.pos - contact_point );

			diffuse += light.color * material.diffuse_color
				* glm::max( dot( l, contact_normal ), 0.0f ) * shadow;

			// accumulate specular
			vec3 r = glm::reflect( ray.dir, contact_normal );

			specular += material.specular_reflection * glm::max( pow<float>( dot( r, l ), material.specular_exponent ), 0.0f ) * material.diffuse_color * shadow;
		}

		// compute final color and clamp
		vec3 color = ambient + diffuse + specular;

		return color;
	}

	/**
	* @brief compute a random point in an sphere
	* @param pos		position of the sphere
	* @param radius		radius of the sphere
	* @return point
	*/
	vec3 get_random_sample( const vec3& pos, const float radius )
	{
		vec3 point;

		// get random coordinates in unit box
		point.x = rand() / static_cast<float>( RAND_MAX ) - 0.5f;
		point.y = rand() / static_cast<float>( RAND_MAX ) - 0.5f;
		point.z = rand() / static_cast<float>( RAND_MAX ) - 0.5f;

		// normalize to get in unit sphere
		point = normalize( point );

		float u = rand() / static_cast<float>( RAND_MAX );
		float c = std::cbrt( u );

		point *= u;

		return pos + point * radius;
	}

	/**
	* @brief compute the reflection coefficient of the material
	* @param eps_i			incident electric permitivity
	* @param nu_i			incident magnetic permeability
	* @param eps_t			transmitted electric permittivity
	* @param nu_t			transmitted magnetic permeability
	* @param cos_angle		cosine of the angle of incidence between the light ray and the surface normal of the transmitted material
	* @return transmitted angle
	* @return refraction coefficient
	*/
	float compute_reflection_coeff( const float eps_i, const float nu_i, const float eps_t, const float nu_t, const float incident_angle )
	{
		// coefficients of refraction
		const float n_i = sqrt( eps_i * nu_i );
		const float n_t = sqrt( eps_t * nu_t );

		// ratio of idices of refraction
		const float ior = n_i / n_t;

		// square root radicant
		float radicant = 1.0f - ior * ior * ( 1.0f - ( incident_angle * incident_angle ) );

		// negative radicant -> no refraction
		if ( radicant < 0.0f )
			return 1.0f;

		const float transmitted_angle = sqrt( radicant );

		// compute index of reflection for light polarized in a direction perpendicular / parallel to the plane of incidence
		const float perpendicular	= ( ior * incident_angle - ( nu_i / nu_t ) * transmitted_angle) /
									  ( ior * incident_angle + ( nu_i / nu_t ) * transmitted_angle );
		const float parallel		= ( ( nu_i / nu_t ) * incident_angle - ior * transmitted_angle) / 
									  ( ( nu_i / nu_t ) * incident_angle + ior * transmitted_angle );

		// average of the index of reflection
		return 0.5f * ( perpendicular * perpendicular + parallel * parallel );
	}

	/**
	* @brief compute random direction vector from camera to pixle position
	* @param camera
	* @param pixel_pos		position of the current pixel in world coordinates
	* @param focal_point	distance from the lens to the focal plane
	* @return random ray from the camera lens through the focus point
	*/
	Shapes::Ray compute_ray_dir_dof( const Camera& camera, const vec3& pixel_pos, const float focal_point )
	{
		// compute ray from lense center
		vec3 center_dir = normalize( pixel_pos - camera.pos );

		// random offset in the lense
		vec2 r_offset = camera.get_rand_lense_point();

		// point in focus
		vec3 focus_plane_pos = camera.pos + focal_point * center_dir;


		// project the lens_point in the lense of the camera
		vec3 lens_point = camera.pos + r_offset.x * camera.u + r_offset.y * camera.v;
		
		float normal = dot( lens_point - camera.pos, camera.w );
		lens_point -= camera.w * normal; 
		
		return Shapes::Ray( lens_point, focus_plane_pos - lens_point );
	}

	/**
	* @brief compute the focal length for the current pixel
	* @param camera
	* @param axis_offset	distance of current point from the optical axis
	*/
	float compute_focal_point( const Camera& camera, const float axis_offset )
	{
		// lens data
		const float n = camera.refraction_index;	// refraction index
		const float r1 = camera.r1;					// radius of thin lens
		const float r2 = camera.r2;

		// no refraction
		if ( n == 0.0f )
			return camera.focal_point;

		// focal length (image plane to focus plane)
		const float focal_length = 1.0f / ( ( n - 1.0f ) * ( 1.0f / r1 - 1.0f / r2 ) );
		// image plane to lens
		const float image_distance = focal_length - camera.focal_point;

		// compute lens aberration
		const float q = ( r2 + r1 ) / ( r2 - r1 );
		const float p = ( image_distance - camera.focal_point ) / focal_length;

		const float K = 1.0f / ( 4.0f * focal_length * n * ( n - 1.0f ) ) *
			( ( n + 2.0f ) / ( n - 1.0f ) * q * q +
				4.0f * ( n + 1.0f ) * q * p +
				( 3.0f * n + 2.0f ) * ( n - 1.0f ) * p * p +
				n * n * n / ( n - 1.0f ) );

		// focal length difference
		const float delta_focal = 0.5f * K * axis_offset * axis_offset;

		// return new distance from lens to focus plane
		return focal_length - delta_focal - image_distance;
	}
}
