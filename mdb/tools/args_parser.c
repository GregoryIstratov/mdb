/*
   We now use the first four fields in ARGP, so here’s a description of them:
     OPTIONS  – A pointer to a vector of struct argp_option (see below)
     PARSER   – A function to parse a single option, called by argp
     ARGS_DOC – A string describing how the non-option arguments should look
     DOC      – A descriptive string about this program; if it contains a
                 vertical tab character (\v), the part after it will be
                 printed *following* the options

   The function PARSER takes the following arguments:
     KEY  – An integer specifying which option this is (taken
             from the KEY field in each struct argp_option), or
             a special key specifying something else; the only
             special keys we use here are ARGP_KEY_ARG, meaning
             a non-option argument, and ARGP_KEY_END, meaning
             that all arguments have been parsed
     ARG  – For an option KEY, the string value of its
             argument, or NULL if it has none
     STATE– A pointer to a struct argp_state, containing
             various useful information about the parsing state; used here
             are the INPUT field, which reflects the INPUT argument to
             argp_parse, and the ARG_NUM field, which is the number of the
             current non-option argument being parsed
   It should return either 0, meaning success, ARGP_ERR_UNKNOWN, meaning the
   given KEY wasn’t recognized, or an errno value indicating some other
   error.

   Note that in this example, main uses a structure to communicate with the
   parse_opt function, a pointer to which it passes in the INPUT argument to
   argp_parse.  Of course, it’s also possible to use global variables
   instead, but this is somewhat more flexible.

   The OPTIONS field contains a pointer to a vector of struct argp_option’s;
   that structure has the following fields (if you assign your option
   structures using array initialization like this example, unspecified
   fields will be defaulted to 0, and need not be specified):
     NAME   – The name of this option’s long option (may be zero)
     KEY    – The KEY to pass to the PARSER function when parsing this option,
               *and* the name of this option’s short option, if it is a
               printable ascii character
     ARG    – The name of this option’s argument, if any
     FLAGS  – Flags describing this option; some of them are:
                 OPTION_ARG_OPTIONAL – The argument to this option is optional
                 OPTION_ALIAS        – This option is an alias for the
                                        previous option
                 OPTION_HIDDEN       – Don’t show this option in –help output
     DOC    – A documentation string for this option, shown in –help output

   An options vector should be terminated by an option with all fields zero. */

#include "args_parser.h"
#include "compiler.h"
#include "timer.h"

#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <dirent.h>

const char* argp_program_version = "mdb 1.0";
const char* argp_program_bug_address = "https://github.com/GregoryIstratov/mdb";

/* Program documentation. */
static char doc[] = "";

/* A description of the arguments we accept. */
static char args_doc[] = "";

enum
{
        GR_INHERIT = 0,
        GR_CORE,
        GR_MD_ONESHOT,
        GR_MD_BENCHMARK,
        GR_EXTRA
};

enum
{
        KEY_MODE = 0xFF00,
        KEY_BENCH_RUNS,
        KEY_COLORS,
        KEY_RSCHED,
        KEY_KRN_LIST
};

#define OPTION_EX(name, key, arg, flags, doc, group) \
        {(name), (key), (arg), (flags), (doc), (group) },

#define OPTION(name, key, arg, doc) \
        OPTION_EX((name), (key), (arg), 0, (doc), GR_INHERIT)

#define rsched_opt_doc \
        "\nAll options are separated by a comma.\n" \
        "{key},{options}\n" \
        "Key - profile. Options:\n" \
        "hist_{run|task|payload}\n" \
        "hist options:\n" \
        "show=[1|0] - Show histogram.\n" \
        "\t\t\t\tsize=[N] - Histogram size.\n" \
        "\t\t\t\tmin=[TIMEVAL] - Minimum sampling time.\n" \
        "\t\t\t\tmax=[TIMEVAL] - Maximum sampling time.\n" \
        "\t\t\t\tlog_scale=[1|0] - Histogram logarithmic scale.\n" \
        "TIMEVAL - is a time in a format like '10[s|ms|mc|ns]' where:\n" \
        "s - seconds.\n" \
        "ms - milliseconds.\n" \
        "mc - microseconds.\n" \
        "ns - nanoseconds.\n"

