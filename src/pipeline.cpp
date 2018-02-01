#include "pipeline.h"
#include <fstream>

Pipeline::Pipeline(const std::string &_vert_shader_path,
                   const std::string &_geom_shader_path,
                   const std::string &_frag_shader_path)
{
    //////////////////////////////////////////////////////////////////////////
    // compile vertex shader
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    std::ifstream read_vert_src(_vert_shader_path, std::ios_base::binary);
    if (!read_vert_src) {
        throw std::runtime_error("ERROR: failed to open vertex shader");
    }
    read_vert_src.seekg(0, std::ios_base::end);
    size_t vert_src_len = read_vert_src.tellg();
    read_vert_src.seekg(0, std::ios_base::beg);
    std::string vert_src(vert_src_len, 0);
    read_vert_src.read(&vert_src[0], vert_src_len);
    read_vert_src.close();

    const char *vert_src_cstr = vert_src.c_str();
    glShaderSource(vert_shader, 1, (const GLchar**)&vert_src_cstr, 0);
    glCompileShader(vert_shader);

    // query whether vertex shader has been successfully compiled
    int vert_shader_status = 0;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_shader_status);
    if (!vert_shader_status) {
        int max_info_len = 2048;
        std::string vert_shader_error(max_info_len, 0);
        glGetShaderInfoLog(vert_shader, max_info_len, &max_info_len, &vert_shader_error[0]);
        throw std::runtime_error("VERTEX SHADER ERROR:\n" + vert_shader_error.substr(0, max_info_len));
    }

    //////////////////////////////////////////////////////////////////////////
    // compile geometry shader (optional)
    GLuint geom_shader = glCreateShader(GL_GEOMETRY_SHADER);
    if (!_geom_shader_path.empty()) {
        std::ifstream read_geom_src(_geom_shader_path, std::ios_base::binary);
        if (!read_geom_src) {
            throw std::runtime_error("ERROR: failed to open geometry shader");
        }
        read_geom_src.seekg(0, std::ios_base::end);
        size_t geom_src_len = read_geom_src.tellg();
        read_geom_src.seekg(0, std::ios_base::beg);
        std::string geom_src(geom_src_len, 0);
        read_geom_src.read(&geom_src[0], geom_src_len);
        read_geom_src.close();

        const char *geom_src_cstr = geom_src.c_str();
        glShaderSource(geom_shader, 1, (const GLchar**)&geom_src_cstr, 0);
        glCompileShader(geom_shader);

        // query whether geometry shader has been successfully compiled
        int geom_shader_status = 0;
        glGetShaderiv(geom_shader, GL_COMPILE_STATUS, &geom_shader_status);
        if (!geom_shader_status) {
            int max_info_len = 2048;
            std::string geom_shader_error(max_info_len, 0);
            glGetShaderInfoLog(geom_shader, max_info_len, &max_info_len, &geom_shader_error[0]);
            throw std::runtime_error("GEOMETRY SHADER ERROR:\n" + geom_shader_error.substr(0, max_info_len));
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // compile fragment shader
    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    std::ifstream read_frag_src(_frag_shader_path, std::ios_base::binary);
    if (!read_frag_src) {
        throw std::runtime_error("ERROR: failed to open fragment shader");
    }
    read_frag_src.seekg(0, std::ios_base::end);
    size_t frag_src_len = read_frag_src.tellg();
    read_frag_src.seekg(0, std::ios_base::beg);
    std::string frag_src(frag_src_len, 0);
    read_frag_src.read(&frag_src[0], frag_src_len);
    read_frag_src.close();

    const char *frag_src_cstr = frag_src.c_str();
    glShaderSource(frag_shader, 1, (const GLchar**)&frag_src_cstr, 0);
    glCompileShader(frag_shader);

    // query whether fragment shader has been successfully compile
    int frag_shader_status = 0;
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_shader_status);
    if (!frag_shader_status) {
        int max_info_len = 2048;
        std::string frag_shader_error(max_info_len, 0);
        glGetShaderInfoLog(frag_shader, max_info_len, &max_info_len, &frag_shader_error[0]);
        throw std::runtime_error("FRAGMENT SHADER ERROR:\n" + frag_shader_error.substr(0, max_info_len));
    }

    //////////////////////////////////////////////////////////////////////////
    // link shaders to program
    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vert_shader);
    if (!_geom_shader_path.empty()) {
        glAttachShader(program_id_, geom_shader);
    }
    glAttachShader(program_id_, frag_shader);
    glLinkProgram(program_id_);
    GLint program_status = 0;
    glGetProgramiv(program_id_, GL_LINK_STATUS, &program_status);
    if (!program_status) {
        int max_info_len = 2048;
        std::string program_error(max_info_len, 0);
        glGetProgramInfoLog(program_id_, max_info_len, &max_info_len, &program_error[0]);
        throw std::runtime_error("PROGRAM LINKING ERROR:\n" + program_error.substr(0, max_info_len));
    }
}