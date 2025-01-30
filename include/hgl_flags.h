
/**
 * LICENSE:
 *
 * MIT License
 *
 * Copyright (c) 2023 Henrik A. Glass
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * MIT License
 *
 *
 * ABOUT:
 *
 * hgl_flags.h implements a simple command-line flag/option parsing utility.
 *
 *
 * USAGE:
 *
 * Include hgl_flags.h file like this:
 *
 *     #define HGL_FLAGS_IMPLEMENTATION
 *     #include "hgl_flags.h"
 *
 * The max number of allowed flags is 32 by default. To increase this, simply
 * redefine HGL_FLAGS_MAX_N_FLAGS before including hgl_flags.h.
 *
 * Code example:
 *
 *     bool *a = hgl_flags_add_bool("-a,--alternative-name", "Simple option for turning something on or off", false, 0);
 *     bool *cmd = hgl_flags_add_bool("-c,--gen-compl-cmd", "Generate a completion command on stdout", false, 0);
 *     int64_t *i = hgl_flags_add_i64_range("-i", "Simple mandatory int option", 0, HGL_FLAGS_OPT_MANDATORY, INT_MIN, INT_MAX);
 *     const char **outfile = hgl_flags_add_str("-o,--output", "Output file path", "a.out", 0);
 *
 *     int err = hgl_flags_parse(argc, argv);
 *     if (err != 0) {
 *         printf("Usage: %s [Options]\n", argv[0]);
 *         hgl_flags_print();
 *         return 1;
 *     }
 *
 *     if (*cmd) {
 *         hgl_flags_generate_completion_cmd(stdout, argv[0]);
 *         return 0;
 *     }
 *
 *     printf("User provided i = %d\n", (int) *i);
 *
 * You could use this program together with the `cmd` flag to generate a
 * command for the `completion` utility on Linux:
 *
 *     $ ./example --gen-compl-cmd > example.completion
 *     $ source example.completion
 *     $ ./example <tab><tab>
 *          -a      --alternative-name      -c      --gen-compl-cmd
 *          -i      -o      --output
 *
 *
 * AUTHOR: Henrik A. Glass
 *
 */

#ifndef HGL_FLAGS_H
#define HGL_FLAGS_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef HGL_FLAGS_MAX_N_FLAGS
#define HGL_FLAGS_MAX_N_FLAGS 32
#endif
#ifndef HGL_FLAGS_PRINT_MARGIN
#define HGL_FLAGS_PRINT_MARGIN 32
#endif
#define HGL_FLAGS_OPT_MANDATORY             (1 << 0)

#define HGL_FLAGS_STATUS_PARSED             (1 << 0)
#define HGL_FLAGS_STATUS_RANGE_OVERFLOW     (1 << 1)
#define HGL_FLAGS_STATUS_RANGE_UNDERFLOW    (1 << 2)
#define HGL_FLAGS_STATUS_DEFV_OUTSIDE_RANGE (1 << 3)
#define HGL_FLAGS_STATUS_INVALID_RANGE      (1 << 4)

// These are different on Windows. fucking useless language...
//static_assert(sizeof(long) == 8);
//static_assert(sizeof(unsigned long) == 8);
static_assert(sizeof(double) == 8);

typedef enum
{
    HGL_FLAGS_KIND_BOOL,
    HGL_FLAGS_KIND_I64,
    HGL_FLAGS_KIND_U64,
    HGL_FLAGS_KIND_F64,
    HGL_FLAGS_KIND_STR
} HglFlagKind;

typedef union
{
    bool b;
    uint64_t u64;
    int64_t i64;
    double f64;
    const char *str;
} HglFlagValue;

typedef struct
{
    HglFlagKind kind;
    const char *names;
    const char *desc;
    HglFlagValue default_value; // can fit any flag value type
    HglFlagValue value;         // -- || --
    HglFlagValue range_min;
    HglFlagValue range_max;
    uint32_t opts;
    uint16_t status;
    int16_t parse_order;
} HglFlag;