/* The options we understand. */
static const
struct argp_option options[] = {
OPTION_EX(0, 0, 0, 0, 0, GR_CORE)
OPTION("width", 'w', "SIZE", "Surface width in pixels")
OPTION("height",'h', "SIZE", "Surface height in pixels")
OPTION("quad", 'x', "SIZE", "Surface NxN in pixels | default: 1024")
OPTION("bailout", 'i', "N", "Bailout / Max iteration depth | default: 256")
OPTION("block-size", 'b', "NxM", "Computation block size | default: 32x32")
OPTION("kernel",'k', "NAME", "Name of a kernel to load.\n"
                       "You can check available list by typing --kernel-list. "
                       "default: generic.")

OPTION("kernel-list", KEY_KRN_LIST, 0, "List available kernels.")

OPTION("threads",'t', "n|auto", "Number of processing threads.\n"
                                "auto - determines count of hardware threads.\t"
                                "default: auto")

OPTION("mode", KEY_MODE, "MODE",
                       "oneshot - Renders one hdr image to --output\t"
                       "benchmark - Suitable for performance measurement\t"
                       "render - Real-time screen render, requires OpenGL\t"
                       "default: oneshot")

OPTION("rsched", KEY_RSCHED, "OPTIONS", rsched_opt_doc)

OPTION("colors", KEY_COLORS, "on|off",
       "Enable coloring by OpenGL shader | default: on")

OPTION_EX(0, 0, 0, 0, "Mode oneshot params:", GR_MD_ONESHOT)
OPTION("output", 'o', "FILE",
       "Output to FILE with HDR format | default: mandelbrot.hdr")

OPTION_EX(0, 0, 0, 0, "Mode benchmark params:", GR_MD_BENCHMARK)
OPTION("benchmark-runs", KEY_BENCH_RUNS,  "N"   ,
       "Number of iterations in benchmark | default: 100")

OPTION_EX(0, 0, 0, 0, "Extra params:", GR_EXTRA)
OPTION("verbose", 'v', 0, "Produce verbose output")
OPTION("quiet", 'q', 0, "Don't produce any output")

OPTION_EX(0, 0, 0, 0, 0, 0)
};

