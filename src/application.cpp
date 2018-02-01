#include "application.h"
#include <stdexcept>
#include <fstream>
#include <string>
#include <json/json.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <Eigen/Eigen>
#include "renderer.h"
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include "camera.h"

Application::~Application()
{
	glfwTerminate();
	delete renderer_;
}

void Application::run(int argc, char *argv[])
{
	//////////////////////////////////////////////////////////////////////////
	// parse the config file
	if (argc != 2) {
		throw std::runtime_error("ERROR: invalid input\ndepth_rasterizer <config.json>");
	}

	std::ifstream config_istream(argv[1]);
	if (!config_istream) {
		throw std::runtime_error("ERROR: failed to open config.json");
	}

	Json::CharReaderBuilder json_reader;
	Json::Value json_root;
	std::string json_error;
	if (!Json::parseFromStream(json_reader, config_istream, &json_root, &json_error)) {
		throw std::runtime_error("ERROR: JSON failed to parse the input parameters\n" + json_error);
	}

	// load path parameters
	depth_in_path_pattern_ = json_root["in_path_pattern"].asString();
	phong_out_path_pattern_ = json_root["phong_out_path_pattern"].asString();
    nmap_out_path_pattern_ = json_root["normal_out_path_pattern"].asString();

    // load shader path
    std::string vert_shader_path = json_root["vert_shader_path"].asString();
    std::string frag_shader_path = json_root["frag_shader_path"].asString();

	// load camera parameters
    Camera cam;
	cam.width_ = json_root["camera"]["width"].asInt();
	cam.height_ = json_root["camera"]["height"].asInt();
	cam.fx_ = json_root["camera"]["fx"].asFloat();
	cam.fy_ = json_root["camera"]["fy"].asFloat();
	cam.cx_ = json_root["camera"]["cx"].asFloat();
	cam.cy_ = json_root["camera"]["cy"].asFloat();

    float near_clip = json_root["near_clip"].asFloat();
    float far_clip = json_root["far_clip"].asFloat();
    float max_dist = json_root["max_dist"].asFloat();

	//////////////////////////////////////////////////////////////////////////
	// create OpenGL context and load OpenGL functions
	if (!glfwInit()) {
		throw std::runtime_error("ERROR: failed to initialize GLFW");
	}

	// create OpenGL context
	glfwWindowHint(GLFW_STEREO, 0);
	glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
	glfwWindowHint(GLFW_VISIBLE, 0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (!(window_ = glfwCreateWindow(cam.width_, cam.height_, "DEPTH_RASTERIZER", 0, 0))) {
		throw std::runtime_error("ERROR: failed to create an OpenGL context");
	}

	// make the context current
	glfwMakeContextCurrent(window_);

	// load OpenGL functions
	if (!gladLoadGL()) {
		throw std::runtime_error("ERROR: failed to load OpenGL functions");
	}

	//////////////////////////////////////////////////////////////////////////
	// create a depth rasterizer
	renderer_ = new DepthVisualizer(cam, vert_shader_path, frag_shader_path, near_clip, far_clip, max_dist);

    int start_frame = json_root["start_frame"].asInt();
    int end_frame = json_root["end_frame"].asInt();
    for (int fid = start_frame; fid <= end_frame; ++fid) {

        // load depth map
        char dmap_path[256] = {0};
        sprintf(dmap_path, depth_in_path_pattern_.c_str(), fid);
        cv::Mat dmap = cv::imread(dmap_path, cv::IMREAD_ANYDEPTH);

        // render phong and normal maps
        cv::Mat phong_map, nmap;
        std::tie(phong_map, nmap) = renderer_->visualize(dmap);

        // phong
        char phong_path[256] = {0};
        sprintf(phong_path, phong_out_path_pattern_.c_str(), fid);
        cv::imwrite(phong_path, phong_map);

        // normal
        char nmap_path[256] = {0};
        sprintf(nmap_path, nmap_out_path_pattern_.c_str(), fid);
        cv::imwrite(nmap_path, nmap);

        printf("frame %04d is done.\n", fid);
    }
}