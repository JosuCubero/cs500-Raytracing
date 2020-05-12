/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: main.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 01/07/2020
----------------------------------------------------------------------------------------------------------*/

#include "scene.h"
#include "raytracer.h"
#include "image.h"

#include <string>
#include <fstream>
#include <iostream>

Configuration read_config( std::string& in_scene, std::string& out_scene );
float read_val( std::string& data );

/**
* @brief read config file
* @return in_scene		file path of input scene
* @return out_scene		file path of output scene
* @return configuration	properties
*/
Configuration read_config( std::string& in_scene, std::string& out_scene )
{
	Configuration configuration;


	// read config
	std::ifstream file;
	file.open( ".config" );

	// no config file -> set default values
	if ( !file )
	{
		in_scene = "scene/RefractScene.txt";
		out_scene = "output/zout.png";
		configuration.depth					= 10;
		configuration.height				= 500;
		configuration.width					= 500;
		configuration.antialiasing_samples	= 10;
		configuration.adaptive_antialiasing	= false;
		configuration.shadow_samples		= 1;
		configuration.dof_samples			= 1;
		configuration.reflection_samples	= 1;
		configuration.window				= true;

		configuration.epsilon				= 0.01f;
	}
	else
	{
		// copy the file stream to the stream buffer
		std::string file_data;
		file.seekg( 0, std::ios::end );
		file_data.reserve( file.tellg() );
		file.seekg( 0, std::ios::beg );
		file_data.assign( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );

		// close the file
		file.close();
		
		// read input file
		size_t new_line;
		new_line = file_data.find( '\n' );
		in_scene = file_data.substr( 0u, new_line );
		file_data = file_data.substr( new_line + 1u );

		// read output file
		new_line = file_data.find( '\n' );
		out_scene = file_data.substr( 0u, new_line );
		file_data = file_data.substr( new_line + 1u );

		// read depth
		configuration.depth	= static_cast<int>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read resolution
		configuration.width	= static_cast<int>( read_val( file_data ) );
		configuration.height = static_cast<int>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read antialiasing samples
		configuration.antialiasing_samples = static_cast<int>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read adaptive samples flag
		configuration.adaptive_antialiasing = static_cast<bool>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read shadow samples
		configuration.shadow_samples = static_cast<int>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read dof flag
		bool dof = static_cast<bool>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read dof samples
		configuration.dof_samples = static_cast<int>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		if ( dof == false )
			configuration.dof_samples = 1;

		// read reflection samples
		configuration.reflection_samples = static_cast<int>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read window flag
		configuration.window = static_cast<bool>( read_val( file_data ) );
		file_data = file_data.substr( 1u );

		// read epsilon
		configuration.epsilon = read_val( file_data );

	}
	return configuration;
}


/**
* @brief read an int value from a string
* @param data		string to read from
* @return int
*/
float read_val( std::string& data )
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
* @brief main function
* @param argc
* @param argv
*/
int main( int argc, char** argv )
{
	// read config
	Configuration config;
	std::string input_file;
	std::string output_file;
	config = read_config( input_file, output_file );



	// command window prompt
	std::cout << "Generating image for scene: " << input_file << " with size " << config.width * config.height << std::endl;

	// load the scene
	Scene scene( input_file.c_str() );


	// compute image
	std::vector<unsigned char> color_buffer;
	if ( Raytracer::trace_scene( color_buffer, scene, config ) )
		// save image
		Image::save_image( output_file.c_str(), config.width, config.height, color_buffer );

	return 0;
}