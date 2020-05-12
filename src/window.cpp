/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: window.cpp
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 02/17/2020
----------------------------------------------------------------------------------------------------------*/

#include "window.h"

#include <iostream>
#include <fstream>
#include <string>

/**
* @brief initialize preview window
* @param width			screen width
* @param height			screen height
* @param color_buffer	reference to the color buffer to render
*/
void Window::initialize( const int width, const int height, const std::vector<unsigned char>& color_buffer )
{
	m_width = width;
	m_height = height;

	// initialize glfw and glad
	if ( !glfwInit() )
	{
		std::cout << "Error initializeing glfw" << std::endl;
		std::abort();
	}
	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 3 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	// create window
	m_window = nullptr;
	m_window = glfwCreateWindow( m_width, m_height, "cs500 Preview", NULL, NULL );

	if ( !m_window )
	{
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		std::abort();
	}
	glfwMakeContextCurrent( m_window );
	if ( !gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ) )
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		std::abort();
	}
	GL_CALL( glViewport( 0, 0, m_width, m_height ) );

	
	// texture
	GL_CALL( glGenTextures( 1, &m_texture ) );
	GL_CALL( glBindTexture( GL_TEXTURE_2D, m_texture ) );
	GL_CALL( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, color_buffer.data() ) );

	// filtering
	GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) );
	GL_CALL( glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) );
	
	// shaders and program
	m_vertex_shader = compile_shader( "shaders/vertex_shader.vs", GL_VERTEX_SHADER );
	m_fragment_shader = compile_shader( "shaders/fragment_shader.fs", GL_FRAGMENT_SHADER );

	m_program = glCreateProgram();
	GL_CALL( glAttachShader( m_program, m_vertex_shader ) );
	GL_CALL( glAttachShader( m_program, m_fragment_shader ) );
	GL_CALL( glLinkProgram ( m_program ) );

	// generate plane vao
	generate_plane();
}

/**
* @brief render current buffer
* @param color_buffer	color buffer to render
*/
void Window::render( const std::vector<unsigned char>& color_buffer )
{
	GL_CALL( glClearColor( 0.0f, 0.0f, 0.0f, 1.0f ) );
	GL_CALL( glClear( GL_COLOR_BUFFER_BIT ) );
	
	GL_CALL( glBindTexture( GL_TEXTURE_2D, m_texture ) );
	GL_CALL( glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, color_buffer.data() ) );
	
	GL_CALL( glUseProgram( m_program ) );
	GL_CALL( glBindVertexArray( m_vao ) );
	GL_CALL( glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0 ) );
	
	GL_CALL( glBindVertexArray( 0 ) );
	
	glfwPollEvents();
	glfwSwapBuffers( m_window );
}

/**
* @brief release memory of the preview window
*/
void Window::exit()
{
	GL_CALL( glDeleteTextures( 1, &m_texture ) );

	GL_CALL( glDeleteBuffers( 3, m_vbo ) );
	GL_CALL( glDeleteVertexArrays( 1, &m_vao ) );

	glfwDestroyWindow( m_window );
	glfwTerminate();
}

/**
* @brief should window close
* @return bool
*/
bool Window::should_close()
{
	glfwPollEvents();
	return glfwWindowShouldClose( m_window );
}



/**
* @brief read a shader form a file and compile it
* @param file_name		file containing the shader code
* @param shader_type	type of the shader
*/
unsigned Window::compile_shader( const char* file_path, GLenum shader_type )
{
	// open the file
	std::ifstream file( file_path, std::ios::in );

	// error check
	if ( !file )
	{
		std::cout << "Unable to open file " << file_path << std::endl;
		std::abort();
	}

	// copy the file content into a string
	std::string source( ( std::istreambuf_iterator<char>( file ) ),
		( std::istreambuf_iterator<char>() ) );

	file.close();

	// create and compile the shader
	GLuint shader = glCreateShader( shader_type );

	const char* c_source = source.c_str();
	GL_CALL( glShaderSource( shader, 1, &c_source, NULL ) );
	GL_CALL( glCompileShader( shader ) );

	return shader;
}

/**
*  @brief generate plane vao in NDC coordinates to print preview of the raytracer
*/
void Window::generate_plane()
{
	// plane vertices
	const float vertices[] = {
		-1.0f,  1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f
	};

	const float texture_coords[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	const unsigned indices[] = {
		0u, 1u, 2u,
		1u, 3u, 2u
	};


	// vao / vbos
	GL_CALL( glGenVertexArrays( 1, &m_vao ) );
	GL_CALL( glBindVertexArray( m_vao ) );

	// vertex position
	GL_CALL( glGenBuffers( 3, m_vbo ) );
	GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, m_vbo[0] ) );
	GL_CALL( glBufferData( GL_ARRAY_BUFFER, sizeof( vertices ), vertices, GL_STATIC_DRAW ) );
	GL_CALL( glEnableVertexAttribArray( 0 ) );
	GL_CALL( glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( float ) * 3, ( void* )0 ) );
	
	// texture coordinates
	GL_CALL( glBindBuffer( GL_ARRAY_BUFFER, m_vbo[1] ) );
	GL_CALL( glBufferData( GL_ARRAY_BUFFER, sizeof( texture_coords ), texture_coords, GL_STATIC_DRAW ) );
	GL_CALL( glEnableVertexAttribArray( 1 ) );
	GL_CALL( glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( float ) * 2, ( void* )0 ) );
	
	// indices
	GL_CALL( glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vbo[2] ) );
	GL_CALL( glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( indices ), indices, GL_STATIC_DRAW ) );
	
	GL_CALL( glBindVertexArray( 0 ) );
}
