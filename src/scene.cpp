/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: scene.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/08/2020
----------------------------------------------------------------------------------------------------------*/

#include "scene.h"

#include <fstream>
#include <iostream>


/**
* @brief scene constructor
* @param filename	file to load scene from
*/
Scene::Scene( const char* filename )
{
	load_scene( filename );
}

/**
* @load the scene from a file
* @param filename	path of the file
*/
void Scene::load_scene( const char * filename )
{
	// clear data if any
	clear();

	std::ifstream file;
	file.open( filename );

	// sanity check
	try
	{
		if ( !file )
			throw "invalid file path";
	}
	catch ( const char* error )
	{
		std::cout << error << std::endl;
		std::abort();
	}

	// copy the file stream to the stream buffer
	std::string file_data;
	file.seekg( 0, std::ios::end );
	file_data.reserve( file.tellg() );
	file.seekg( 0, std::ios::beg );
	file_data.assign( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );

	// close the file
	file.close();

	// proccess data
	while ( file_data.empty() == false )
	{
		// proccess the line
		read_line( file_data );
	}
}

/**
* @brief scene destructor
*/
Scene::~Scene()
{
	clear();
}

/**
* @brief clear the scene data
*/
void Scene::clear()
{
	for ( auto shape : m_shapes )
		delete shape;
	m_shapes.clear();
}


/**
* @brief proccess a line of the scene to load
* @param line	line to process
*/
void Scene::read_line( std::string& line )
{
	// skip comments
	if ( line.rfind( '#', 0u ) == 0u )
	{
		line = line.substr( line.find( '\n' ) + 1u );
		return;
	}
		


	// read sphere
	if ( line.rfind( "SPHERE", 0u ) == 0u )
	{
		Shapes::Sphere* sphere = read_sphere( line );
		m_shapes.push_back( sphere );
		return;
	}

	// read box
	if ( line.rfind( "BOX", 0u ) == 0u )
	{
		Shapes::Box* box = read_box( line );
		m_shapes.push_back( box );
		return;
	}

	// read polygon
	if ( line.rfind( "POLYGON", 0u ) == 0u )
	{
		Shapes::Polygon* polygon = read_polygon( line );
		m_shapes.push_back( polygon );
		return;
	}

	// read ellipsoid
	if ( line.rfind( "ELLIPSOID", 0u ) == 0u )
	{
		Shapes::Ellipsoid* ellipsoid = read_ellipsoid( line );
		m_shapes.push_back( ellipsoid );
		return;
	}

	// read mesh
	if ( line.rfind( "MESH", 0u ) == 0u )
	{
		Shapes::Mesh* mesh = read_mesh( line );
		m_shapes.push_back( mesh );
		return;
	}

	// read light
	if ( line.rfind( "LIGHT", 0u ) == 0u )
	{
		Lights::Point light = read_point_light( line );
		m_lights.push_back( light );
		return;
	}

	// read ambient
	if ( line.rfind( "AMBIENT", 0u ) == 0u )
	{
		m_ambient = read_ambient( line );
		return;
	}

	// read air
	if ( line.rfind( "AIR", 0u ) == 0u )
	{
		m_air = read_air( line );
		return;
	}

	// read camera
	if ( line.rfind( "CAMERA", 0u ) == 0u )
	{
		m_camera = read_camera( line );
		return;
	}

	// empty lines
	else
	{
		line = line.substr( 1u );
		return;
	}
}

/**
* @brief read sphere data
* @param data
* @return Sphere
*/
Shapes::Sphere* Scene::read_sphere( std::string& data )
{
	Shapes::Sphere* sphere = new Shapes::Sphere;

	sphere->pos			= read_vector( data );		// position values
	sphere->radius		= read_float( data );	// read radius

	sphere->material		= read_material( data );	// read the material properties

	return sphere;
}

/**
* @brief read box data
* @param data
* @return box
*/
Shapes::Box* Scene::read_box( std::string& data )
{
	Shapes::Box* box = new Shapes::Box;

	box->pos			= read_vector( data );		// corner
	box->length			= read_vector( data );		// length
	box->width			= read_vector( data );		// width
	box->height			= read_vector( data );		// height

	box->generate_planes();

	box->material	= read_material( data );	// material

	return box;
}

/**
* @brief read polygon data
* @param data
* @return polygon
*/
Shapes::Polygon* Scene::read_polygon( std::string& data )
{
	Shapes::Polygon* polygon = new Shapes::Polygon;

	int i = read_int( data );

	for ( ; i > 0; i-- )
		polygon->vertices.push_back( read_vector( data ) );

	auto& vertices = polygon->vertices;
	polygon->normal = normalize( cross( vertices[1u] - vertices[0u], vertices[2u] - vertices[0u] ) );

	polygon->material = read_material( data );

	return polygon;
}

