#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <Eigen/Eigen>
#include <opencv2/core.hpp>
#include <glad/glad.h>
#include "camera.h"

class DepthVisualizer
{
public:
	DepthVisualizer(const Camera &_cam,
					const std::string &_vert_shader_path,
					const std::string &_frag_shader_path,
                    float _near_clip,
                    float _far_clip,
                    float _max_dist);
    std::pair<cv::Mat, cv::Mat> visualize(const cv::Mat &_dmap_host);

private:
    Eigen::Matrix4f gl_proj(float _fx, float _fy, float _cx, float _cy, int _width, int _height);

    Camera cam_;

	// OpenGL objects
    GLuint default_vao_;
    GLuint dmap_tex_;
    GLuint fbo_;
    GLuint phong_rbo_;
    GLuint nmap_rbo_;
    GLuint zbuffer_rbo_;
    GLuint cam_ubo_;
    GLuint light_ubo_;
    GLuint material_ubo_;
    GLuint pipeline_;

    // rasterization params
    float max_dist = 0.1f;
    
	// clipping plane
	float near_clip_ = 0.1f;
	float far_clip_ = 10.0f;
};

#endif