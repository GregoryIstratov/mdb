#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <time.h>
#include <locale.h>

#define PRINT_ERR(msg,...) { fprintf(stderr, msg, ##__VA_ARGS__); fprintf(stderr, ": %s\n", strerror(errno)); }
#define PRINT_ERR_EXIT(msg, ... ) { PRINT_ERR(msg, ##__VA_ARGS__); exit(EXIT_FAILURE); }

void print8_ps(const float v[8]);
void save_surface(const void* data, unsigned long size);
void test_timers();
void set_def_loc();
double sample_timer();
void print_clocks(uint64_t begin, uint64_t end);


void print8_ps(const float v[8])
{
    printf("[%f %f %f %f ", v[0],v[1],v[2],v[3]);
    printf("%f %f %f %f]\n", v[4],v[5],v[6],v[7]);
}

void save_surface(const void* data, unsigned long size)
{
    FILE* f = fopen("mandelbrot.raw", "wb");

    if(!f)
    {
        PRINT_ERR_EXIT("[save_surface]: Can't open the file")
    }


    size_t res = fwrite(data, 1, size, f);

    if(res != size)
    {
        PRINT_ERR_EXIT("[save_surface]: fwrite failed");
    }


    fclose(f);
}

void set_def_loc()
{
    setlocale(LC_ALL, "");
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


double sample_timer()
{
    static struct timespec beg, end;
    static int start_stop = 1;

    if(start_stop)
    {
        clock_gettime(CLOCK_MONOTONIC, &beg);
        start_stop = 0;

    }
    else
    {
        clock_gettime(CLOCK_MONOTONIC, &end);
        start_stop = 1;

        double elapsed = get_total_sec(&end) - get_total_sec(&beg);

        printf("Total second: %f\n", elapsed);
        printf("Pixel/Second: %'lu\n", (uint64_t)(1024.*1024./elapsed));
        printf("Frame/Second: %f\n", (1.0/elapsed));

        return elapsed;
    }
}

void print_clocks(uint64_t begin, uint64_t end)
{
    uint64_t elapsed = end - begin;
    printf("Total cycles: %'lu\n", elapsed);
    printf("Cycles/Pixel: %f\n", ((double)elapsed/(1024*1024)));
}