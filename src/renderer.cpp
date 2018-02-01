#include "renderer.h"
#include <fstream>
#include <string>
#include "pipeline.h"
#include <iostream>
#include <opencv2/opencv.hpp>

struct CamUboHost
{
    float fx, fy, cx, cy;
    float max_dist;
} cam_ubo_host;

struct LightUboHost
{
    alignas(16) Eigen::Vector3f la;
    alignas(16) Eigen::Vector3f ld;
    alignas(16) Eigen::Vector3f ls;
    alignas(16) Eigen::Vector3f ldir;
};

struct MaterialUboHost
{
    alignas(16) Eigen::Vector3f ma;
    alignas(16) Eigen::Vector3f md;
    alignas(16) Eigen::Vector3f ms;
    float ss;
};

Eigen::Matrix4f DepthVisualizer::gl_proj(float _fx, float _fy, float _cx, float _cy, int _width, int _height)
{
	Eigen::Matrix4f cam_proj;
	cam_proj(0, 0) = 2*_fx/_width;
	cam_proj(0, 1) = 0.0f;
	cam_proj(0, 2) = (2*_cx-_width)/_width;
	cam_proj(0, 3) = 0.0f;

	cam_proj(1, 0) = 0.0f;
	cam_proj(1, 1) = -2*_fy/_height;
	cam_proj(1, 2) = (_height-2*_cy)/_height;
	cam_proj(1, 3) = 0.0f;

	cam_proj(2, 0) = 0.0f;
	cam_proj(2, 1) = 0.0f;
	cam_proj(2, 2) = -(far_clip_+near_clip_)/(near_clip_-far_clip_);
	cam_proj(2, 3) = 2*near_clip_*far_clip_/(near_clip_-far_clip_);

	cam_proj(3, 0) = 0.0f;
	cam_proj(3, 1) = 0.0f;
	cam_proj(3, 2) = 1.0f;
	cam_proj(3, 3) = 0.0f;

    return cam_proj;
}

