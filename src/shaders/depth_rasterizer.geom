#version 460 core

layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

layout (binding = 0, std140) uniform CAM_UBO
{
	float fx, fy, cx, cy;
	float max_dist;
	mat4 proj;
};

out GS_FS_INTERFACE
{
    float depth;
} gs_out;

layout (binding = 0) uniform usampler2D dmap_tex;

void main()
{
	ivec2 dmap_size = textureSize(dmap_tex, 0);
	int width = dmap_size.x;
	int xid = gl_PrimitiveIDIn%width;
	int yid = gl_PrimitiveIDIn/width;
	
	float z0 = float(texelFetch(dmap_tex, ivec2(xid  , yid  ), 0).x)/1000.0;
	float z1 = float(texelFetch(dmap_tex, ivec2(xid  , yid+1), 0).x)/1000.0;
	float z2 = float(texelFetch(dmap_tex, ivec2(xid+1, yid  ), 0).x)/1000.0;
	float z3 = float(texelFetch(dmap_tex, ivec2(xid+1, yid+1), 0).x)/1000.0;
	
	vec3 v1 = vec3(z1*(xid  -cx)/fx, z1*(yid+1-cy)/fy, z1);
	vec3 v2 = vec3(z2*(xid+1-cx)/fx, z2*(yid  -cy)/fy, z2);
	if (length(v1-v2) < max_dist) {
		vec3 v0 = vec3(z0*(xid-cx)/fx, z0*(yid-cy)/fy, z0);
		if (length(v1-v0) < max_dist && length(v2-v0) < max_dist) {
			gl_Position = proj * vec4(v0.x, v0.y, v0.z, 1.0);
			gs_out.depth = z0;
			EmitVertex();
			gl_Position = proj * vec4(v1.x, v1.y, v1.z, 1.0);
			gs_out.depth = z1;
			EmitVertex();
			gl_Position = proj * vec4(v2.x, v2.y, v2.z, 1.0);
			gs_out.depth = z2;
			EmitVertex();
		}
		EndPrimitive();
		
		vec3 v3 = vec3(z3*(xid+1-cx)/fx, z3*(yid+1-cy)/fy, z3);
		if (length(v3-v1) < max_dist && length(v3-v2) < max_dist) {
			gl_Position = proj * vec4(v1.x, v1.y, v1.z, 1.0);
			gs_out.depth = z1;
			EmitVertex();
			gl_Position = proj * vec4(v3.x, v3.y, v3.z, 1.0);
			gs_out.depth = z3;
			EmitVertex();
			gl_Position = proj * vec4(v2.x, v2.y, v2.z, 1.0);
			gs_out.depth = z2;
			EmitVertex();
		}
		EndPrimitive();
	}
}