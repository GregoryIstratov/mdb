#include "mdb_kernel_common.h"

static uint32_t bailout_get_mod(uint32_t bailout)
{
    if(bailout == 1)
        return 0;

    if (bailout < 256)
        return 1;
    if (bailout < 512)
        return 2;
    if (bailout < 1024)
        return 4;
    if (bailout < 2048)
        return 8;
    if (bailout < 4096)
        return 32;
    if (bailout < 8192)
        return 128;
    if (bailout < 16384)
        return 256;

    return 512;
}

static int event_keyboard(struct mdb_event_keyboard* event)
{
    if (event->action == MDB_ACTION_PRESS || event->action == MDB_ACTION_REPEAT)
    {
        switch (event->key)
        {
            case MDB_KEY_1:
            {
                mdb.scale *= 1.1;
                break;
            }
            case MDB_KEY_2:
            {
                mdb.scale *= 0.9;
                break;
            }
            case MDB_KEY_3:
            {
                mdb.bailout -= bailout_get_mod(mdb.bailout);
                break;
            }
            case MDB_KEY_4:
            {
                mdb.bailout += bailout_get_mod(mdb.bailout);
                break;
            }
            case MDB_KEY_RIGHT:
            {
                mdb.shift_x += 0.1 * mdb.scale;
                break;
            }
            case MDB_KEY_LEFT:
            {
                mdb.shift_x -= 0.1 * mdb.scale;
                break;
            }
            case MDB_KEY_UP:
            {
                mdb.shift_y += 0.1 * mdb.scale;
                break;
            }
            case MDB_KEY_DOWN:
            {
                mdb.shift_y -= 0.1 * mdb.scale;
                break;
            }
            case MDB_KEY_F1:
            {
                mdb.scale = 2.793042f;
                mdb.shift_x = -0.860787f;
                mdb.shift_y = 0.0f;
                break;
            }
            case MDB_KEY_F2:
            {
                mdb.scale = 0.00188964f;
                mdb.shift_x = -1.347385054652062f;
                mdb.shift_y = -0.063483549665202f;
                break;
            }
            case MDB_KEY_F3:
            {
                mdb.shift_x = -0.715882f;
                mdb.shift_y = -0.287651f;
                mdb.scale = 0.057683f;
                break;
            }
            case MDB_KEY_F4:
            {
                mdb.shift_x = 0.356868f;
                mdb.shift_y = -0.348140f;
                mdb.scale = 0.003869f;
                break;
            }

            default:
                return MDB_FAIL;
        }

    }

    return MDB_SUCCESS;
}

int mdb_kernel_init(void)
{
    /* View */

    switch(2)
    {
        case 1:
            mdb.bailout = 1;
            mdb.scale   = 2.793042f;
            mdb.shift_x = -0.860787f;
            mdb.shift_y = 0.0f;
            break;

        default:
        case 2:
            mdb.bailout = 256;
            mdb.scale   = 0.00188964f;
            mdb.shift_x = -1.347385054652062f;
            mdb.shift_y = -0.063483549665202f;
            break;

    }

    KPARAM_INFO("[KRN] BAILOUT", "%d", mdb.bailout);
    //KPARAM_INFO("[KRN] SCALE ")

    return MDB_SUCCESS;
}


int mdb_kernel_shutdown(void)
{
    return MDB_SUCCESS;
}

int mdb_kernel_event_handler(int type, void* event)
{
    switch(type)
    {
        case MDB_EVENT_KEYBOARD:
            return event_keyboard((struct mdb_event_keyboard*)event);

        default:
            return MDB_FAIL;
    }

    return MDB_SUCCESS;
}


int mdb_kernel_metadata_query(int query, char* buff, uint32_t buff_size)
{
    switch(query)
    {
        case MDB_KERNEL_META_NAME:
            return metadata_copy(GLOBAL_VAR(name), buff, buff_size);

        case MDB_KERNEL_META_VER_MAJ:
            return metadata_copy(GLOBAL_VAR(ver_maj), buff, buff_size);

        case MDB_KERNEL_META_VER_MIN:
            return metadata_copy(GLOBAL_VAR(ver_min), buff, buff_size);

        default:
            return MDB_QUERY_NO_PARAM;
    }
}

int mdb_kernel_set_size(uint32_t width, uint32_t height)
{
    mdb.width = width;
    mdb.height = height;
    mdb.width_r = 1.0f / width;
    mdb.height_r = 1.0f / height;
    mdb.aspect_ratio = (float)width / height;

    return MDB_SUCCESS;
}


int mdb_kernel_set_surface(surface* surf)
{
    mdb.surf = surf;

    return MDB_SUCCESS;
}
