#ifndef _PIPELINE_H_
#define _PIPELINE_H_

#include <string>
#include <glad/glad.h>

struct Pipeline
{
    Pipeline(const std::string &_vert_shader_path,
             const std::string &_geom_shader_path,
             const std::string &_frag_shader_path);

    GLuint program_id_;
};

#endif