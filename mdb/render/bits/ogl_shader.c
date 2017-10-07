#include "ogl_shader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <mdb/tools/log.h>
#include <mdb/tools/compiler.h>


static char* file_read_shader_source(const char* filename)
{
    size_t res_size = BUFSIZ;
    char* res;
    char* p_res;

    size_t nb_read_total = 0;

    FILE* in;

    in = fopen(filename, "rb");
    if (in == NULL)
    {
        LOG_ERROR("Failed to open the file '%s': %s", filename, strerror(errno));
        return NULL;
    }


    res = (char*)malloc(res_size);

    while (!feof(in) && !ferror(in)) {
        if (nb_read_total + BUFSIZ > res_size) {
            if (res_size > 10 * 1024 * 1024)
                break;
            res_size = res_size * 2;
            res = (char*)realloc(res, res_size);
        }
        p_res = res + nb_read_total;
        nb_read_total += fread(p_res, 1, BUFSIZ, in);
    }

    fclose(in);
    res = (char*)realloc(res, nb_read_total + 1);
    res[nb_read_total] = '\0';
    return res;
}

void ogl_shader_log(const char* text, GLuint object)
{
    char* log;
    GLint log_length = 0;

    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        LOG_ERROR("Not a shader or a program '%s'", text);
        return;
    }


    if(log_length == 0)
    {
#if !defined(NDEBUG)
        LOG_DEBUG("%s: OK\n", text);
#else
        UNUSED_PARAM(text);
#endif
        return;
    }

    log = (char*)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    LOG_ERROR("%s: %s", text, log);
    free(log);
}

GLuint ogl_shader_create(const char* filename, const char* defines, GLenum type)
{
    static const char* version_define = "#version 150 \n";

    GLuint res;
    GLint compile_ok = GL_FALSE;

    char *source = file_read_shader_source(filename);

    if (source == NULL)
        return 0;

    res = glCreateShader(type);

    if(defines)
    {
        const char* sources[] = { version_define, defines, source};

        glShaderSource(res, 3, sources, NULL);
    }
    else
    {
        const char* sources[] = {version_define, source};

        glShaderSource(res, 2, sources, NULL);
    }

    glCompileShader(res);
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);

    free(source);

    ogl_shader_log(filename, res);
    if (compile_ok == GL_FALSE) {
        glDeleteShader(res);
        return 0;
    }

    return res;
}

