/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: scene.h.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/08/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "shapes.h"
#include "light.h"
#include "camera.h"
#include <vector>
#include <string>

class Scene
{
public:

	Scene() = default;
	Scene( const char* filename );
	void load_scene( const char* filename );

	~Scene();
	void clear();

private:

	void read_line( std::string& line );

	Shapes::Sphere*		read_sphere			( std::string& data );
	Shapes::Box*		read_box			( std::string& data );
	Shapes::Polygon*	read_polygon		( std::string& data );
	Shapes::Ellipsoid*	read_ellipsoid		( std::string& data );
	Shapes::Mesh*		read_mesh			( std::string& data );
	Lights::Point		read_point_light	( std::string& data );
	Lights::Ambient		read_ambient		( std::string& data );
	Lights::Air			read_air			( std::string& data );
	Camera				read_camera			( std::string& data );

	Material			read_material		( std::string& data );

	vec3				read_vector			( std::string& data );
	vec2				read_vector2		( std::string& data );
	float				read_float			( std::string& data, const char prev_char, const char post_char );
	float				read_float			( std::string& data );
	int					read_int			( std::string& data );

	Shapes::Mesh* load_obj( const char* file_path );

public:
	const std::vector<Shapes::Shape*>&	shapes	() const;
	const Camera&						camera	() const;
	const std::vector<Lights::Point>&	lights	() const;
	const Lights::Ambient&				ambient	() const;
	const Lights::Air&					air		() const;

private:
	std::vector<Shapes::Shape*>		m_shapes;

	std::vector<Lights::Point>		m_lights;
	Lights::Ambient					m_ambient;
	Lights::Air						m_air;
	Camera							m_camera;
};