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
#include <mdb/tools/utils.h>

#define PRINT_ERR(msg,...) { fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, ": %m %s:%i\n", __FILE__, __LINE__); }
#define PRINT_ERR_EXIT(msg, ... ) { PRINT_ERR(msg, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define LOG_PARAM(label, fmt, ...) { fprintf(stdout, "%-20s: ", (label)); fprintf(stdout, (fmt), ##__VA_ARGS__); fprintf(stdout,"\n"); fflush(stdout); }
#define LOG_DEBUG(label, fmt, ...) LOG_PARAM(label, fmt, ##__VA_ARGS__)
#define PTHREAD_CHECK_RETURN(exp) { int ret; if((ret = (exp))) { fprintf(stderr, "%s: %s %s:%i\n", #exp, strerror(ret), __FILE__, __LINE__); exit(EXIT_FAILURE); } }
#define CHECK_RETURN_ERRNO(exp) { if(exp) { fprintf(stderr, "%s: %s %s:%i\n", #exp, strerror(errno), __FILE__, __LINE__); exit(EXIT_FAILURE); } }

#define PBO_SIZE_MULTIPLIER 8

static char* file_read(const char* filename)
{
    FILE* in = fopen(filename, "rb");
    if (in == NULL)
    {
        fprintf(stderr, "Failed to open the file '%s': %s", filename, strerror(errno));
        fflush(stderr);
        return NULL;
    }

    size_t res_size = BUFSIZ;
    char* res = (char*)malloc(res_size);
    size_t nb_read_total = 0;

    while (!feof(in) && !ferror(in)) {
        if (nb_read_total + BUFSIZ > res_size) {
            if (res_size > 10 * 1024 * 1024)
                break;
            res_size = res_size * 2;
            res = (char*)realloc(res, res_size);
        }
        char* p_res = res + nb_read_total;
        nb_read_total += fread(p_res, 1, BUFSIZ, in);
    }

    fclose(in);
    res = (char*)realloc(res, nb_read_total + 1);
    res[nb_read_total] = '\0';
    return res;
}

static void file_read_all(const char* filename, size_t* size, void** data)
{
    FILE* in = fopen(filename, "rb");
    if (in == NULL)
    {
        fprintf(stderr, "Failed to open the file '%s': %s", filename, strerror(errno));
        fflush(stderr);
        exit(EXIT_FAILURE);
    }

    fseek(in, 0, SEEK_END);

    *size = (size_t)ftell(in);

    rewind(in);


    *data = calloc(1, *size);

    if((*data) == NULL)
    {
        PRINT_ERR_EXIT("[file_read_all] calloc failed");
    }

    size_t read = fread(*data, 1, *size, in);

    if (read != *size)
    {
        PRINT_ERR_EXIT("[file_read_all] fread has read not all bytes in stream read = %lu != total = %lu", read, (*size));
    }

    if (ferror(in))
    {
        PRINT_ERR_EXIT("[file_read_all] fread error");
    }

    fclose(in);
}

static void print_log(GLuint object)
{
    GLint log_length = 0;
    if (glIsShader(object))
        glGetShaderiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else if (glIsProgram(object))
        glGetProgramiv(object, GL_INFO_LOG_LENGTH, &log_length);
    else {
        fprintf(stderr, "printlog: Not a shader or a program\n");
        return;
    }

    if(log_length == 0)
    {
        fprintf(stderr, "OK\n");
        return;
    }

    char* log = (char*)malloc(log_length);

    if (glIsShader(object))
        glGetShaderInfoLog(object, log_length, NULL, log);
    else if (glIsProgram(object))
        glGetProgramInfoLog(object, log_length, NULL, log);

    fprintf(stderr, "%s", log);
    free(log);
}

static GLuint create_shader(const char* filename, GLenum type)
{

    char *source = file_read(filename);

    if (source == NULL)
        return 0;

    const char* sources[] = {source };


    GLuint res = glCreateShader(type);

    glShaderSource(res, 1, sources, NULL);

    glCompileShader(res);
    GLint compile_ok = GL_FALSE;
    glGetShaderiv(res, GL_COMPILE_STATUS, &compile_ok);

    free(source);

    fprintf(stderr, "%s: ", filename);
    print_log(res);
    if (compile_ok == GL_FALSE) {
        glDeleteShader(res);
        return 0;
    }

    return res;
}


static GLuint create_screen_quad_program()
{
    GLint link_ok = GL_FALSE;
    GLuint gs, vs, fs;
    GLuint program;

    if ((gs = create_shader("shaders/screen_quad_gs.glsl", GL_GEOMETRY_SHADER)) == 0)
        exit(EXIT_FAILURE);
    if ((vs = create_shader("shaders/screen_quad_gs_vs.glsl", GL_VERTEX_SHADER)) == 0)
        exit(EXIT_FAILURE);
    if ((fs = create_shader("shaders/screen_quad_fs.glsl", GL_FRAGMENT_SHADER)) == 0)
        exit(EXIT_FAILURE);


    program = glCreateProgram();

    glAttachShader(program, gs);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
        fprintf(stderr, "glLinkProgram:");
        print_log(program);
        exit(EXIT_FAILURE);
    }

    return program;
}


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}