DepthVisualizer::DepthVisualizer(const Camera &_cam,
								 const std::string &_vert_shader_path,
								 const std::string &_frag_shader_path,
                                 float _near_clip,
                                 float _far_clip,
                                 float _max_dist)
{
	cam_ = _cam;

    near_clip_ = _near_clip;
    far_clip_ = _far_clip;

    max_dist = _max_dist;

	// generate a program
    pipeline_ = Pipeline(_vert_shader_path, "", _frag_shader_path).program_id_;

	// create a default vao
	glCreateVertexArrays(1, &default_vao_);

	// create fbo, rbos
	glCreateFramebuffers(1, &fbo_);
	glCreateRenderbuffers(1, &phong_rbo_);
	glCreateRenderbuffers(1, &nmap_rbo_);
    glCreateRenderbuffers(1, &zbuffer_rbo_);

	// allocate storage for rbos
	glNamedRenderbufferStorage(phong_rbo_, GL_RGBA8, cam_.width_, cam_.height_);
    glNamedRenderbufferStorage(nmap_rbo_, GL_RGBA8, cam_.width_, cam_.height_);
	glNamedRenderbufferStorage(zbuffer_rbo_, GL_DEPTH_COMPONENT32F, cam_.width_, cam_.height_);

	// attach rbos to the fbo
	glNamedFramebufferRenderbuffer(fbo_, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, phong_rbo_);
    glNamedFramebufferRenderbuffer(fbo_, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, nmap_rbo_);
	glNamedFramebufferRenderbuffer(fbo_, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, zbuffer_rbo_);

	// check whether the fbo is complete
	if (glCheckNamedFramebufferStatus(fbo_, GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		throw std::runtime_error("ERROR: fbo is not complete");
	}

	// set draw and read buffer of fbo
    GLenum draw_buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
	glNamedFramebufferDrawBuffers(fbo_, 2, draw_buffers);

    // create a texture to store depth map
    glCreateTextures(GL_TEXTURE_2D, 1, &dmap_tex_);
    glTextureStorage2D(dmap_tex_, 1, GL_R16UI, cam_.width_, cam_.height_);

	// create ubo to store camera parameters
    cam_ubo_host.fx = cam_.fx_;
    cam_ubo_host.fy = cam_.fy_;
    cam_ubo_host.cx = cam_.cx_;
    cam_ubo_host.cy = cam_.cy_;
    cam_ubo_host.max_dist = max_dist;
	glCreateBuffers(1, &cam_ubo_);
	glNamedBufferStorage(cam_ubo_, sizeof(CamUboHost), &cam_ubo_host, GL_MAP_READ_BIT);

    // create ubo to store lighting coefficients
    LightUboHost light_coeffs;
	/*light_coeffs.la = Eigen::Vector3f(0.2f, 0.2f, 0.2f);
	light_coeffs.ld = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	light_coeffs.ls = Eigen::Vector3f(1.0f, 1.0f, 1.0f);*/
    light_coeffs.la = Eigen::Vector3f(0.1f, 0.1f, 0.1f);
	light_coeffs.ld = Eigen::Vector3f(0.9f, 0.9f, 0.9f);
	light_coeffs.ls = Eigen::Vector3f(0.3f, 0.3f, 0.3f);
    light_coeffs.ldir = Eigen::Vector3f(0.0f, 0.3f, 0.9f);
    glCreateBuffers(1, &light_ubo_);
    glNamedBufferStorage(light_ubo_, sizeof(LightUboHost), &light_coeffs, GL_MAP_READ_BIT);

    // create ubo to store material parameters
    MaterialUboHost material_coeffs;
    //material_coeffs.ma = Eigen::Vector3f(0.26f, 0.26f, 0.26f);
    //material_coeffs.md = Eigen::Vector3f(0.35f, 0.35f, 0.35f);
	//material_coeffs.ms = Eigen::Vector3f(0.70f, 0.70f, 0.70f);
    material_coeffs.ma = Eigen::Vector3f(0.26f, 0.26f, 0.26f);
    material_coeffs.md = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
    material_coeffs.ms = Eigen::Vector3f(0.35f, 0.35f, 0.35f);
    material_coeffs.ss = 5.5f;
    glCreateBuffers(1, &material_ubo_);
    glNamedBufferStorage(material_ubo_, sizeof(MaterialUboHost), &material_coeffs, GL_MAP_READ_BIT);
}

std::pair<cv::Mat, cv::Mat> DepthVisualizer::visualize(const cv::Mat &_dmap)
{
    // upload depth map to GPU texture
    int dmap_width = _dmap.cols;
    int dmap_height = _dmap.rows;
    if (dmap_width > cam_.width_ || dmap_height > cam_.height_) {
        throw std::runtime_error("ERROR: depth texture is oversize.");
    }

    cv::Mat dmap_pad = cv::Mat::zeros(cam_.height_, cam_.width_, CV_16UC1);
    cv::Point2i top_left((cam_.width_-dmap_width)/2, (cam_.height_-dmap_height)/2);
    _dmap.copyTo(dmap_pad(cv::Rect(top_left.x, top_left.y, dmap_width, dmap_height)));
    glTextureSubImage2D(dmap_tex_, 0, 0, 0, dmap_pad.cols, dmap_pad.rows, GL_RED_INTEGER, GL_UNSIGNED_SHORT, dmap_pad.data);

	// enable depth test
	glEnable(GL_DEPTH_TEST);

	// enable culling
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);

	// bind vao
	glBindVertexArray(default_vao_);
	
    // clear dmap_fbo
    GLfloat color_clear[4] = {0, 0, 0, 0};
    GLfloat z_clear = 1.0f;
    glClearNamedFramebufferfv(fbo_, GL_COLOR, 0, color_clear);
    glClearNamedFramebufferfv(fbo_, GL_COLOR, 1, color_clear);
    glClearNamedFramebufferfv(fbo_, GL_DEPTH, 0, &z_clear);

	// bind fbo
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_);

	// bind program
	glUseProgram(pipeline_);

	// bind ubo
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, cam_ubo_);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, light_ubo_);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, material_ubo_);

    // bind dmap texture
    glBindTextureUnit(0, dmap_tex_);

	// draw call
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// read back dmap_fbo
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glNamedFramebufferReadBuffer(fbo_, GL_COLOR_ATTACHMENT0);
	cv::Mat phong_image = cv::Mat(cam_.height_, cam_.width_, CV_8UC4);
	glReadPixels(0, 0, cam_.width_, cam_.height_, GL_BGRA, GL_UNSIGNED_BYTE, phong_image.data);
    glNamedFramebufferReadBuffer(fbo_, GL_COLOR_ATTACHMENT1);
    cv::Mat normal_image = cv::Mat(cam_.height_, cam_.width_, CV_8UC4);
    glReadPixels(0, 0, cam_.width_, cam_.height_, GL_BGRA, GL_UNSIGNED_BYTE, normal_image.data);

	// flip images
	cv::Mat phong_image2, normal_image2;
	cv::flip(phong_image, phong_image2, 0);
    cv::flip(normal_image, normal_image2, 0);
    return {phong_image2, normal_image2};
}