#version 460 core

layout (location = 0) out unsigned short z_value;

layout (binding = 0) uniform usampler2D dmap_tex;

layout (origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;

void main()
{
    //z_value = unsigned short(1000.0*fs_in.depth);
	
	unsigned short z = unsigned short(10*texelFetch(dmap_tex, ivec2(gl_FragCoord.x, gl_FragCoord.y), 0).x);

	z_value = z;
}