/**
 * Add a flag of type `bool`.
 */
bool *hgl_flags_add_bool(const char *names, const char *desc, bool default_value, uint32_t opts);

/**
 * Add a flag of type `int64_t`.
 */
int64_t *hgl_flags_add_i64(const char *names, const char *desc, int64_t default_value, uint32_t opts);

/**
 * Add a flag of type `int64_t` with specified range of valid values.
 */
int64_t *hgl_flags_add_i64_range(const char *names, const char *desc, int64_t default_value,
                                 uint32_t opts, int64_t range_min, int64_t range_max);

/**
 * Add a flag of type `uint64_t`.
 */
uint64_t *hgl_flags_add_u64(const char *names, const char *desc, uint64_t default_value, uint32_t opts);

/**
 * Add a flag of type `uint64_t` with specified range of valid values.
 */
uint64_t *hgl_flags_add_u64_range(const char *names, const char *desc, uint64_t default_value,
                                  uint32_t opts, uint64_t range_min, uint64_t range_max);

/**
 * Add a flag of type `double`.
 */
double *hgl_flags_add_f64(const char *names, const char *desc, double default_value, uint32_t opts);

/**
 * Add a flag of type `double` with specified range of valid values.
 */
double *hgl_flags_add_f64_range(const char *names, const char *desc, double default_value,
                                uint32_t opts, double range_min, double range_max);

/**
 * Add a flag of type `const char *`.
 */
const char **hgl_flags_add_str(const char *names, const char *desc, const char *default_value, uint32_t opts);

/**
 * Parses all command line arguments.
 */
int hgl_flags_parse(int argc, char *argv[]);

/**
 * Prints the descriptions for all flags defined through calls to hgl_flags_add_*.
 */
void hgl_flags_print(void);

/**
 * Returns `true` if `opt_value` was parsed from the arguments. Conversely, returns
 * `false` if it simply inherited the default value.
 */
bool hgl_flags_occured_in_args(void *opt_value);

/**
 * Returns `true` if `opt_a` was parsed before `opt_b` from the arguments. If either
 * option retained it's default value (i.e. it was not parse from the arguments) then
 * it's treated as occuring "first". If both options retained their default values
 * then reconsider calling this function.
 */
bool hgl_flags_occured_before(void *opt_a, void *opt_b);

/**
 * Generates a completion cmd for the `completion` command line utility on
 * the given stream.
 */
void hgl_flags_generate_completion_cmd(FILE *stream, const char *program_name);

#endif

#ifdef HGL_FLAGS_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <stddef.h>

#define max(a, b) ((a) > (b)) ? (a) : (b)
#define min(a, b) ((a) < (b)) ? (a) : (b)

#define BOLD   "\033[0;1m"
#define RED    "\033[1;31m"
#define YELLOW "\033[1;33m"
#define NC     "\033[0m"

static HglFlag hgl_flags_[HGL_FLAGS_MAX_N_FLAGS] = {0};
static size_t hgl_n_flags_ = 0;

HglFlag *hgl_flag_create_(HglFlagKind kind, const char *names, const char *desc,
                          HglFlagValue default_value, uint32_t opts,
                          HglFlagValue range_min, HglFlagValue range_max);

HglFlag *hgl_flag_create_(HglFlagKind kind, const char *names, const char *desc,
                          HglFlagValue default_value, uint32_t opts,
                          HglFlagValue range_min, HglFlagValue range_max)
{
    hgl_flags_[hgl_n_flags_++] = (HglFlag) {
        .kind          = kind,
        .names         = names,
        .default_value = default_value,
        .value         = default_value,
        .opts          = opts,
        .desc          = desc,
        .status        = 0,
        .range_min     = range_min,
        .range_max     = range_max,
    };
    return &hgl_flags_[hgl_n_flags_ - 1];
}

