#version 450 core

layout (std140, binding = 0) uniform CAM_PAPAMETERS
{
	float fx, fy, cx, cy, max_dist;
};

layout (std140, binding = 1) uniform LIGHT_COEFFS
{
	vec3 la;
	vec3 ld;
	vec3 ls;
	vec3 ldir;
};

layout (std140, binding = 2) uniform MATERIAL_COEFFS
{
	vec3 ma;
	vec3 md;
	vec3 ms;
	float ss;
};

layout (binding = 0) uniform usampler2D depth_map;
layout (origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;
layout (location = 0) out vec4 phong_shading;
layout (location = 1) out vec4 normal_shading;

void main()
{
	int xid = int(gl_FragCoord.x);
	int yid = int(gl_FragCoord.y);
	float d  = float(texelFetch(depth_map, ivec2(xid  , yid  ), 0))/1000.0;
	float dl = float(texelFetch(depth_map, ivec2(xid-1, yid  ), 0))/1000.0;
	float dr = float(texelFetch(depth_map, ivec2(xid+1, yid  ), 0))/1000.0;
	float dt = float(texelFetch(depth_map, ivec2(xid  , yid-1), 0))/1000.0;
	float db = float(texelFetch(depth_map, ivec2(xid  , yid+1), 0))/1000.0;
	if (d > 0 && dl > 0 && dr > 0 && dt > 0 && db > 0) {
		vec3 p  = vec3(d *(xid  -cx)/fx, d *(yid  -cy)/fy, d );
		vec3 pl = vec3(dl*(xid-1-cx)/fx, dl*(yid  -cy)/fy, dl);
		vec3 pr = vec3(dr*(xid+1-cx)/fx, dr*(yid  -cy)/fy, dr);
		vec3 pt = vec3(dt*(xid  -cx)/fx, dt*(yid-1-cy)/fy, dt);
		vec3 pb = vec3(db*(xid  -cx)/fx, db*(yid+1-cy)/fy, db);
		vec3 e0 = pb - pt;
		vec3 e1 = pr - pl;
		vec3 n = normalize(cross(e0, e1));
		
		// normal shading
		normal_shading = vec4(0.5*vec3(n.x, -n.y, -n.z)+0.5, 1.0);
		//normal_shading = vec4(1, 0, 0, 1);
		
		// phong shading
		vec3 ka = la * ma;
		vec3 kd = ld * md;
		vec3 ks = ls * ms;
		vec3 vdir = normalize(-p);
		vec3 rdir = reflect(ldir, n);
		vec3 ca = ka;
		vec3 cd = kd * max(dot(n, -ldir), 0.0);
		vec3 cs = ks * pow(max(dot(vdir, rdir), 0.0), ss);
		vec3 c = clamp(ca + cd + cs, 0.0, 1.0);
		phong_shading = vec4(c, 1.0);
		//phong_shading = vec4(0, 1, 0, 1);
	}
	else {
		discard;
	}
}