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

#include <stdlib.h>
#include <argp.h>
#include <string.h>
#include <mdb/kernel/kernel_cpu.h>

const char* argp_program_version =
        "mandelbrot 1.0";
const char* argp_program_bug_address = "";

/* Program documentation. */
static char doc[] =
        "Argp example #3 -- a program with options and arguments using argp";

/* A description of the arguments we accept. */
static char args_doc[] = "ARG1 ARG2";

enum
{
    ARG_GROUP_INHERIT = 0,
    ARG_GROUP_CORE,
    ARG_GROUP_MODE_ONESHOT,
    ARG_GROUP_MODE_BENCHMARK,
    ARG_GROUP_EXTRA
};

enum
{
    ARG_KEY_MODE = 0xFF00,
    ARG_KEY_BENCH_RUNS
};

/* The options we understand. */
static struct argp_option options[] = {
        {0,   0, 0, 0, "Core params:", ARG_GROUP_CORE},
        {"width",       'w', "SIZE", 0, "Surface width in pixels", ARG_GROUP_INHERIT},
        {"height",      'h', "SIZE", 0, "Surface height in pixels", ARG_GROUP_INHERIT},
        {"quad",        'x', "SIZE", 0, "Surface NxN in pixels | default: 1024 ", ARG_GROUP_INHERIT},
        {"bailout",     'i', "N"   , 0, "Bailout / Max iteration depth | default: 256 ", ARG_GROUP_INHERIT},
        {"block-size",  'b', "NxM" , 0, "Computation block size | default: 64x64", ARG_GROUP_INHERIT},
        {"kernel-type", 'k', "native|generic|avx|avx_fma" , 0, "\nThis is the most important option determines which kernel should be used for computation which can significantly increase performance, but your CPU should support those features that you want use:\n"
                               "generic - This Kernel is written in C and compiled to use generic cpu instruction set of your cpu architecture for processing.\n"
                               "native  - This kernel is written in C and only available if you compile this program from the sources with specifying MDB_ENABLE_NATIVE_KERNEL flag "
                               "this allows compiler to determine your cpu and use suitable instruction set like SSE,AVX,FMA, but performance of this kernel depends only on how smart is your compiler and this may benefit not significantly compared to the generic kernel.\n"
                               "avx - This Kernel is written in assembler intrinsics using AVX instruction set to vectorize computation.\n"
                               "avx_fma - This Kernel is written in assembler intrinsics using AVX and FMA3 instruction set to vectorize computation.\n"
                               "default: generic",
                ARG_GROUP_INHERIT},
        {"threads",     't', "n|auto"   , 0, "Processing threads number, auto - determines count of hardware threads | default: auto", ARG_GROUP_INHERIT},
        {"mode",       ARG_KEY_MODE, "oneshot|benchmark|render"   , 0, "Run mode:\n"
                               "oneshot - Renders one hdr image to --output\n"
                               "benchmark - Suitable for performance measurement\n"
                               "render - Real-time render to screen, requires opengl for output\n"
                               "default: oneshot",
                ARG_GROUP_INHERIT},

        {0,   0, 0, 0, "Mode oneshot params:", ARG_GROUP_MODE_ONESHOT},
        {"output",  'o', "FILE", 0, "Output to FILE with HDR format | default: mandelbrot.hdr", ARG_GROUP_INHERIT},
        {0,   0, 0, 0, "Mode benchmark params:", ARG_GROUP_MODE_BENCHMARK},
        {"benchmark-runs", ARG_KEY_BENCH_RUNS,  "N"   , 0, "Number of iterations in benchmark | default: 100", ARG_GROUP_INHERIT},
        {0,   0, 0, 0, "Extra params:", ARG_GROUP_EXTRA},
        {"verbose", 'v', 0,      0, "Produce verbose output", ARG_GROUP_INHERIT},
        {"quiet",   'q', 0,      0, "Don't produce any output", ARG_GROUP_INHERIT},
        {"silent",  's', 0, OPTION_ALIAS, 0, 0},

        {0}
};



static void debug_arguments(struct arguments* args)
{
    fprintf(stdout, "[DEBUG] arguments\n"
                    "width=%i\n"
                    "height=%i\n"
                    "bailout=%i\n"
                    "block_size=%ix%i\n"
                    "kernel_type=%i\n"
                    "threads=%i\n"
                    "mode=%i\n"
                    "benchmark_runs=%i\n"
                    "silent=%i\n"
                    "verbose=%i\n"
                    "output_file=%s\n",
            args->width,
            args->height,
            args->bailout,
            args->block_size.x,
            args->block_size.y,
            args->kernel_type,
            args->threads,
            args->mode,
            args->benchmark_runs,
            args->silent,
            args->verbose,
            args->output_file
    );
}

