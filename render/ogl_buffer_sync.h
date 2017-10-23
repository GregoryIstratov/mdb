#pragma once

#include <glad/glad.h>

void ogl_buffer_lock(GLsync* syncObj);

void ogl_buffer_wait(GLsync* syncObj);
