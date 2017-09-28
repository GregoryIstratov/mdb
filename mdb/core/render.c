#include "render.h"
#include <GLFW/glfw3.h>

#include <mdb/sched/rsched.h>
#include <mdb/kernel/mdb_kernel.h>
#include <mdb/tools/utils.h>
#include <string.h>
#include <mdb/tools/log.h>

#if defined(OGL_RENDER_ENABLED)
#include <mdb/render/ogl_render.h>
#endif

static const char* render_control_keys =
        "Arrows  - Move Up/Down/Left/Right\n"
                "[1]/[2] - Scale up/down\n"
                "[3]/[4] - Iterations/bailout increase/decrease\n"
                "[5]/[6] - Exposure increase/decrease\n";


struct render_ctx
{
    rsched* sched;
    mdb_kernel* kernel;
    float shift_x, shift_y, scale;
    uint32_t bailout;
    uint32_t width;
    uint32_t height;
    struct block_size grain;
};



static void render_update_scale(struct render_ctx* ctx)
{
    mdb_kernel_set_scale(ctx->kernel, ctx->scale);
    mdb_kernel_submit_changes(ctx->kernel);
    PARAM_INFO("scale", "%f", ctx->scale);
}

static void render_update_shift(struct render_ctx* ctx)
{
    mdb_kernel_set_shift(ctx->kernel, ctx->shift_x, ctx->shift_y);
    mdb_kernel_submit_changes(ctx->kernel);
    PARAM_INFO("shift", "%f %f", ctx->shift_x, ctx->shift_y);
}

static void render_update_bailout(struct render_ctx* ctx)
{
    mdb_kernel_set_bailout(ctx->kernel, ctx->bailout);
    PARAM_INFO("bailout", "%u", ctx->bailout);
}

static uint32_t iterations_get_mod(struct render_ctx* ctx)
{
    if (ctx->bailout < 256)
        return 1;
    if (ctx->bailout < 512)
        return 2;
    if (ctx->bailout < 1024)
        return 4;
    if (ctx->bailout < 2048)
        return 8;
    if (ctx->bailout < 4096)
        return 32;
    if (ctx->bailout < 8192)
        return 128;
    if (ctx->bailout < 16384)
        return 256;

    return 512;
}

static void iterations_increase(struct render_ctx* ctx)
{
    ctx->bailout += iterations_get_mod(ctx);
}

static void iterations_decrease(struct render_ctx* ctx)
{
    if (ctx->bailout > 1) ctx->bailout -= iterations_get_mod(ctx);
}

static void shift_right(struct render_ctx* ctx)
{
    ctx->shift_x += 0.1 * ctx->scale;
}

static void shift_left(struct render_ctx* ctx)
{
    ctx->shift_x -= 0.1 * ctx->scale;
}

static void shift_up(struct render_ctx* ctx)
{
    ctx->shift_y -= 0.1 * ctx->scale;
}

static void shift_down(struct render_ctx* ctx)
{
    ctx->shift_y += 0.1 * ctx->scale;
}

static void render_key_callback(int key, void* user_ctx)
{
    struct render_ctx* ctx = (struct render_ctx*)user_ctx;

    switch (key)
    {
        case GLFW_KEY_1:
        {
            ctx->scale *= 1.1;
            render_update_scale(ctx);
            return;
        }
        case GLFW_KEY_2:
        {
            ctx->scale *= 0.9;
            render_update_scale(ctx);
            return;
        }
        case GLFW_KEY_3:
        {
            iterations_decrease(ctx);
            render_update_bailout(ctx);
            return;
        }
        case GLFW_KEY_4:
        {
            iterations_increase(ctx);
            render_update_bailout(ctx);
            return;
        }
        case GLFW_KEY_RIGHT:
        {
            shift_right(ctx);
            render_update_shift(ctx);
            return;
        }
        case GLFW_KEY_LEFT:
        {
            shift_left(ctx);
            render_update_shift(ctx);
            return;
        }
        case GLFW_KEY_UP:
        {
            shift_up(ctx);
            render_update_shift(ctx);
            return;
        }
        case GLFW_KEY_DOWN:
        {
            shift_down(ctx);
            render_update_shift(ctx);
            return;
        }
    }
}

static void render_resize(int width, int height, void* context)
{
    struct render_ctx* ctx = (struct render_ctx*)context;

    ctx->width  = (uint32_t)width;
    ctx->height = (uint32_t)height;

    mdb_kernel_set_size(ctx->kernel, ctx->width, ctx->height);
    mdb_kernel_submit_changes(ctx->kernel);

    //rsched_queue_resize(ctx->sched, ctx->width * ctx->height / (ctx->grain.x * ctx->grain.y));
    rsched_create_tasks(ctx->sched, ctx->width, ctx->height, &ctx->grain);

}

static void render_update(void* data, void* context)
{
    struct render_ctx* ctx = (struct render_ctx*)context;

    mdb_kernel_set_surface(ctx->kernel, (float*)data);

    rsched_yield(ctx->sched, RSCHED_ROOT);

    rsched_requeue(ctx->sched);
}

static void render_kernel_proc_fun(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, void* ctx)
{
    struct render_ctx* rend_ctx = (struct render_ctx*)ctx;

    mdb_kernel_process_block(rend_ctx->kernel, x0, x1, y0, y1);
}

int render_run(rsched* sched, mdb_kernel* kernel, uint32_t width, uint32_t height)
{
    struct render_ctx ctx;

    ctx.bailout = 256;
    ctx.scale   = 2.793042f;
    ctx.shift_x = -0.860787f;
    ctx.shift_y = 0.0f;
    ctx.width   = width;
    ctx.height  = height;

    ctx.sched = sched;
    ctx.kernel = kernel;

    rsched_set_proc_fun(sched, &render_kernel_proc_fun, &ctx);

    LOG_SAY("Starting render mode...\nControl keys:\n%s\n", render_control_keys);

#if defined(OGL_RENDER_ENABLED)
    ogl_render* rend;
    ogl_render_create(&rend, ctx.width, ctx.height, &render_update, &ctx, &render_key_callback);
    ogl_render_set_resize_callback(rend, &render_resize);


    ogl_render_render_loop(rend);

    ogl_render_destroy(rend);
#else
    UNUSED_PARAM(render_control_keys);
    LOG_ERROR("OGL Render disabled at the build time.");
    return -1;
#endif

    return 0;
}

