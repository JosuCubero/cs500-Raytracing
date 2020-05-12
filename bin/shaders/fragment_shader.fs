#version 330

uniform sampler2D image;

in 	vec2 text_coord;
out vec4 output_color;

void main()
{
	output_color = vec4( texture( image, text_coord ).rgb, 1.0 );
}
