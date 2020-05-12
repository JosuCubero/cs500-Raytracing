#version 330

layout( location = 0 ) in vec3  v_pos;
layout( location = 1 ) in vec2  v_text_coord;

out vec2 text_coord;

void main()
{
	gl_Position = vec4( v_pos, 1.0 );
	text_coord = v_text_coord;
}