bool *hgl_flags_add_bool(const char *names, const char *desc, bool default_value, uint32_t opts)
{
    return (bool *) &hgl_flag_create_(HGL_FLAGS_KIND_BOOL, names, desc, (HglFlagValue){.b = default_value},
                                      opts, (HglFlagValue){.b = false}, (HglFlagValue){.b = true})->value;
}

int64_t *hgl_flags_add_i64(const char *names, const char *desc, int64_t default_value, uint32_t opts)
{
    return (int64_t *) &hgl_flag_create_(HGL_FLAGS_KIND_I64, names, desc, (HglFlagValue) {.i64 = default_value},
                                         opts, (HglFlagValue) {.i64 = LONG_MIN}, (HglFlagValue) {.i64 = LONG_MAX})->value;
}

int64_t *hgl_flags_add_i64_range(const char *names, const char *desc, int64_t default_value,
                                 uint32_t opts, int64_t range_min, int64_t range_max)
{
    bool invalid_range = (range_min > range_max);
    bool defv_inside_range = (default_value < range_min) || (default_value > range_max);
    HglFlag *flag = hgl_flag_create_(HGL_FLAGS_KIND_I64, names, desc, (HglFlagValue) {.i64 = default_value},
                                     opts, (HglFlagValue) {.i64 = range_min}, (HglFlagValue) {.i64 = range_max});
    flag->status |= (invalid_range) ? HGL_FLAGS_STATUS_INVALID_RANGE : 0;
    flag->status |= (defv_inside_range) ? HGL_FLAGS_STATUS_DEFV_OUTSIDE_RANGE : 0;
    return &flag->value.i64;
}

uint64_t *hgl_flags_add_u64(const char *names, const char *desc, uint64_t default_value, uint32_t opts)
{
    return (uint64_t *) &hgl_flag_create_(HGL_FLAGS_KIND_U64, names, desc, (HglFlagValue) {.u64 = default_value},
                                          opts, (HglFlagValue) {.u64 = 0}, (HglFlagValue) {.u64 = ULONG_MAX})->value;
}

uint64_t *hgl_flags_add_u64_range(const char *names, const char *desc, uint64_t default_value,
                                  uint32_t opts, uint64_t range_min, uint64_t range_max)
{
    bool invalid_range = (range_min > range_max);
    bool defv_inside_range = (default_value < range_min) || (default_value > range_max);
    HglFlag *flag = hgl_flag_create_(HGL_FLAGS_KIND_U64, names, desc, (HglFlagValue) {.u64 = default_value},
                                     opts, (HglFlagValue) {.u64 = range_min}, (HglFlagValue) {.u64 = range_max});
    flag->status |= (invalid_range) ? HGL_FLAGS_STATUS_INVALID_RANGE : 0;
    flag->status |= (defv_inside_range) ? HGL_FLAGS_STATUS_DEFV_OUTSIDE_RANGE : 0;
    return &flag->value.u64;
}

double *hgl_flags_add_f64(const char *names, const char *desc, double default_value, uint32_t opts)
{
    return (double *) &hgl_flag_create_(HGL_FLAGS_KIND_F64, names, desc, (HglFlagValue) {.f64 = default_value},
                                        opts, (HglFlagValue) {.f64 = -DBL_MAX}, (HglFlagValue) {.f64 = DBL_MAX})->value;
}

double *hgl_flags_add_f64_range(const char *names, const char *desc, double default_value,
                                uint32_t opts, double range_min, double range_max)
{
    bool invalid_range = (range_min > range_max);
    bool defv_inside_range = (default_value < range_min) || (default_value > range_max);
    HglFlag *flag = hgl_flag_create_(HGL_FLAGS_KIND_F64, names, desc, (HglFlagValue) {.f64 = default_value},
                                     opts, (HglFlagValue) {.f64 = range_min}, (HglFlagValue) {.f64 = range_max});
    flag->status |= (invalid_range) ? HGL_FLAGS_STATUS_INVALID_RANGE : 0;
    flag->status |= (defv_inside_range) ? HGL_FLAGS_STATUS_DEFV_OUTSIDE_RANGE : 0;
    return &flag->value.f64;
}