/**
* @brief read ellipsoid data
* @param data
* @return ellipsoid
*/
Shapes::Ellipsoid* Scene::read_ellipsoid( std::string& data )
{
	Shapes::Ellipsoid* ellipsoid = new Shapes::Ellipsoid;

	ellipsoid->pos = read_vector( data );
	ellipsoid->u = read_vector( data );
	ellipsoid->v = read_vector( data );
	ellipsoid->w = read_vector( data );

	ellipsoid->inv_model = inverse( mat3( ellipsoid->u, ellipsoid->v, ellipsoid->w ) );

	ellipsoid->material = read_material( data );

	return ellipsoid;
}

/**
* @brief read mesh data
* @param data
* @return mesh
*/
Shapes::Mesh * Scene::read_mesh( std::string& data )
{
	// read file_path
	size_t new_line = data.find( '\n' );
	size_t space = data.find( ' ' );

	size_t start = new_line < space ? new_line : space;
	data = data.substr( start + 1u );

	new_line = data.find( '\n' );
	space = data.find( ' ' );

	size_t end = new_line < space ? new_line : space;

	std::string path = data.substr( 0u, end );
	data = data.substr( end );

	// read vertices
	Shapes::Mesh* mesh = load_obj( path.c_str() );

	// get position, rotation and scale
	vec3 pos = read_vector( data );
	vec3 rot = read_vector( data );
	float scl = read_float( data );

	// construct model to world
	mat4 translate = glm::translate( pos );
	rot = glm::radians( rot );
	mat4 rot_x = glm::rotate( translate, rot.x, vec3( 1.0f, 0.0f, 0.0f ) );
	mat4 rot_y = glm::rotate( rot_x, rot.y, vec3( 0.0f, 1.0f, 0.0f ) );
	mat4 rot_z = glm::rotate( rot_y, rot.z, vec3( 0.0f, 0.0f, 1.0f ) );
	mat4 model = glm::scale( rot_z, vec3( scl ) );

	// transform vertices
	for ( auto& vertex : mesh->vertices )
		vertex = vec3( model * vec4( vertex, 1.0f ) );

	// generate bounding volume
	mesh->compute_bv();

	// read material
	mesh->material = read_material( data );

	return mesh;
}

/**
* @breif read the point light data
* @param data
* @return point light
*/
Lights::Point Scene::read_point_light( std::string& data )
{
	Lights::Point light;

	light.pos		= read_vector( data );
	light.color		= read_vector( data );
	light.radius	= read_float ( data );

	return light;
}

/**
* @brief read the ambient light data
* @param data
* @return ambient light
*/
Lights::Ambient Scene::read_ambient( std::string& data )
{
	Lights::Ambient ambient;

	ambient.color = read_vector( data );

	return ambient;
}

/**
* @brief read the air
* @param data
* @return air
*/
Lights::Air Scene::read_air( std::string& data )
{
	Lights::Air air;

	air.electric_permitivity	= read_float ( data );
	air.magnetic_permeability	= read_float ( data );
	air.attenuation				= read_vector( data );

	return air;
}

/**
* @brief read camera data
* @param data
* @return sphere
*/
Camera Scene::read_camera( std::string& data )
{
	Camera camera;
	
	camera.center		= read_vector( data );			// center of viewport
	camera.u			= read_vector( data );			// projection u values
	camera.v			= read_vector( data );			// projection v values
    camera.w = normalize( cross( camera.u, camera.v ) );	// forward vector

	camera.r			= read_float( data );			// r value

	camera.aperture		= read_float( data );			// aperture
	camera.focal_point	= read_float( data );			// focal length
	// thin lens data
	camera.refraction_index = read_float( data );
	camera.r1 = read_float( data );
	camera.r2 = read_float( data );

	data = data.substr( 1u );
	if ( data.rfind( "LENSE", 0u ) == 0u )
	{
		float total_area = 0.0f;

		int i = read_int( data );
		for ( ; i > 0; i-- )
		{
			Shapes::LenseTriangle t;

			t.a = read_vector2( data ) * camera.aperture;
			t.b = read_vector2( data ) * camera.aperture;
			t.c = read_vector2( data ) * camera.aperture;

			vec2 va = t.b - t.a;
			vec2 vb = t.c - t.a;

			float h = std::abs( dot( vb, normalize( vec2{ -va.y, va.x } ) ) );

			t.area_euristic = length( t.b - t.a ) * h / 2.0f;
			total_area += t.area_euristic;

			camera.lense_triangles.push_back( t );
		}

		// set the euristic value
		for ( auto& triangle : camera.lense_triangles )
			triangle.area_euristic /= total_area;
	}


	// compute camera position
	camera.pos = camera.center + normalize( cross( camera.u, camera.v ) ) * camera.r;

	return camera;
}

