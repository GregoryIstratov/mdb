#include "ogl_pixel_buffer.h"
#include "ogl_buffer_sync.h"

#include <stdlib.h>

#include <mdb/tools/log.h>
#include <mdb/tools/compiler.h>


struct _ogl_pixel_buffer
{
        GLuint texture_id;
        GLuint buffer_id;
        uint32_t buff_size;
        uint32_t width;
        uint32_t height;
        ogl_data_update_callback data_update;
        void* update_context;
        void* data;
};

void ogl_pbo_create(ogl_pixel_buffer** ppbo, uint32_t width, uint32_t height,
                    ogl_data_update_callback callback, void* ctx)
{
        GLuint flags;
        ogl_pixel_buffer* pbo;

        *ppbo = calloc(1, sizeof(**ppbo));
        pbo = *ppbo;

        pbo->width = width;
        pbo->height = height;
        pbo->buff_size = width * height * sizeof(float);
        pbo->data_update = callback;
        pbo->update_context = ctx;

        glGenBuffers(1, &pbo->buffer_id);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo->buffer_id);

        flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_PIXEL_UNPACK_BUFFER, pbo->buff_size, 0, flags);

        pbo->data = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
                                     pbo->buff_size, flags);

        glGenTextures(1, &pbo->texture_id);
        glBindTexture(GL_TEXTURE_2D, pbo->texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, pbo->width, pbo->height,
                     0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

void ogl_pbo_update(ogl_pixel_buffer* pbo, GLsync* sync)
{
        ogl_buffer_wait(sync);

        pbo->data_update(pbo->data, pbo->update_context);

        ogl_buffer_lock(sync);

}

void ogl_pbo_bind_texture0(ogl_pixel_buffer* pbo)
{
        glActiveTexture(GL_TEXTURE0);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo->buffer_id);
        glBindTexture(GL_TEXTURE_2D, pbo->texture_id);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pbo->width, pbo->height,
                        GL_RED, GL_FLOAT, NULL);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void ogl_pbo_destroy(ogl_pixel_buffer* pbo)
{
        glDeleteBuffers(1, &pbo->buffer_id);
        glDeleteTextures(1, &pbo->texture_id);

        free(pbo);
}


void ogl_pbo_resize(ogl_pixel_buffer* pbo, GLsync* sync,
                    uint32_t width, uint32_t height)
{
        /*FIXME spurious corruption on resize*/

        GLuint flags;

        LOG_DEBUG("enter");

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        ogl_buffer_wait(sync);

        glDeleteBuffers(1, &pbo->buffer_id);
        glDeleteTextures(1, &pbo->texture_id);

        pbo->width = width;
        pbo->height = height;
        pbo->buff_size = width * height * sizeof(float);

        glGenBuffers(1, &pbo->buffer_id);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo->buffer_id);

        flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        glBufferStorage(GL_PIXEL_UNPACK_BUFFER, pbo->buff_size, 0, flags);


        pbo->data = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0,
                                     pbo->buff_size, flags);

        glGenTextures(1, &pbo->texture_id);
        glBindTexture(GL_TEXTURE_2D, pbo->texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, pbo->width, pbo->height,
                     0, GL_RED, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        ogl_buffer_lock(sync);

        LOG_DEBUG("leave");
}