const char **hgl_flags_add_str(const char *names, const char *desc, const char *default_value, uint32_t opts)
{
    return (const char **) &hgl_flag_create_(HGL_FLAGS_KIND_STR, names, desc, (HglFlagValue) {.str = default_value},
                                             opts, (HglFlagValue) {0}, (HglFlagValue) {0})->value;
}

static inline bool is_delimiting_char_(char c)
{
    return (c == '\n') ||
           (c == '\t') ||
           (c == ' ')  ||
           (c == '\r') ||
           (c == ',')  ||
           (c == '\0');
}

int hgl_flags_parse(int argc, char *argv[])
{
    /* iterate argument list */
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];
        size_t arg_len = strlen(arg);

        /* iterate flags */
        bool match = false;
        for (size_t j = 0; (j < hgl_n_flags_) && !match; j++) {
            /* iterate flag names */
            HglFlag *flag = &hgl_flags_[j];
            const char *names = flag->names;
            const char *name  = names;

            size_t names_len  = strlen(names);
            size_t name_len   = 0;

            while ((name + name_len) < (names + names_len)) {

                /* find next `name` */
                name_len = 0;
                while (!is_delimiting_char_(name[name_len])) name_len++;

                /* compare `name` with arg string */
                if ((name_len == arg_len) && (strncmp(name, arg, name_len) == 0)) {
                    match = true;
                    break;
                }

                /* go to possible next `name` in `names` */
                name += name_len + 1;
                continue;
            }

            if (!match) {
                continue;
            }

            /* if the option takes an argument check that argv[i + 1] exists. */
            char *next_arg = NULL;
            char *end;
            HglFlagKind kind = hgl_flags_[j].kind;
            if ((kind == HGL_FLAGS_KIND_I64) || (kind == HGL_FLAGS_KIND_U64) ||
                (kind == HGL_FLAGS_KIND_F64) || (kind == HGL_FLAGS_KIND_STR)) {
                if (i + 1 >= argc) {
                    fprintf(stderr, BOLD RED "Error:" NC " Option `%s` takes "
                            "an argument. User provided nothing.\n", names);
                    return -1;
                }

                /* read next early */
                next_arg = argv[++i];
            }

            switch (kind) {

                /* parse simple boolean option flag */
                case HGL_FLAGS_KIND_BOOL: {
                    flag->value.b = true;
                } break;

                /* parse i64 flag */
                case HGL_FLAGS_KIND_I64: {
                    int64_t value = strtol(next_arg, &end, 0);

                    /* Check if strtol failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, BOLD RED "Error:" NC " Option `%s` takes "
                                "an int. User provided: %s\n", names, next_arg);
                        return -1;
                    }

                    /* clamp to range */
                    int64_t range_min = flag->range_min.i64;
                    int64_t range_max = flag->range_max.i64;
                    int64_t old_value = value;
                    value = min(max(old_value, range_min), range_max);

                    flag->status |= (value > old_value) ? HGL_FLAGS_STATUS_RANGE_UNDERFLOW : 0;
                    flag->status |= (value < old_value) ? HGL_FLAGS_STATUS_RANGE_OVERFLOW : 0;
                    flag->value.i64 = value;
                } break;

                case HGL_FLAGS_KIND_U64: {
                    uint64_t value = strtoul(next_arg, &end, 0);

                    /* Check if strtol failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, BOLD RED "Error:" NC " Option `%s` takes "
                                "an unsigned int. User provided: %s\n", names, next_arg);
                        return -1;
                    }

                    /* clamp to range */
                    uint64_t range_min = flag->range_min.u64;
                    uint64_t range_max = flag->range_max.u64;
                    uint64_t old_value = value;
                    value = min(max(old_value, range_min), range_max);

                    flag->status |= (value > old_value) ? HGL_FLAGS_STATUS_RANGE_UNDERFLOW : 0;
                    flag->status |= (value < old_value) ? HGL_FLAGS_STATUS_RANGE_OVERFLOW : 0;
                    flag->value.u64 = value;
                } break;

                /* parse float64 flag */
                case HGL_FLAGS_KIND_F64: {
                    double value = strtod(next_arg, &end);

                    /* Check if strtof failed */
                    if((end == next_arg) || (*end != '\0')) {
                        fprintf(stderr, BOLD RED "Error:" NC " Option `%s` takes "
                                "a float. User provided: %s\n", names, next_arg);
                        return -1;
                    }

                    /* clamp to range */
                    double range_min = flag->range_min.f64;
                    double range_max = flag->range_max.f64;
                    double old_value = value;
                    value = min(max(old_value, range_min), range_max);

                    flag->status |= (value > old_value) ? HGL_FLAGS_STATUS_RANGE_UNDERFLOW : 0;
                    flag->status |= (value < old_value) ? HGL_FLAGS_STATUS_RANGE_OVERFLOW : 0;
                    flag->value.f64 = value;
                } break;

                /* parse string flag */
                case HGL_FLAGS_KIND_STR: {
                    flag->value.str = next_arg;
                } break;
            }

            /* mark flag as parsed and assign a parse order number */
            flag->status |= HGL_FLAGS_STATUS_PARSED;
            flag->parse_order = (int16_t) i; // let's hope no one wants to parse 64k options..
        }

        if (!match) {
            fprintf(stderr, BOLD RED "Error:" NC " Unrecognized command-line option: \"%s\"\n", arg);
            return -1;
        }
    }

    int err = 0;

    /* Check for parsing errors and warnings */
    for (size_t i = 0; i < hgl_n_flags_; i++) {
        HglFlag flag = hgl_flags_[i];

        /* Assert that mandatory flag have been parsed */
        if (((flag.opts & HGL_FLAGS_OPT_MANDATORY) != 0) &&
            ((flag.status & HGL_FLAGS_STATUS_PARSED) == 0)) {
            fprintf(stderr, BOLD RED "Error:" NC " Option marked as mandatory not provided: `%s`\n", flag.names);
            err = 1;
        }

        /* Assert that ranged flag has a valid range */
        if (flag.status & HGL_FLAGS_STATUS_INVALID_RANGE) {
            fprintf(stderr, BOLD RED "Error:" NC " Option `%s` has an invalid range. \n", flag.names);
            err = 1;
        }

        /* Assert that default value lies inside valid range */
        if (flag.status & HGL_FLAGS_STATUS_DEFV_OUTSIDE_RANGE) {
            fprintf(stderr, BOLD RED "Error:" NC " Option `%s` has a default value outside of the valid range. \n", flag.names);
            err = 1;
        }

        /* Warn if user provided any out-of-range values */
        if (flag.status & (HGL_FLAGS_STATUS_RANGE_OVERFLOW | HGL_FLAGS_STATUS_RANGE_UNDERFLOW)) {
            fprintf(stderr, BOLD YELLOW "Warning:" NC " Option `%s` was provided with an "
                    "out-of-range value. Value has been clamped to: ", flag.names);
#ifndef _WIN32
            HglFlagValue val = flag.value;
            HglFlagValue rmin = flag.range_min;
            HglFlagValue rmax = flag.range_max;
            switch (flag.kind) {
                case HGL_FLAGS_KIND_I64: {
                    fprintf(stderr, "%ld. Valid range = [%ld, %ld]\n", val.i64, rmin.i64, rmax.i64);
                } break;
                case HGL_FLAGS_KIND_U64: {
                    fprintf(stderr, "%lu. Valid range = [%lu, %lu]\n", val.u64, rmin.u64, rmax.u64);
                } break;
                case HGL_FLAGS_KIND_F64: {
                    fprintf(stderr, "%.8g. Valid range = [%.8g, %.8g]\n", val.f64, rmin.f64, rmax.f64);
                } break;
                default: assert(0 && "Unreachable"); break;
            }
#endif
        }
    }

    return err;
}

