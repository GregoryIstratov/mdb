#include "ogl_squad.h"
#include "ogl_shader.h"

#include <stdlib.h>

struct _ogl_squad
{
    GLuint vao;
    GLuint vbo;
    GLuint program;
};

static const GLfloat squad_data[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f,  1.0f, 0.0f,
};


static GLuint ogl_squad_create_program(bool color_enabled)
{
    const char* color_define = "#define ENABLE_COLORS \n";

    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    GLuint program;

    if ((vs = ogl_shader_create("shaders/screen_quad_vs.glsl", NULL, GL_VERTEX_SHADER)) == 0)
        exit(EXIT_FAILURE);
    if ((fs = ogl_shader_create("shaders/screen_quad_fs.glsl",color_enabled? color_define : NULL, GL_FRAGMENT_SHADER)) == 0)
        exit(EXIT_FAILURE);


    program = glCreateProgram();

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        ogl_shader_log("glLinkProgram", program);
        exit(EXIT_FAILURE);
    }

    return program;
}

void ogl_squad_create(ogl_squad** psquad, bool color_enabled)
{
    *psquad = calloc(1, sizeof(ogl_squad));
    ogl_squad* squad = *psquad;

    glGenVertexArrays(1, &squad->vao);

    glBindVertexArray(squad->vao);

    glGenBuffers(1, &squad->vbo);

    glBindBuffer(GL_ARRAY_BUFFER, squad->vbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(squad_data), squad_data, GL_STATIC_DRAW);


    squad->program = ogl_squad_create_program(color_enabled);
}

void ogl_squad_destroy(ogl_squad* squad)
{
    glDeleteProgram(squad->program);
    glDeleteVertexArrays(1, &squad->vao);
    free(squad);
}

void ogl_squad_begin(ogl_squad* squad)
{
    glUseProgram(squad->program);
}

void ogl_squad_set_exposure(ogl_squad* squad, float exposure)
{
    GLint exposure_location = glGetUniformLocation(squad->program, "exposure");
    glUniform1f(exposure_location, exposure);
}

void ogl_squad_end(ogl_squad* squad)
{
    glBindVertexArray(squad->vao);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, squad->vbo);
    glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );


    //update_pbo(&rend->pbo);
    //render_pbo(&rend->pbo);

    // Draw the triangles !
    glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

    glDisableVertexAttribArray(0);
}