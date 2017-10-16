#pragma once

#include <glad/glad.h>

GLuint ogl_shader_create(const char* filename, const char* defines,
                         GLenum type);

void ogl_shader_log(const char* text, GLuint object);