void hgl_flags_print()
{
    printf(BOLD "Options:\n" NC);
    for (size_t i = 0; i < hgl_n_flags_; i++) {
        HglFlagKind kind  = hgl_flags_[i].kind;
        const char *names = hgl_flags_[i].names;
        const char *desc  = hgl_flags_[i].desc;
        HglFlagValue defv = hgl_flags_[i].default_value;
        HglFlagValue rmin = hgl_flags_[i].range_min;
        HglFlagValue rmax = hgl_flags_[i].range_max;
        uint32_t opts = hgl_flags_[i].opts;
        switch (kind) {
            case HGL_FLAGS_KIND_BOOL: {
                printf("  %-*s %s (default = %d)", -HGL_FLAGS_PRINT_MARGIN, names, desc, defv.b); break;
            } break;
            case HGL_FLAGS_KIND_I64: {
                printf("  %-*s %s (default = %ld, valid range = [%ld, %ld])",
                       -HGL_FLAGS_PRINT_MARGIN, names, desc, (long)defv.i64, (long)rmin.i64, (long)rmax.i64);
            } break;
            case HGL_FLAGS_KIND_U64: {
                printf("  %-*s %s (default = %lu, valid range = [%lu, %lu])",
                       -HGL_FLAGS_PRINT_MARGIN, names, desc, (unsigned long)defv.u64, (unsigned long)rmin.u64, (unsigned long)rmax.u64);
            } break;
            case HGL_FLAGS_KIND_F64: {
                printf("  %-*s %s (default = %.8g, valid range = [%.8g, %.8g])",
                       -HGL_FLAGS_PRINT_MARGIN, names, desc, defv.f64, rmin.f64, rmax.f64);
            } break;
            case HGL_FLAGS_KIND_STR: {
                printf("  %-*s %s (default = %s)", -HGL_FLAGS_PRINT_MARGIN, names, desc, defv.str); break;
            } break;
        }

        if (opts & HGL_FLAGS_OPT_MANDATORY) {
            printf(" -- MANDATORY");
        }

        printf("\n");
    }
}