struct pbo_t
{
    GLuint texture_id;
    GLuint buffer_id;
    uint32_t buff_size;
    uint32_t width;
    uint32_t height;
    data_update_callback data_update;
    void* update_context;
};

struct _ogl_render
{
    uint32_t window_width;
    uint32_t window_height;
    GLuint vao;
    GLFWwindow* window;
    GLuint program;
    struct pbo_t pbo;
};

static float g_exposure = 1.0;
static void* g_user_ctx;
static ogl_render_key_callback g_user_key_callback;
static ogl_render_resize_callback g_resize_cb;

static ogl_render* g_rend;
static GLsync gSyncObject;
static const size_t MAX_BUFFER_COUNT = 3;

static bool gParamSyncBuffers   = true;
static size_t gParamBufferCount = 1;

static size_t gWaitCount = 0;

void LockBuffer(GLsync* syncObj)
{
    if (*syncObj)
        glDeleteSync(*syncObj);

    *syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void WaitBuffer(GLsync* syncObj)
{
    if (*syncObj)
    {
        while (1)
        {
            GLenum waitReturn = glClientWaitSync(*syncObj, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED)
                return;

            gWaitCount++;
        }
    }
}

static void create_pbo(struct pbo_t* pbo, uint32_t width, uint32_t height, data_update_callback callback, void* ctx)
{
    pbo->width = width;
    pbo->height = height;
    pbo->buff_size = width*height*sizeof(float);
    pbo->data_update = callback;
    pbo->update_context = ctx;

    glGenBuffers(1, &pbo->buffer_id);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo->buffer_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo->buff_size, NULL, GL_STREAM_DRAW);


    glGenTextures(1, &pbo->texture_id);
    glBindTexture(GL_TEXTURE_2D, pbo->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, pbo->width, pbo->height, 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

static void update_pbo(struct pbo_t* pbo)
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo->buffer_id);

    void* pbo_mem_ptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);

    if(!pbo_mem_ptr)
    {
        fprintf(stderr, "Can't map buffer");
        exit(EXIT_FAILURE);
    }

    //glClientWaitSync(g_sync, GL_SYNC_FLUSH_COMMANDS_BIT, 0);

    pbo->data_update(pbo_mem_ptr, pbo->update_context);

    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

}

static void render_pbo(struct pbo_t* pbo)
{
    glActiveTexture(GL_TEXTURE0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo->buffer_id);
    glBindTexture(GL_TEXTURE_2D, pbo->texture_id);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pbo->width, pbo->height, GL_RED, GL_FLOAT, NULL);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

static void destroy_pbo(struct pbo_t* pbo)
{

}