/**
* @brief read material data of an object
* @param data	line to parse
* @return material
*/
Material Scene::read_material( std::string& data )
{
	Material material;

	material.diffuse_color			= read_vector( data );
	material.specular_reflection	= read_float ( data );
	material.specular_exponent		= read_float ( data );
	material.attenuation			= read_vector( data );
	material.electric_permittivity	= read_float ( data );
	material.magnetic_permeability	= read_float ( data );
	material.roughness				= read_float ( data );

	return material;
}

/**
* @brief read a vector from a string
* @param data	string to read from
* @return vector
*/
vec3 Scene::read_vector( std::string & data )
{
	vec3 vec;

	vec.x = read_float( data, '(', ',' );	// x value
	vec.y = read_float( data, ',', ',' );	// y value
	vec.z = read_float( data, ',', ')' );	// z value

	return vec;
}

/**
* @brief read a vector from a string
* @param data	string to read from
* @return vector
*/
vec2 Scene::read_vector2( std::string & data )
{
	vec2 vec;

	vec.x = read_float( data, '(', ',' );	// x value
	vec.y = read_float( data, ',', ')' );	// y value

	return vec;
}

/**
* @brief read a float value from a string
* @param data		string to read from
* @param prev_char	character previous to the value
* @param post_char	character right after the value
* @return float
*/
float Scene::read_float( std::string& data, const char prev_char, const char post_char )
{
	float result;

	size_t start	= data.find( prev_char );
	data = data.substr( start + 1u );
	size_t end		= data.find( post_char );

	result = static_cast<float>( std::atof( data.substr( 0u, end ).c_str() ) );
	data = data.substr( end );

	return result;
}

/**
* @brief read a float value from a string
* @param data		string to read from
* @return float
*/
float Scene::read_float( std::string& data )
{
	float result;

	size_t new_line = data.find( '\n' );
	size_t space = data.find( ' ' );

	size_t start = new_line < space ? new_line : space;
	data = data.substr( start + 1u );

	new_line = data.find( '\n' );
	space = data.find( ' ' );

	size_t end = new_line < space ? new_line : space;

	result = static_cast< float >( std::atof( data.substr( 0u, end ).c_str() ) );
	data = data.substr( end );

	return result;
}

/**
* @brief read an int value from a string
* @param data		string to read from
* @return int
*/
int Scene::read_int( std::string & data )
{
	int result;

	size_t new_line = data.find( '\n' );
	size_t space = data.find( ' ' );

	size_t start = new_line < space ? new_line : space;
	data = data.substr( start + 1u );

	new_line = data.find( '\n' );
	space = data.find( ' ' );

	size_t end = new_line < space ? new_line : space;

	result = static_cast<int>( std::atof( data.substr( 0u, end ).c_str() ) );
	data = data.substr( end );

	return result;
}

/**
* @brief read a mesh from an obj file
* @param file_path
*/
Shapes::Mesh* Scene::load_obj( const char* file_path )
{
	Shapes::Mesh* mesh = new Shapes::Mesh;

	std::ifstream file( file_path );

	if ( !file.is_open() )
	{
		std::cout << "Couldn't open the file " << file_path << std::endl;
		std::abort();
	}

	while ( !file.eof() )
	{
		std::string line;
		file >> line;

		// vertex
		if ( line == "v" )
		{
			vec3 vertex;

			std::string val;
			file >> val;
			vertex.x = static_cast< float >( std::atof( val.c_str() ) );
			file >> val;
			vertex.y = static_cast< float >( std::atof( val.c_str() ) );
			file >> val;
			vertex.z = static_cast< float >( std::atof( val.c_str() ) );

			mesh->vertices.push_back( vertex );
		}
		// face
		else if ( line == "f" )
		{
			ivec3 index;

			std::string val;
			file >> val;
			index.x = static_cast<int>( std::atof( val.c_str() ) );
			file >> val;
			index.y = static_cast<int>( std::atof( val.c_str() ) );
			file >> val;
			index.z = static_cast<int>( std::atof( val.c_str() ) );

			mesh->indices.push_back( index - glm::one<ivec3>() );
		}
		// not relevant
		else
		{
			std::string useless;
			std::getline( file, useless );
		}
	}

	file.close();

	return mesh;
}

/**
* @brief get the shapes of the scene
* @return shapes
*/
const std::vector<Shapes::Shape*>& Scene::shapes() const
{
	return m_shapes;
}

/**
* @brief get the camera of the scene
* @return camera
*/
const Camera& Scene::camera() const
{
	return m_camera;
}

/**
* @brief get the lights of the scene
* @return lights
*/
const std::vector<Lights::Point>& Scene::lights() const
{
	return m_lights;
}

/**
* @brief get the ambient light of the scene
* @return ambient light
*/
const Lights::Ambient & Scene::ambient() const
{
	return m_ambient;
}

/**
* @brief get the medium of the scene
* @return air
*/
const Lights::Air & Scene::air() const
{
	return m_air;
}