bool hgl_flags_occured_in_args(void *opt)
{
    uint8_t *ptr8 = (uint8_t *) opt;
    ptr8 -= offsetof(HglFlag, value);
    HglFlag *flag = (HglFlag *) ptr8;

    return ((flag->status & HGL_FLAGS_STATUS_PARSED) != 0);
}

bool hgl_flags_occured_before(void *opt_a, void *opt_b)
{
    uint8_t *ptr8_a = (uint8_t *) opt_a;
    uint8_t *ptr8_b = (uint8_t *) opt_b;
    ptr8_a -= offsetof(HglFlag, value);
    ptr8_b -= offsetof(HglFlag, value);
    HglFlag *flag_a = (HglFlag *) ptr8_a;
    HglFlag *flag_b = (HglFlag *) ptr8_b;
    return (flag_a->parse_order < flag_b->parse_order);
}

void hgl_flags_generate_completion_cmd(FILE *stream, const char *program_name)
{
    fprintf(stream, "complete -f -d -W \"");
    for (size_t i = 0; i < hgl_n_flags_; i++) {
        const char *names = hgl_flags_[i].names;
        size_t offset = 0;
        for (size_t j = 0;; j++) {
            if (names[j] == ',') {
                fprintf(stream, "%.*s ", (int)(j - offset), names + offset);
                offset = ++j;
            }
            if (names[j] == '\0') {
                fprintf(stream, "%.*s ", (int)(j - offset), names + offset);
                break;
            }
        }
    }
    fprintf(stream, "\" %s", program_name);
    fprintf(stream, "\n");
    fflush(stream);
}

#endif

