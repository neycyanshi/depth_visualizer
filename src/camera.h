#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <Eigen/core>

struct Camera
{
    int width_, height_;
    float fx_, fy_, cx_, cy_;
};

#endif