static void resize_pbo(struct pbo_t* pbo, uint32_t width, uint32_t height)
{

    printf("[resize_pbo] enter\n");

    //glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    //glBindTexture(GL_TEXTURE_2D, 0);

    //glDeleteBuffers(1, &pbo->buffer_id);
    //glDeleteTextures(1, &pbo->texture_id);

    pbo->width = width;
    pbo->height = height;
    pbo->buff_size = width*height*sizeof(float);

    //glGenBuffers(1, &pbo->buffer_id);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo->buffer_id);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, pbo->buff_size, 0, GL_STREAM_DRAW);


    //glGenTextures(1, &pbo->texture_id);
    glBindTexture(GL_TEXTURE_2D, pbo->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, pbo->width, pbo->height, 0, GL_RED, GL_FLOAT, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    printf("[resize_pbo] leave\n");
}

static void APIENTRY openglCallbackFunction(
        GLenum source,
        GLenum type,
        GLuint id,
        GLenum severity,
        GLsizei length,
        const GLchar* message,
        const void* userParam
){
    (void)source; (void)type; (void)id;
    (void)severity; (void)length; (void)userParam;
    fprintf(stderr, "%s\n", message);
    if (severity==GL_DEBUG_SEVERITY_HIGH) {
        fprintf(stderr, "GL DECIDED TO ABORT...\n");
        abort();
    }
}

static double get_total_sec(const struct timespec* ts)
{
    static const double NS_IN_SEC = 1000000000;
    if (ts->tv_sec == 0)
        return (double) ts->tv_nsec / NS_IN_SEC;

    double total_sec;
    total_sec = (double) ts->tv_nsec / NS_IN_SEC;
    total_sec += (double) ts->tv_sec;

    return total_sec;
}


static double sample_timer(void)
{
    struct timespec tm;

    clock_gettime(CLOCK_MONOTONIC, &tm);

    return get_total_sec(&tm);

}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {

        switch(key)
        {
            case GLFW_KEY_5:
            {
                g_exposure *= 0.9;
                LOG_PARAM("exposure","%f", g_exposure);
                break;
            }
            case GLFW_KEY_6:
            {
                g_exposure *= 1.1;
                LOG_PARAM("exposure","%f", g_exposure);
                break;
            }

            default:
            {
                if (g_user_key_callback)
                    g_user_key_callback(key, g_user_ctx);
            }
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

static void resize_callback(GLFWwindow* window, int width, int height)
{
    g_rend->window_width = (uint32_t)width;
    g_rend->window_height = (uint32_t)height;

    if(g_resize_cb)
    {
        int rem = width % PBO_SIZE_MULTIPLIER;

        if(rem)
        {
            width = width - rem;
            //glfwSetWindowSize(window, width, height);
            //return;
        }

        resize_pbo(&g_rend->pbo, width, height);
        g_resize_cb(width, height, g_user_ctx);
    }
}


void ogl_render_create(ogl_render** _rend, uint32_t width, uint32_t height, data_update_callback cb, void* user_ctx, ogl_render_key_callback key_cb)
{
    ogl_render* rend = calloc(1, sizeof(ogl_render));
    g_rend = rend;

    rend->window_height = height;
    rend->window_width  = width;
    g_user_ctx = user_ctx;
    g_user_key_callback = key_cb;


    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    rend->window = glfwCreateWindow(width, height, "Mandlebrot", NULL, NULL);

    if (!rend->window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetWindowSizeLimits(rend->window, 640, 480, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwSetKeyCallback(rend->window, key_callback);
    glfwSetWindowSizeCallback(rend->window, &resize_callback);

    glfwMakeContextCurrent(rend->window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize OpenGL context\n");
        exit(EXIT_FAILURE);
    }

    // Enable the debug callback
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(openglCallbackFunction, NULL);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, 1);

    glEnable(GL_FRAMEBUFFER_SRGB);

    glfwSwapInterval(0);

    printf("OpenGL %d.%d\n", GLVersion.major, GLVersion.minor);
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

    rend->program = create_screen_quad_program();

    int width_rem = width % PBO_SIZE_MULTIPLIER;
    if(width_rem)
    {
        width -= width_rem;
    }

    create_pbo(&rend->pbo, width, height, cb, user_ctx);

    glGenVertexArrays(1, &rend->vao);

    *_rend = rend;
}

void ogl_render_render_loop(ogl_render* rend)
{
    char title[32];
    while (!glfwWindowShouldClose(rend->window))
    {
        double start = sample_timer();

        //glfwGetFramebufferSize(rend->window, (int*)&rend->width, (int*)&rend->height);

        //glViewport(0, 0, rend->width, rend->height);

        int vp_bot = 0;
        int vp_left = rend->window_width - rend->pbo.width;

        if(vp_left < 0)
        {
            LOG_ERROR("PBO size is greater than window size. Setting left offset to 0");
            vp_left = 0;
        }

        glViewport(vp_left, vp_bot, rend->pbo.width, rend->pbo.height);

        glClearColor(1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(rend->program);

//        GLint width_location = glGetUniformLocation(program, "width");
//        GLint height_location = glGetUniformLocation(program, "height");
//        GLint scale_location = glGetUniformLocation(program, "scale");
//        GLint x_location = glGetUniformLocation(program, "x_shift");
//        GLint y_location = glGetUniformLocation(program, "y_shift");
//        GLint iterations_location = glGetUniformLocation(program, "iterations");

//        glUniform1i(width_location, width);
//        glUniform1i(height_location, height);
//        glUniform1d(scale_location, scale);
//        glUniform1d(x_location, x_shift);
//        glUniform1d(y_location, y_shift);
//        glUniform1i(iterations_location, iterations);

        WaitBuffer(&gSyncObject);

        GLint exposure_location = glGetUniformLocation(rend->program, "exposure");
        glUniform1f(exposure_location, g_exposure);

        update_pbo(&rend->pbo);
        render_pbo(&rend->pbo);

        glBindVertexArray(rend->vao);

        glDrawArrays(GL_POINTS, 0, 1);

        LockBuffer(&gSyncObject);

        glfwSwapBuffers(rend->window);

        glfwPollEvents();

        double end = sample_timer();
        double elapsed = end - start;
        uint32_t fps = (uint32_t)(1.0/elapsed);
        snprintf(title, 32, "f:%.3f %u", elapsed, fps);
        glfwSetWindowTitle(rend->window, title);
    }
}

void ogl_render_destroy(ogl_render* rend)
{
    glfwDestroyWindow(rend->window);
    glDeleteProgram(rend->program);
    glDeleteVertexArrays(1, &rend->vao);

    free(rend);
}

void ogl_render_set_resize_callback(ogl_render* rend, ogl_render_resize_callback cb)
{
    g_resize_cb = cb;
}