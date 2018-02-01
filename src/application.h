#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <string>

typedef struct GLFWwindow GLFWwindow;
class DepthVisualizer;

class Application
{
public:
	Application() {}
	~Application();
	void run(int argc, char *argv[]);

private:
	std::string depth_in_path_pattern_;
	std::string phong_out_path_pattern_;
    std::string nmap_out_path_pattern_;

	GLFWwindow *window_ = nullptr;
	DepthVisualizer *renderer_ = nullptr;
};

#endif