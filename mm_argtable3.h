/*
 * Copyright 2023 Morse Micro
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see
 * <https://www.gnu.org/licenses/>.
 */
#pragma once

#include <stdlib.h>
#include <string.h>
#include "utilities.h"
#include "argtable3/argtable3.h"

extern char TOOL_NAME[];

struct mm_argtable {
    int count;
    struct arg_lit *help;
    struct arg_end *end;
    void **argtable;
};

void mctrl_print(const char* format, ...);

static inline void mm_help_argtable(const char *name, struct mm_argtable *mm_args)
{
    mctrl_print("\t%s", name);
    arg_print_syntax(stdout, mm_args->argtable, "\n");
    arg_print_glossary(stdout, mm_args->argtable, "\t\t%-40s %s\n");
}

static inline int mm_parse_argtable_noerror(const char *name, struct mm_argtable *mm_args,
                                            int argc, char **argv)
{
    int nerrors;

    nerrors = arg_parse(argc, argv, mm_args->argtable);

    if (mm_args->help->count > 0)
    {
        mctrl_print("%s %s", TOOL_NAME, name ? name : "");
        arg_print_syntax(stdout, mm_args->argtable, "\n");
        arg_print_glossary(stdout, mm_args->argtable, "\t%-40s %s\n");
        return -1;
    }

    return nerrors;
}

static inline int mm_parse_argtable(const char *name, struct mm_argtable *mm_args,
                                    int argc, char **argv)
{
    int nerrors = mm_parse_argtable_noerror(name, mm_args, argc, argv);

    if (nerrors > 0)
    {
        arg_print_errors(stdout, mm_args->end, name != NULL ? name : TOOL_NAME);
        mctrl_print("Try %s --help for more information\n", TOOL_NAME);
    }

    return nerrors;
}

static inline void mm_free_argtable(struct mm_argtable *mm_args)
{
    arg_freetable(mm_args->argtable, mm_args->count);
    free(mm_args->argtable);
}

// NOLINT(-whitespace/comma)
#define MM_INIT_ARGTABLE(_argtable, ...)                                    \
    do {                                                                    \
        _argtable->end = arg_end(20);                                       \
        void *tmp_table[] = {                                               \
            _argtable->help = arg_lit0("h", "help",                         \
                                    "display this help and exit"),          \
            __VA_ARGS__ __VA_OPT__(,)                                       \
            _argtable->end                                                  \
        };                                                                  \
        _argtable->argtable = malloc(sizeof(tmp_table));                    \
        memcpy(_argtable->argtable, tmp_table, sizeof(tmp_table));          \
        _argtable->count = sizeof(tmp_table) / sizeof(_argtable->end);      \
    } while (0)
// NOLINT(+whitespace/comma)
