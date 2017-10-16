#include "render.h"

#include <mdb/config/config.h>

#if defined(CONFIG_OGL_RENDER)
#include <mdb/render/ogl_render.h>
#endif

#include <stdlib.h>
#include <string.h>


#include <mdb/sched/rsched.h>
#include <mdb/kernel/mdb_kernel.h>
#include <mdb/tools/compiler.h>
#include <mdb/tools/log.h>
#include <mdb/kernel/bits/mdb_kernel_event.h>
#include <mdb/tools/error_codes.h>




static const char* render_control_keys =
        "Arrows  - Move Up/Down/Left/Right\n"
                "[1]/[2] - Scale up/down\n"
                "[3]/[4] - Iterations/bailout increase/decrease\n"
                "[5]/[6] - Exposure increase/decrease\n"
                "[F1-F4] - Change position\n"
;


struct render_ctx
{
        struct rsched* sched;
    mdb_kernel* kernel;
    surface* surf;
    uint32_t width;
    uint32_t height;
    struct block_size grain;
};

static void render_key_callback(void* context, int key, int scancode, int action, int mods)
{
    struct render_ctx* ctx = (struct render_ctx*)context;

    struct mdb_event_keyboard event =
            {
                    .key = key,
                    .scancode = scancode,
                    .action = action,
                    .mods = mods
            };

    mdb_kernel_event(ctx->kernel, MDB_EVENT_KEYBOARD, &event);
}

static void render_resize(int width, int height, void* context)
{
    struct render_ctx* ctx = (struct render_ctx*)context;

    ctx->width  = (uint32_t)width;
    ctx->height = (uint32_t)height;

    mdb_kernel_set_size(ctx->kernel, ctx->width, ctx->height);

    //rsched_queue_resize(ctx->sched, ctx->width * ctx->height / (ctx->grain.x * ctx->grain.y));
    rsched_create_tasks(ctx->sched, ctx->width, ctx->height, &ctx->grain);

}

static void render_update(void* data, void* context)
{
    struct render_ctx* ctx = (struct render_ctx*)context;

    surface_set_buffer(ctx->surf, data);

    if(rsched_host_yield(ctx->sched) != MDB_SUCCESS)
    {
        LOG_ERROR("Scheduler failed to yield.");
        rsched_shutdown(ctx->sched);
        exit(EXIT_FAILURE);
    }

    rsched_requeue(ctx->sched);
}

static void render_kernel_proc_fun(uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1, void* ctx)
{
    struct render_ctx* rend_ctx = (struct render_ctx*)ctx;

    mdb_kernel_process_block(rend_ctx->kernel, x0, x1, y0, y1);
}

#if defined(CONFIG_OGL_RENDER)
static void run_ogl_render(struct render_ctx* ctx, bool color_enabled)
{
    ogl_render* rend;

    ogl_render_create(&rend, "Mdb", ctx->width, ctx->height, ctx);

    ogl_render_init_render_target(rend, &render_update);
    ogl_render_init_screen(rend, color_enabled);

    ogl_render_set_key_callback(rend, &render_key_callback);
    ogl_render_set_resize_callback(rend, &render_resize);

    ogl_render_render_loop(rend);

    ogl_render_destroy(rend);
}
#endif

int render_run(struct rsched* sched, mdb_kernel* kernel, surface* surf, uint32_t width, uint32_t height, bool color_enabled)
{
    struct render_ctx ctx;

    ctx.width   = width;
    ctx.height  = height;

    ctx.sched = sched;
    ctx.kernel = kernel;
    ctx.surf = surf;

        rsched_set_user_context(sched, &render_kernel_proc_fun, &ctx);

    LOG_SAY("Starting render mode...\nControl keys:\n%s\n", render_control_keys);

#if defined(CONFIG_OGL_RENDER)
    run_ogl_render(&ctx, color_enabled);

    return MDB_SUCCESS;
#else
    UNUSED_PARAM(render_control_keys);
    UNUSED_PARAM(color_enabled);

    LOG_ERROR("OGL Render disabled at the build time.");
    return MDB_FAIL;
#endif

}