#undef LOG_DEBUG
#define LOG_DEBUG(fmt, ...) \
        if(IS_ENABLED(CONFIG_ARG_PARSER_DEBUG)) \
                _log_printf(stdout, false, __func__, (fmt), ##__VA_ARGS__)

#undef LOG_ERROR
#define LOG_ERROR(fmt, ...) \
                _log_printf(stderr, true, __func__, (fmt), ##__VA_ARGS__)


static inline
void _log_printf(FILE* sink, bool is_err, const char* fun,
                 const char* fmt, ...)
{
        char* buf;
        const char* type;
        va_list args;

        type = is_err ? "ERROR" : "DEBUG";

        va_start(args, fmt);
        vasprintf(&buf, fmt, args);
        va_end(args);

        fprintf(sink, "[%s][arg_parser][%s]: %s\n", type, fun, buf);

        free(buf);
}


static
int parse_int(const char* key, const char* val, int min_allowed,
              int max_allowed)
{
        char* pend = NULL;
        int i;

        errno = 0;
        i = (int)strtol(val, &pend, 10);

        if(pend != NULL && *pend != '\0')
        {
                fprintf(stderr, "Failed to parse option as int'--%s=%s' : "
                        "doesn't look like a number\n", key, val);
                exit(EXIT_FAILURE);
        }

        if(errno != 0)
        {
                fprintf(stderr, "Failed to parse option as int '--%s=%s' : "
                        "%s \n", key, val, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if(i < min_allowed || i > max_allowed)
        {
                fprintf(stderr, "Failed to parse option as int '--%s=%s' : "
                        "value is out of range [%i-%i]\n",
                        key, val, min_allowed, max_allowed);
                exit(EXIT_FAILURE);
        }


        return i;
}

static
int parse_u64_next(const char* val, uint64_t* res, char** next)
{
        char* pend = NULL;
        uint64_t i;

        errno = 0;

#if __WORDSIZE == 64
        i = strtoul(val, &pend, 10);

#else
        i = strtoull(val, &pend, 10);
#endif

        if(errno != 0)
        {
                LOG_ERROR("Failed to parse number as u64 in string '%s': "
                          " %s \n", val, strerror(errno));

                return -1;
        }

        if(!next)
        {
                if(pend != NULL && *pend != '\0')
                        return -1;
        }


        *res = i;
        *next = pend;
        return 0;
}

static
int parse_int_c(const char* key, const char* val, char** pend,
                int min_allowed, int max_allowed)
{
        int i;

        errno = 0;
        i = (int)strtol(val, pend, 10);

        if(errno != 0)
        {
                fprintf(stderr, "Failed to parse option as int '--%s=%s' : "
                        "%s \n", key, val, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if(i < min_allowed || i > max_allowed)
        {
                fprintf(stderr, "Failed to parse option as int '--%s=%s' : "
                        "value is out of range [%i-%i]\n",
                        key, val, min_allowed, max_allowed);
                exit(EXIT_FAILURE);
        }


        return i;
}

static
void parse_block_size(const char* val, uint32_t* x,
                      uint32_t* y)
{
        char* pend = NULL;
        errno = 0;

        *x = (uint32_t)parse_int_c("block-size", val, &pend, 8, UINT16_MAX);

        if(pend == NULL || (pend != NULL && *pend == '\0'))
        {
                *y = *x;
        }
        else if(pend != NULL && *pend == 'x')
        {
                *y = (uint32_t) parse_int("block-size", pend + 1, 8, UINT16_MAX);
        }
        else
        {
                fprintf(stderr, "Failed to parse option '--block-size=%s' : "
                        "Unknown format\n", val);
                exit(EXIT_FAILURE);
        }
}

static
int parse_threads(char* arg)
{
        if(strcmp(arg, "auto") == 0)
        {
                return -1;
        }
        else
        {
                return parse_int("threads", arg, 1, INT_MAX);
        }
}

static
int parse_mode(char* arg)
{
        if(strcmp(arg, "oneshot") == 0)
        {
                return MODE_ONESHOT;
        }
        else if(strcmp(arg, "benchmark") == 0)
        {
                return MODE_BENCHMARK;
        }
        else if(strcmp(arg, "render") == 0)
        {
                return MODE_RENDER;
        }
        else
        {
                fprintf(stderr, "Unknown value for --mode=%s\n", arg);
                exit(EXIT_FAILURE);
        }
}

static
int parse_on_off(char* key, char* arg)
{
        if(strcmp(arg, "on") == 0)
        {
                return 1;
        }
        else if(strcmp(arg, "off") == 0)
        {
                return 0;
        }
        else
        {
                LOG_ERROR("Unknown value for --'%s'=%s allowed on|off\n",
                          key, arg);
                exit(EXIT_FAILURE);
        }
}


static
int parse_next_comma_opt(char* arg, char** next)
{
        char* found = strchr(arg, ',');

        if(found)
        {
                *next = found + 1;
                return 0;
        }

        return -1;
}

static
bool is_sub_opt(const char* opt, char* arg, char** next)
{
        int cmp;
        size_t opt_len, arg_len;

        opt_len = strlen(opt);
        arg_len = strlen(arg);

        if(arg_len < opt_len + (next? 1 : 0))
                return false;


        cmp = strncmp(opt, arg, opt_len);
        if(cmp == 0)
        {
                if(next)
                        *next = arg + opt_len + 1;

                return true;
        }


        return false;
}

enum
{
        PROFILE_RUN_HIST,
        PROFILE_TASK_HIST,

        HIST_LOG_SCALE,
        HIST_MIN,
        HIST_MAX,



        PROFILE_LAST
};



static
uint64_t parse_convert_time_postfix(ulong val, const char* postfix)
{
        if(strcmp("s", postfix) == 0)
                return val * NS_IN_SEC;

        if(strcmp("mc", postfix) == 0)
                return val * NS_IN_MCS;

        if(strcmp("ms", postfix) == 0)
                return val * NS_IN_MS;

        if(strcmp("ns", postfix) == 0)
                return val;


        LOG_ERROR( "Unknown time postfix: %s\n", postfix);
        exit(EXIT_FAILURE);
}

static
uint64_t parse_time_value(char* arg)
{
        char* postfix;
        uint64_t val;
        int ret;

        ret = parse_u64_next(arg, &val, &postfix);

        if(ret != 0)
                exit(EXIT_FAILURE);


        if(postfix != NULL && *postfix != '\0')
        {
                val = parse_convert_time_postfix(val, postfix);
        }
        else
        {
                LOG_ERROR( "Failed to parse time postfix: %s\n", arg);
                exit(EXIT_FAILURE);
        }

        return val;
}

static
int parse_sub_key_val(char* arg,
                             char* key,
                             char* val,
                             char** next)
{
        char* sep;
        char* val_start;
        char* end;
        size_t key_len, val_len;

        sep = strchr(arg, '=');
        if(!sep)
        {
                LOG_ERROR( "Failed to parse key=value: %s\n", arg);
                return -1;
        }

        end = strchr(sep, ',');

        if(!end)
        {
                end = strchr(sep, '\0');
                if(next) *next = end;
        }
        else
        {
                if(next) *next = end + 1;
        }

        key_len = sep - arg;
        val_start = sep + 1;
        val_len = end - val_start;

        LOG_DEBUG("arg '%s' sep '%s' val_start '%s' end '%s' key_len=%i, val_len=%i",
                  arg, sep, val_start, end, key_len, val_len);

        memcpy(key, arg, key_len);
        key[key_len] = '\0';

        memcpy(val, val_start, val_len);
        val[val_len] = '\0';

        return 0;
}


static
void parse_hist_opts(char* arg, struct arg_rsched_hist* hist)
{
        static const char* show_lab = "show";
        static const char* log_scale_lab = "log_scale";
        static const char* min_lab = "min";
        static const char* max_lab = "max";
        static const char* size_lab = "size";

        char* next_opt;
        char key[64];
        char val[64];
        int ret;

        LOG_DEBUG("arg '%s'", arg);

        next_opt = arg;
        while(next_opt && *next_opt != '\0')
        {
                ret = parse_sub_key_val(next_opt, key, val, &next_opt);

                if(ret != 0) exit(EXIT_FAILURE);

                LOG_DEBUG("key '%s' val '%s'", key, val);

                if(strcmp(show_lab, key) == 0)
                {
                        int i = parse_int(show_lab, val, 0, 1);
                        optional_set(&hist->show, i ? true : false);
                }

                if(strcmp(size_lab, key) == 0)
                {
                        int i = parse_int(size_lab, val, 8, 1024);
                        optional_set(&hist->size, i);
                }

                if(strcmp(log_scale_lab, key) == 0)
                {
                        int i = parse_int(log_scale_lab, val, 0, 1);
                        optional_set(&hist->log_scale, i ? true : false);
                }

                if(strcmp(min_lab, key) == 0)
                {
                        uint64_t time = parse_time_value(val);
                        optional_set(&hist->min, time);
                }

                if(strcmp(max_lab, key) == 0)
                {
                        uint64_t time = parse_time_value(val);
                        optional_set(&hist->max, time);
                }
        }
}

static
void parse_profile_hist(char* arg, struct arg_rsched_hist* hist)
{
        parse_hist_opts(arg, hist);
}

static
int parse_rsched_profile(char* arg, struct arg_rsched* rsched)
{
        char* next_opt;

        LOG_DEBUG( "opt: %s\n", arg);


        if(is_sub_opt("run_hist", arg, &next_opt))
        {
                parse_profile_hist(next_opt, &rsched->run_hist);

                LOG_DEBUG("run_hist: show=%d log_scale=%d min=%lu max=%lu",
                          rsched->run_hist.show,
                          rsched->run_hist.log_scale,
                          rsched->run_hist.min,
                          rsched->run_hist.max);

                return 0;
        }

        if(is_sub_opt("task_hist", arg, &next_opt))
        {
                parse_profile_hist(next_opt, &rsched->task_hist);

                LOG_DEBUG("task_hist: show=%d log_scale=%d min=%lu max=%lu",
                          rsched->task_hist.show,
                          rsched->task_hist.log_scale,
                          rsched->task_hist.min,
                          rsched->task_hist.max);

                return 0;
        }

        if(is_sub_opt("payload_hist", arg, &next_opt))
        {
                parse_profile_hist(next_opt, &rsched->payload_hist);

                LOG_DEBUG("payload_hist: show=%d log_scale=%d min=%lu max=%lu",
                          rsched->payload_hist.show,
                          rsched->payload_hist.log_scale,
                          rsched->payload_hist.min,
                          rsched->payload_hist.max);

                return 0;
        }

        LOG_ERROR("Unknown option '%s'\n", arg);

        return -1;
}


static
int parse_rsched(char* arg, struct arg_rsched* rsched)
{
        char* opt_arg;

        LOG_DEBUG("rsched opt: %s\n", arg);


        if(is_sub_opt("profile", arg, &opt_arg))
        {
                if(parse_rsched_profile(opt_arg, rsched) != 0)
                        exit(EXIT_FAILURE);
        }
        else
        {
                LOG_ERROR( "Unknown value for --rsched\n");
                exit(EXIT_FAILURE);
        }

        return 0;
}

static
void list_kernels()
{
        DIR* dir;
        struct dirent* ent;

        dir = opendir("./modules/kernels/");
        if(dir)
        {
                while((ent = readdir(dir)) != NULL)
                {
                        if(strcmp(".", ent->d_name) == 0)
                                continue;

                        if(strcmp("..", ent->d_name) == 0)
                                continue;

                        printf("%s\n", ent->d_name);
                }
                closedir(dir);
        }
        else
        {
                perror("[list_kernels] opendir");
                exit(EXIT_FAILURE);
        }
}


/* Parse a single option. */
static
error_t parse_opt(int key, char* arg, struct argp_state* state)
{
        /* Get the input argument from argp_parse, which we
           know is a pointer to our arguments structure. */
        struct arguments* arguments = state->input;

        switch (key)
        {
case 'w':
        arguments->width = (uint32_t)parse_int("width", arg, 512, UINT16_MAX);
        break;
case 'h':
        arguments->height = (uint32_t)parse_int("height", arg, 512, UINT16_MAX);
        break;
case 'x':
{
        arguments->width = (uint32_t)parse_int("quad", arg, 512, UINT16_MAX);
        arguments->height = arguments->width;
        break;
}
case 'i':
        arguments->bailout = (uint32_t)parse_int("bailout", arg, 1, UINT16_MAX);
        break;

case 'b':
        parse_block_size(arg, &arguments->block_size_x,
                         &arguments->block_size_y);
        break;

case 'k':
        arguments->kernel_name = arg;
        break;

case 't':
        arguments->threads = parse_threads(arg);
        break;

case KEY_MODE:
        arguments->mode = parse_mode(arg);
        break;

case KEY_BENCH_RUNS:
        arguments->benchmark_runs = parse_int("benchmark-runs", arg, 1, INT_MAX);
        break;

case KEY_COLORS:
        arguments->shader_colors = parse_on_off("colors", arg);
        break;

case 'q':
case 's':
        arguments->silent = 1;
        break;
case 'v':
        arguments->verbose = parse_int("verbose", arg, 0, 5);
        break;
case 'o':
        arguments->output_file = arg;
        break;

case KEY_RSCHED:
#ifdef CONFIG_RSCHED_PROFILE
        parse_rsched(arg, &arguments->rsched);
#endif
        break;

case KEY_KRN_LIST:
        list_kernels();
        exit(0);

default:
        return ARGP_ERR_UNKNOWN;
        }
        return 0;
}

/* Our argp parser. */
static const struct argp argp = {
        .options = options,
        .parser = parse_opt,
        .args_doc = args_doc,
        .doc = doc
};

void args_parse(int argc, char** argv, struct arguments* arguments)
{
        memset(arguments, 0, sizeof(struct arguments));

        /* Default values. */
        arguments->width         = 1024;
        arguments->height        = 1024;
        arguments->bailout       = 256;
        arguments->block_size_x  = 32;
        arguments->block_size_y  = 32;
        arguments->kernel_name   = "mdb_generic";
        arguments->threads       = -1;
        arguments->mode          = MODE_ONESHOT;
        arguments->output_file   = "mandelbrot.hdr";
        arguments->benchmark_runs= 100;
        arguments->shader_colors = 1;

        /* Parse our arguments; every option seen by parse_opt will
           be reflected in arguments. */
        argp_parse(&argp, argc, argv, 0, 0, arguments);

        //debug_arguments(arguments);
}