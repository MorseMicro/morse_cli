/*
 * Copyright 2020 Morse Micro
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

#include <stdbool.h>
#include "utilities.h"
#include "transport/transport.h"
#include "mm_argtable3.h"

#define MORSE_ARRAY_SIZE(array) (sizeof((array)) / sizeof((array[0])))

#define MORSE_OK 0
#define MORSE_ARG_ERR 1
#define MORSE_CMD_ERR 2

typedef struct statistics_offchip_data offchip_stats_t; /* typedef reqd for circular definition */

struct morsectrl {
    bool debug;
    struct morsectrl_transport *transport;
    offchip_stats_t *stats;
    size_t n_stats;
};

enum mm_intr_requirements {
    /** Indicates that for this command a network interface is not required */
    MM_INTF_NOT_REQUIRED = false,
    /** Indicates that for this command a network interface is required */
    MM_INTF_REQUIRED = true
};

enum mm_direct_chip_support {
    /** Indicates that this command does not support direct to chip communication */
    MM_DIRECT_CHIP_NOT_SUPPORTED = false,
    /** Indicates that this command supports direct to chip communication */
    MM_DIRECT_CHIP_SUPPORTED = true
};

/**
 * @brief Parse a config file to obtain transport, interface and other configuration options.
 *
 * The parser will allocate the memory for the transport, interface and configuration strings.
 *
 * @param file_opts     String containing the filename of the config file
 * @param trans_opts    Transport string (to be filled by parser)
 * @param iface_opts    Interface string (to be filled by parser)
 * @param cfg_opts      Other configuration options (to be filled by parser)
 * @param debug         Whether or not to print debug information while parsing
 *
 * @return              0 if success otherwise -1
 */
int morsectrl_config_file_parse(const char *file_opts,
                                char **trans_opts,
                                char **iface_opts,
                                char **cfg_opts,
                                bool debug);

/* Our command link handlers need to be aligned to 8 byte boundaries (for up to 64-bit platforms) */
#define MM_CLI_HANDLER_ALIGN __attribute__((aligned(8)))

struct MM_CLI_HANDLER_ALIGN command_handler
{
    const char *name;
    int (*init)(struct morsectrl *, struct mm_argtable *);
    int (*handler)(struct morsectrl *, int, char **);
    const enum mm_intr_requirements is_intf_cmd;
    const enum mm_direct_chip_support direct_chip_supported_cmd;
    const bool deprecated;
    struct mm_argtable args;
};

#define _MM_CLI_HANDLER(command, _is_intf_cmd, _direct_chip_supported_cmd, deprecated) \
    __attribute__((weak)) int command##_init(struct morsectrl *mors, struct mm_argtable *mmargs); \
    __attribute__((section("cli_handlers"))) MM_CLI_HANDLER_ALIGN \
    struct command_handler command##_cli_handler = { \
        #command, \
        command##_init, \
        command, \
        _is_intf_cmd, \
        _direct_chip_supported_cmd, \
        deprecated }

#define MM_CLI_HANDLER(command, _is_intf_cmd, _direct_chip_supported_cmd) \
     _MM_CLI_HANDLER(command, _is_intf_cmd, _direct_chip_supported_cmd, false)

#define MM_CLI_HANDLER_DEPRECATED(command, _is_intf_cmd, _direct_chip_supported_cmd) \
    _MM_CLI_HANDLER(command, _is_intf_cmd, _direct_chip_supported_cmd, true)
