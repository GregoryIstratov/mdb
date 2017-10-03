#pragma once

#include <glad/glad.h>
#include <stdbool.h>

typedef struct _ogl_squad ogl_squad;

void ogl_squad_create(ogl_squad** psquad, bool color_enabled);

void ogl_squad_destroy(ogl_squad* squad);

void ogl_squad_begin(ogl_squad* squad);

void ogl_squad_set_exposure(ogl_squad* squad, float exposure);

void ogl_squad_end(ogl_squad* squad);