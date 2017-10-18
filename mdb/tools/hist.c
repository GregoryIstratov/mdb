#include "hist.h"

#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "timer.h"
#include "log.h"

#include <inttypes.h>

static inline
size_t HIST_BOUNDL(const struct perf_hist* hist)
{
        return hist->size;
}

static inline
size_t HIST_BOUNDR(const struct perf_hist* hist)
{
        return hist->size + 1;
}

void perf_hist_init(struct perf_hist* hist, uint32_t size,
                    uint64_t min, uint64_t max, bool log_scale)
{
        size_t i;
        uint64_t width;
        uint64_t bin_min, bin_max;

        width = max - min;
        bin_min = min;

        hist->bin = calloc(size + 2, sizeof(*hist->bin));
        hist->size = size;

        hist->bin[HIST_BOUNDL(hist)].min = min;
        hist->bin[HIST_BOUNDL(hist)].max = min;

        hist->bin[HIST_BOUNDR(hist)].min = max;
        hist->bin[HIST_BOUNDR(hist)].max = max;

        for (i = 0; i < size; ++i)
        {
                if (log_scale)
                {
                        double s = (double) (i + 1) / size;
                        double f = log(width);
                        double fi = exp(f * s);

                        bin_max = min + (uint64_t) fi;
                }
                else
                {
                        bin_max = min + (width / size * (i + 1));
                }

                hist->bin[i].min = bin_min;
                hist->bin[i].max = bin_max;

                bin_min = bin_max;
        }
}

void perf_hist_destroy(struct perf_hist* hist)
{
        free(hist->bin);
}

static inline
int perf_hist_check_range(const struct perf_hist_bin* bin,
                          uint64_t val)
{
        if(val < bin->min)
                return -1;
        else if(val >= bin->max)
                return 1;
        else
                return 0;
}

static inline
int perf_hist_bsearch(const struct perf_hist* hist, uint64_t val)
{
        const struct perf_hist_bin* bin;
        ssize_t m, l, r;
        int cmp;

        l = 0;
        r = hist->size - 1;

        while(l <= r)
        {
                m = l + (r - l) / 2;


                bin = &hist->bin[m];

                cmp = perf_hist_check_range(bin, val);

                if (cmp == 0)
                {
                        return (int)m;
                }
                else if(cmp == 1)
                {
                        l = m + 1;
                }
                else
                {
                        r = m - 1;
                }

        }

        /* last 2 bins are reserved for values
         * don't fit to the specified range
         * */
        return l == 0 ? HIST_BOUNDL(hist) : HIST_BOUNDR(hist);
}

void perf_hist_add(struct perf_hist* hist, uint64_t val)
{
        int idx = perf_hist_bsearch(hist, val);


        ++hist->bin[idx].data;

}

void perf_hist_print(struct perf_hist* hist)
{
        uint64_t i;
        char tm0[16];
        char tm1[16];
        uint64_t v;

        perf_format_time(hist->bin[HIST_BOUNDL(hist)].min, tm0, 16);
        v = hist->bin[HIST_BOUNDL(hist)].data;

        LOG_SAY("[**][xxxx xx < %s]: %" PRIu64, tm0, v);

        for(i = 0; i < hist->size; ++i)
        {
                perf_format_time(hist->bin[i].min, tm0, 16);
                perf_format_time(hist->bin[i].max, tm1, 16);

                v = hist->bin[i].data;

                LOG_SAY("[%02" PRIu64 "][%s - %s]: %" PRIu64, i, tm0, tm1, v);
        }

        perf_format_time(hist->bin[HIST_BOUNDR(hist)].min, tm0, 16);
        v = hist->bin[HIST_BOUNDR(hist)].data;

        LOG_SAY("[**][xxxx xx > %s]: %" PRIu64, tm0, v);
}
