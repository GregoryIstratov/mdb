#pragma once

#include <kernel_module.h>

#define GLOBAL_VAR_DEFINE(type, name) extern type _gvar_##name
#define GLOBAL_VAR_INIT(type, name, value) type _gvar_##name = (value)
#define GLOBAL_VAR(name) _gvar_##name

struct mdb_t
{
        uint32_t bailout;
        uint32_t width;
        uint32_t height;
        float shift_x;
        float shift_y;
        float scale;
        struct surface* surf;
        float width_r;
        float height_r;
        float aspect_ratio;
};

struct mdb_t mdb;

GLOBAL_VAR_DEFINE(const char*, name);
GLOBAL_VAR_DEFINE(const char*, ver_maj);
GLOBAL_VAR_DEFINE(const char*, ver_min);
