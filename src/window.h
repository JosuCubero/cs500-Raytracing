/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.

File Name: window.h
Author: Josu Cubero Ruiz de Gopegui, josu.cubero, 540001316
Creation date: 02/17/2020
----------------------------------------------------------------------------------------------------------*/

#pragma once

#include "math_utils.h"

#include "opengl.h"
#include <vector>

class Window
{
public:

	void initialize( const int width, const int height, const std::vector<unsigned char>& color_buffer );
	void render( const std::vector<unsigned char>& color_buffer );
	void exit();

	bool should_close();

private:
	unsigned compile_shader( const char* file, GLenum shader_type );
	void generate_plane();

private:
	GLFWwindow* m_window;

	int m_width;
	int m_height;

	unsigned m_vao;
	unsigned m_vbo[3];

	unsigned m_program;
	unsigned m_vertex_shader;
	unsigned m_fragment_shader;

	unsigned m_texture;
};