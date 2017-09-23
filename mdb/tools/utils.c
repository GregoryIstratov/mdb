#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "utils.h"

int file_read_all(const char* filename, size_t* size, void** data)
{
    FILE* in = fopen(filename, "rb");
    if (in == NULL)
    {
        LOG_ERROR("Failed to open the file '%s'", filename);
        return errno;
    }

    fseek(in, 0, SEEK_END);

    *size = (size_t)ftell(in);

    rewind(in);

    *data = calloc(1, *size);

    if((*data) == NULL)
    {
        LOG_ERROR("[file_read_all] calloc failed");
        return errno;
    }

    size_t read = fread(*data, 1, *size, in);

    if (read != *size)
    {
        LOG_ERROR("[file_read_all] fread has read not all bytes in stream read = %lu != total = %lu", read, (*size));
        return -1;
    }

    if (ferror(in))
    {
        LOG_ERROR("[file_read_all] fread error");
        return errno;
    }

    fclose(in);

    return 0;
}