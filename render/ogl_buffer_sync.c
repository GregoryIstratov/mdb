#include "ogl_buffer_sync.h"

void ogl_buffer_lock(GLsync* syncObj)
{
        if (*syncObj)
                glDeleteSync(*syncObj);

        *syncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}

void ogl_buffer_wait(GLsync* syncObj)
{
        GLenum wait;

        if (*syncObj)
        {
                while (1)
                {
                        wait = glClientWaitSync(*syncObj,
                                                GL_SYNC_FLUSH_COMMANDS_BIT,
                                                1);

                        if (wait == GL_ALREADY_SIGNALED
                            || wait == GL_CONDITION_SATISFIED)
                                return;
                }
        }
}