static int parse_int(const char* key, const char* val, int min_allowed, int max_allowed)
{
    //printf("[parse_int]: %c -> %s\n", key, val);
    errno = 0;
    char* pend = NULL;
    int i = (int)strtol(val, &pend, 10);

    if(pend != NULL && *pend != '\0')
    {
        fprintf(stderr, "Failed to parse option as int'--%s=%s' : doesn't look like a number\n", key, val);
        exit(EXIT_FAILURE);
    }

    if(errno != 0)
    {
        fprintf(stderr, "Failed to parse option as int '--%s=%s' : %s \n", key, val, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(i < min_allowed || i > max_allowed)
    {
        fprintf(stderr, "Failed to parse option as int '--%s=%s' : value is out of range [%i-%i]\n", key, val, min_allowed, max_allowed);
        exit(EXIT_FAILURE);
    }


    return i;
}


static int parse_kernel_type(char* arg)
{
    //fprintf(stdout, "[parse_kernel_type] arg: %s\n", arg);

    if(strcmp(arg, "generic") == 0)
    {
        return MDB_KERNEL_GENERIC;
    }
    else if(strcmp(arg, "native") == 0)
    {
        return MDB_KERNEL_NATIVE;
    }
    else if(strcmp(arg, "avx") == 0)
    {
        return MDB_KERNEL_AVX;
    }
    else if(strcmp(arg, "avx_fma") == 0)
    {
        return MDB_KERNEL_AVX_FMA;
    }
    else
    {
        fprintf(stderr, "Unknown value for --kernel-type=%s\n", arg);
        exit(EXIT_FAILURE);
    }

}

static int parse_threads(char* arg)
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

static int parse_mode(char* arg)
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


/* Parse a single option. */
static error_t parse_opt(int key, char* arg, struct argp_state* state)
{
    /* Get the input argument from argp_parse, which we
       know is a pointer to our arguments structure. */
    struct arguments* arguments = state->input;

    switch (key)
    {
        case 'w':
            arguments->width = parse_int("width", arg, 512, INT_MAX);
            break;
        case 'h':
            arguments->height = parse_int("height", arg, 512, INT_MAX);
            break;
        case 'x':
        {
            arguments->width = parse_int("quad", arg, 512, INT_MAX);
            arguments->height = arguments->width;
            break;
        }
        case 'i':
            arguments->bailout = parse_int("bailout", arg, 1, INT_MAX);
            break;

        case 'b':
        {
            int res = sscanf(arg, "%dx%d", &arguments->block_size.x, &arguments->block_size.y);
            if(res == 1)
            {
                arguments->block_size.y = arguments->block_size.x;
            }
            else if(res == 0)
            {
                fprintf(stderr, "[--block-size] Can't parse the input '%s'", arg);
                exit(EXIT_FAILURE);
            }

            break;
        }

        case 'k':
            arguments->kernel_type = parse_kernel_type(arg);
            break;

        case 't':
            arguments->threads = parse_threads(arg);
            break;

        case ARG_KEY_MODE:
            arguments->mode = parse_mode(arg);
            break;

        case ARG_KEY_BENCH_RUNS:
            arguments->benchmark_runs = parse_int("benchmark-runs", arg, 1, INT_MAX);
            break;

        case 'q':
        case 's':
            arguments->silent = 1;
            break;
        case 'v':
            arguments->verbose = 1;
            break;
        case 'o':
            arguments->output_file = arg;
            break;

//        case ARGP_KEY_ARG:
//            if (state->arg_num >= 2)
//                /* Too many arguments. */
//                argp_usage(state);
//
//            break;
//
//        case ARGP_KEY_END:
//            if (state->arg_num < 2)
//                /* Not enough arguments. */
//                argp_usage(state);
//            break;

        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

/* Our argp parser. */
static struct argp argp = {
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
    arguments->block_size.x  = 64;
    arguments->block_size.y  = 64;
    arguments->kernel_type   = MDB_KERNEL_GENERIC;
    arguments->threads       = -1;
    arguments->mode          = MODE_ONESHOT;
    arguments->output_file   = "mandelbrot.hdr";
    arguments->benchmark_runs= 100;

    /* Parse our arguments; every option seen by parse_opt will
       be reflected in arguments. */
    argp_parse(&argp, argc, argv, 0, 0, arguments);

    //debug_arguments(arguments);
}