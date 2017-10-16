#include "ogl_render.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <mdb/tools/compiler.h>
#include <mdb/tools/timer.h>
#include <mdb/tools/log.h>


#include "ogl_shader.h"
#include "ogl_pixel_buffer.h"
#include "ogl_buffer_sync.h"
#include "ogl_squad.h"

#define PBO_SIZE_MULTIPLIER 8

struct _ogl_render
{
        uint32_t window_width;
        uint32_t window_height;
        GLFWwindow* window;

        void* user_ctx;
        ogl_render_key_callback user_key_cb;
        ogl_render_resize_callback resize_cb;

        ogl_pixel_buffer* pbo;

        ogl_squad* squad;

        float exposure;

        GLsync sync;

};
static ogl_render* g_rend;

static
void error_callback(int error, const char* description)
{
        UNUSED_PARAM(error);

        LOG_ERROR("Error: %s", description);
}


static
void APIENTRY openglCallbackFunction(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
)
{
        UNUSED_PARAM(message);
        (void)source; (void)type; (void)id;
        (void)severity; (void)length; (void)userParam;

        LOG_DEBUG("%s", message);

        if (severity == GL_DEBUG_SEVERITY_HIGH)
        {
                LOG_ERROR("GL DECIDED TO ABORT...");
                abort();
        }
}

static
void key_callback(GLFWwindow* window, int key, int scancode,
                  int action, int mods)
{
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {
                glfwSetWindowShouldClose(window, GL_TRUE);
                return;
        }

        if (action == GLFW_PRESS || action == GLFW_REPEAT)
        {

                switch(key)
                {
                case GLFW_KEY_5:
                        g_rend->exposure *= 0.9;
                        PARAM_INFO("exposure","%f", g_rend->exposure);
                        return;

                case GLFW_KEY_6:
                        g_rend->exposure *= 1.1;
                        PARAM_INFO("exposure","%f", g_rend->exposure);
                        return;

                default:
                        break;
                }
        }

        if (g_rend->user_key_cb)
                g_rend->user_key_cb(g_rend->user_ctx, key,
                                    scancode, action, mods);
}

static
void resize_callback(GLFWwindow* window, int width, int height)
{
        UNUSED_PARAM(window);

        g_rend->window_width = width;
        g_rend->window_height = height;

        if(g_rend->resize_cb)
        {
                int rem = width % PBO_SIZE_MULTIPLIER;

                if(rem)
                {
                        width = width - rem;
                }

                ogl_pbo_resize(g_rend->pbo, &g_rend->sync, width, height);
                g_rend->resize_cb(width, height, g_rend->user_ctx);
        }
}


void ogl_render_create(ogl_render** _rend, const char* win_title,
                       uint32_t width, uint32_t height, void* user_ctx)
{
        ogl_render* rend = calloc(1, sizeof(**_rend));
        *_rend = rend;
        g_rend = rend;

        rend->window_height = height;
        rend->window_width  = width;

        rend->user_ctx      = user_ctx;

        rend->exposure      = 1.0f;

        glfwSetErrorCallback(error_callback);

        if (!glfwInit())
                exit(EXIT_FAILURE);


        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        rend->window = glfwCreateWindow(width, height, win_title, NULL, NULL);

        if (!rend->window)
        {
                glfwTerminate();
                exit(EXIT_FAILURE);
        }

#if GLFW_VERSION_MAJOR >= 3 && GLFW_VERSION_MINOR >= 2
        glfwSetWindowSizeLimits(rend->window, 640, 480,
                                GLFW_DONT_CARE, GLFW_DONT_CARE);
#endif

        glfwSetKeyCallback(rend->window, &key_callback);

        /* FIXME disabled due to spurious buffer storage corruption on resize */
        /* glfwSetWindowSizeCallback(rend->window, &resize_callback); */

        glfwMakeContextCurrent(rend->window);

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
        {
                LOG_ERROR("Failed to initialize OpenGL context");
                exit(EXIT_FAILURE);
        }

        if (!glfwExtensionSupported("GL_ARB_buffer_storage"))
        {
                LOG_ERROR("GL_ARB_buffer_storage is not supported.");
                exit(EXIT_FAILURE);
        }

        LOG_INFO("OpenGL %s, GLSL %s",
                 glGetString(GL_VERSION),
                 glGetString(GL_SHADING_LANGUAGE_VERSION));


#if !defined(NDEBUG) && defined(CONFIG_OGL_DEBUG_OUTPUT)
        if (glfwExtensionSupported("GL_ARB_debug_output"))
        {
                /* Enable the debug callback */
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(openglCallbackFunction, NULL);
                glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE,
                                      0, NULL, 1);
        }
#endif

        glEnable(GL_FRAMEBUFFER_SRGB);

        glfwSwapInterval(0);
}

void ogl_render_init_render_target(ogl_render* rend,
                                   ogl_data_update_callback cb)
{
        int width_rem = rend->window_width % PBO_SIZE_MULTIPLIER;
        if(width_rem)
        {
                rend->window_width -= width_rem;
        }

        ogl_pbo_create(&rend->pbo, rend->window_width,
                       rend->window_height, cb, rend->user_ctx);
}

void ogl_render_init_screen(ogl_render* rend, bool color_enabled)
{
        ogl_squad_create(&rend->squad, color_enabled);
}

void ogl_render_set_key_callback(ogl_render* rend, ogl_render_key_callback cb)
{
        rend->user_key_cb = cb;
}

void ogl_render_set_resize_callback(ogl_render* rend,
                                    ogl_render_resize_callback cb)
{
        rend->resize_cb = cb;
}


void ogl_render_render_loop(ogl_render* rend)
{
        char title[32];
        double start, end, elapsed;
        uint32_t fps;

        while (!glfwWindowShouldClose(rend->window))
        {
                start = sample_timer();

                ogl_pbo_update(rend->pbo, &rend->sync);

                ogl_buffer_wait(&rend->sync);

                /* If resize enabled
                 *
                int vp_bot = 0;
                int vp_left = rend->window_width - rend->pbo->width;

                if(vp_left < 0)
                {
                    LOG_ERROR("PBO size is greater than window size. "
                              "Setting left offset to 0");
                    vp_left = 0;
                }

                glViewport(vp_left, vp_bot, rend->pbo.width, rend->pbo.height);

                */

                glViewport(0, 0, rend->window_width, rend->window_height);

                glClearColor(1.0, 1.0, 1.0, 1.0);
                glClear(GL_COLOR_BUFFER_BIT);

                ogl_squad_begin(rend->squad);

                ogl_squad_set_exposure(rend->squad, rend->exposure);

                ogl_pbo_bind_texture0(rend->pbo);

                ogl_squad_end(rend->squad);

                ogl_buffer_lock(&rend->sync);

                glfwSwapBuffers(rend->window);

                glfwPollEvents();

                end = sample_timer();
                elapsed = end - start;
                fps = (uint32_t)(1.0/elapsed);

                snprintf(title, 32, "f:%.3f %u", elapsed, fps);

                glfwSetWindowTitle(rend->window, title);
        }
}

void ogl_render_destroy(ogl_render* rend)
{
        glfwDestroyWindow(rend->window);

        ogl_pbo_destroy(rend->pbo);
        ogl_squad_destroy(rend->squad);

        free(rend);
}
