#include "mdb_kernel.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <tools/compiler.h>
#include <tools/log.h>
#include <tools/error_codes.h>

#include <tools/cpu_features.h>
#include <stdio.h>


static int mdb_kernel_init(struct mdb_kernel* mdb);

typedef void (*register_log_context_t)(struct log_context* log);

static inline
bool load_sym(void* handle, void** dst, const char* sym)
{
    char* error;

    *dst = dlsym(handle, sym);
    if ((error = dlerror()) != NULL)
    {
        LOG_ERROR("Error on symbol resolving: %s", error);
        dlclose(handle);
        return false;
    }

    return true;
}

static int mdb_kernel_load(struct mdb_kernel* mdb, const char* kernel_name)
{
    char filename[1024];
    void* handle;
    register_log_context_t register_log_context;

    if(!kernel_name)
    {
        LOG_ERROR("Kernel name is not set.");
        return MDB_FAIL;
    }

    snprintf(filename, 1024, "./modules/kernels/%s.kern", kernel_name);

    LOG_SAY("Loading kernel: %s ...", filename);

    handle = dlopen(filename, RTLD_NOW);
    if(!handle)
    {
        LOG_ERROR("Failed to load kernel '%s': %s", kernel_name, dlerror());
        return MDB_FAIL;
    }

    /* Clear any existing error */
    dlerror();

    if(!load_sym(handle, (void**)&mdb->init_fun,
                 "mdb_kernel_init"))
        return MDB_FAIL;

    if(!load_sym(handle, (void**)&mdb->shutdown_fun,
                 "mdb_kernel_shutdown"))
        return MDB_FAIL;


    if(!load_sym(handle, (void**)&mdb->cpu_features_fun,
                 "mdb_kernel_cpu_features"))
        return MDB_FAIL;


    if(!load_sym(handle, (void**)&mdb->metadata_query_fun,
                 "mdb_kernel_metadata_query"))
        return MDB_FAIL;


    if(!load_sym(handle, (void**)&mdb->event_handler_fun,
                 "mdb_kernel_event_handler"))
        return MDB_FAIL;

    if(!load_sym(handle, (void**)&mdb->block_fun,
                 "mdb_kernel_process_block"))
        return MDB_FAIL;

    if(!load_sym(handle, (void**)&mdb->set_size_fun,
                 "mdb_kernel_set_size"))
        return MDB_FAIL;


    if(!load_sym(handle, (void**)&mdb->set_surface_fun,
                 "mdb_kernel_set_surface"))
        return MDB_FAIL;

    if(!load_sym(handle, (void**)&register_log_context,
                 "register_log_context"))
        return MDB_FAIL;

    register_log_context(__log_ctx);

    mdb->dl_handle = handle;
    mdb->state     = MDB_KRN_LOADED;

    LOG_SAY("Kernel '%s' has been successfully loaded.", kernel_name);

    return MDB_SUCCESS;
}

static void mdb_kernel_log_info(struct mdb_kernel* mdb)
{
    char buff[256];
    memset(buff, 0, 256);

    LOG_SAY("== Kernel metadata info ==");

    mdb->metadata_query_fun(MDB_KERNEL_META_NAME, buff, 256);
    PARAM_INFO("Name", "%s", buff);

    mdb->metadata_query_fun(MDB_KERNEL_META_VER_MAJ, buff, 256);
    PARAM_INFO("Version major", "%s", buff);

    mdb->metadata_query_fun(MDB_KERNEL_META_VER_MIN, buff, 256);
    PARAM_INFO("Version minor", "%s", buff);

    LOG_SAY("---------------------------------");
}

static int mdb_kernel_check_cpu_features(struct mdb_kernel* mdb)
{
    char fet_buf[256];
    int mask;
    int features = mdb->cpu_features_fun();

    if(!features)
        return MDB_SUCCESS;


    memset(fet_buf, 0, 256);

    cpu_features_to_str(features, fet_buf, 256);

    LOG_DEBUG("Required cpu features [%s]", fet_buf);

    mask = cpu_check_features(features);
    if(mask)
    {
        cpu_features_to_str(mask, fet_buf, 256);
        LOG_ERROR("Unsupported cpu features [%s]", fet_buf);
        return MDB_FAIL;
    }


    return MDB_SUCCESS;


}

int mdb_kernel_create(struct mdb_kernel** pmdb, const char* kernel_name)
{
        struct mdb_kernel* mdb;

    *pmdb = calloc(1, sizeof(**pmdb));
    mdb = *pmdb;

    if(mdb_kernel_load(mdb, kernel_name) != MDB_SUCCESS)
        goto error_exit;

    mdb_kernel_log_info(mdb);

    if(mdb_kernel_check_cpu_features(mdb) != MDB_SUCCESS)
    {
        mdb_kernel_destroy(mdb);
        goto error_exit;
    }

    if(mdb_kernel_init(mdb) != MDB_SUCCESS)
    {
        LOG_ERROR("Kernel initialization failed.");
        goto error_exit;
    }

    return MDB_SUCCESS;

error_exit:
    mdb_kernel_destroy(*pmdb);
    *pmdb = NULL;
    return MDB_FAIL;
}


static int mdb_kernel_init(struct mdb_kernel* mdb)
{
    int res;
    if((res = mdb->init_fun()) != MDB_SUCCESS)
        return res;

    mdb->state = MDB_KRN_INITED;

    return MDB_SUCCESS;
}

void mdb_kernel_destroy(struct mdb_kernel* mdb)
{
    if(mdb->state >= MDB_KRN_INITED)
        mdb->shutdown_fun();

    if(mdb->state >= MDB_KRN_LOADED)
        dlclose(mdb->dl_handle);

    free(mdb);
}

int mdb_kernel_event(struct mdb_kernel* mdb, int event_type, void* event)
{
    return mdb->event_handler_fun(event_type, event);
}

int mdb_kernel_set_surface(struct mdb_kernel* mdb, struct surface* surf)
{
    return mdb->set_surface_fun(surf);
}

int mdb_kernel_set_size(struct mdb_kernel* mdb, uint32_t width, uint32_t height)
{
    return mdb->set_size_fun(width, height);
}

void mdb_kernel_process_block(struct mdb_kernel* mdb, uint32_t x0, uint32_t x1,
                              uint32_t y0, uint32_t y1)
{
    mdb->block_fun(x0, x1, y0, y1);
}
