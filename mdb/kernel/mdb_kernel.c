#include "mdb_kernel.h"
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <mdb/tools/compiler.h>
#include <mdb/tools/log.h>

#include <mdb/kernel/bits/mdb_kernel.h>
#include <mdb/kernel/bits/mdb_kernel_error.h>

#include <immintrin.h>
#include <mdb/tools/cpu_features.h>


static void mdb_kernel_init(mdb_kernel* mdb);

static int mdb_kernel_load(mdb_kernel* mdb, const char* kernel_name)
{

#define MDB_KRN_RETURN_IF_ERR()  \
    if ((error = dlerror()) != NULL) \
    { \
        LOG_ERROR("Error on symbol resolving: %s", error); \
        return -1; \
    }

    if(!kernel_name)
    {
        LOG_ERROR("Kernel name is not set.");
        return -1;
    }

    char filename[1024];
    memset(filename, 0, 1024);

    strcpy(filename, "./modules/kernels/");
    strcat(filename, kernel_name);
    strcat(filename, ".so");

    LOG_SAY("Loading kernel: %s ...", filename);

    void* handle;
    char* error;

    handle = dlopen(filename, RTLD_NOW);
    if(!handle)
    {
        LOG_ERROR("Failed to load kernel '%s': %s", kernel_name, dlerror());
        return -1;
    }

    /* Clear any existing error */
    dlerror();

    mdb->init_fun = (mdb_kernel_init_t)dlsym(handle, "mdb_kernel_init");

    MDB_KRN_RETURN_IF_ERR();

    mdb->shutdown_fun = (mdb_kernel_shutdown_t)dlsym(handle, "mdb_kernel_shutdown");

    MDB_KRN_RETURN_IF_ERR();

    mdb->cpu_features_fun = (mdb_kernel_cpu_features_t)dlsym(handle, "mdb_kernel_cpu_features");

    MDB_KRN_RETURN_IF_ERR();

    mdb->metadata_query_fun = (mdb_kernel_metadata_query_t)dlsym(handle, "mdb_kernel_metadata_query");

    MDB_KRN_RETURN_IF_ERR();

    mdb->event_handler_fun = (mdb_kernel_event_handler_t)dlsym(handle, "mdb_kernel_event_handler");

    MDB_KRN_RETURN_IF_ERR();

    mdb->block_fun = (mdb_kernel_process_block_t)dlsym(handle, "mdb_kernel_process_block");

    MDB_KRN_RETURN_IF_ERR();

    mdb->set_size_fun = (mdb_kernel_set_size_t)dlsym(handle, "mdb_kernel_set_size");

    MDB_KRN_RETURN_IF_ERR();

    mdb->set_surface_fun = (mdb_kernel_set_surface_t)dlsym(handle, "mdb_kernel_set_surface");

    MDB_KRN_RETURN_IF_ERR();


#undef MDB_KRN_RETURN_IF_ERR

    mdb->dl_handle = handle;

    LOG_SAY("Kernel '%s' has been successfully loaded.", kernel_name);

    return 0;
}

static void mdb_kernel_log_info(mdb_kernel* mdb)
{
    char buff[256];
    memset(buff, 0, 256);

    LOG_SAY("==External kernel metadata info==");

    mdb->metadata_query_fun(MDB_KERNEL_META_NAME, buff, 256);
    PARAM_INFO("Name", "%s", buff);

    mdb->metadata_query_fun(MDB_KERNEL_META_VER_MAJ, buff, 256);
    PARAM_INFO("Version major", "%s", buff);

    mdb->metadata_query_fun(MDB_KERNEL_META_VER_MIN, buff, 256);
    PARAM_INFO("Version minor", "%s", buff);

    LOG_SAY("---------------------------------");
}

static int mdb_kernel_check_cpu_features(mdb_kernel* mdb)
{
    int features = mdb->cpu_features_fun();

    if(!features)
        return 0;

    char fet_buf[256];
    memset(fet_buf, 0, 256);

    cpu_features_to_str(features, fet_buf, 256);

    LOG_DEBUG("Required cpu features [%s]", fet_buf);

    int mask = cpu_check_features(features);
    if(mask)
    {
        cpu_features_to_str(mask, fet_buf, 256);
        LOG_ERROR("Unsupported cpu features [%s]", fet_buf);
        return -1;
    }


    return 0;


}

int mdb_kernel_create(mdb_kernel** pmdb, const char* kernel_name)
{
    *pmdb = calloc(1, sizeof(mdb_kernel));
    mdb_kernel* mdb = *pmdb;


    if(mdb_kernel_load(mdb, kernel_name) != 0)
        goto error_exit;

    mdb_kernel_log_info(mdb);

    if(mdb_kernel_check_cpu_features(mdb) != 0)
    {
        mdb_kernel_destroy(mdb);
    }

    mdb_kernel_init(mdb);
    mdb_kernel_set_size(mdb, 1024, 1024);

    return 0;

error_exit:
    free(*pmdb);
    *pmdb = NULL;
    return -1;
}


static void mdb_kernel_init(mdb_kernel* mdb)
{
    mdb->init_fun();
}

void mdb_kernel_destroy(mdb_kernel* mdb)
{

    mdb->shutdown_fun();

    if(mdb->dl_handle)
    {
        dlclose(mdb->dl_handle);
        mdb->dl_handle = NULL;
    }


    free(mdb);
}

int mdb_kernel_event(mdb_kernel* mdb, int event_type, void* event)
{
    return mdb->event_handler_fun(event_type, event);
}

void mdb_kernel_set_surface(mdb_kernel* mdb, surface* surf)
{
    mdb->set_surface_fun(surf);
}

void mdb_kernel_set_size(mdb_kernel* mdb, uint32_t width, uint32_t height)
{
    mdb->set_size_fun(width, height);
}

void mdb_kernel_process_block(mdb_kernel* mdb, uint32_t x0, uint32_t x1, uint32_t y0, uint32_t y1)
{
    mdb->block_fun(x0, x1, y0, y1);
}
