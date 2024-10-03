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
#include <stdint.h>

#ifdef MORSE_WIN_BUILD
#include <winsock2.h>
#include <windows.h>
#endif

#include "portable_endian.h"
#include "morsectrl.h"

#define PACKED __attribute__((packed))

/*
 * The header for a command
 */
struct PACKED command_hdr
{
    /** Flags - used between host and firmware */
    uint16_t flags;
    /** Message ID - from enum morse_commands_id */
    uint16_t message_id;
    /** Command length excluding the header */
    uint16_t len;
    /** Host sequence id - used between host and firmware only */
    uint16_t host_id;
    /** Interface id - set by the host and copied into the response */
    uint16_t vif_id;
    /** Padding for word alignment */
    uint16_t pad;
};

struct PACKED command
{
    /** The request command starts with a header */
    struct command_hdr hdr;
    /** An opaque data pointer */
    uint8_t data[0];
};

struct PACKED response
{
    /** The confirm header */
    struct command_hdr hdr;
    /** The status of the of the command. @see morse_error_t */
    uint32_t status;
    /** An opaque data pointer */
    uint8_t data[0];
};

/**
 *  Host to firmware/driver messages
 *
 * @note you must hardcode the values here, and they must all be unique
 */
enum morse_commands_id
{
    MORSE_COMMAND_SET_CHANNEL = 0x0001,
    MORSE_COMMAND_GET_VERSION = 0x0002,
    MORSE_COMMAND_ADD_INTERFACE = 0x0004,
    MORSE_COMMAND_REMOVE_INTERFACE = 0x0005,
    MORSE_COMMAND_BSS_CONFIG = 0x0006,
    MORSE_COMMAND_APP_STATS_LOG_DEPRECATED = 0x0007,
    MORSE_COMMAND_APP_STATS_LOG = 0x2007,
    MORSE_COMMAND_APP_STATS_RESET = 0x2008,
    MORSE_COMMAND_RPG = 0x0009,
    MORSE_COMMAND_CFG_SCAN = 0x0010,
    MORSE_COMMAND_GET_FULL_CHANNEL = 0x0013,
    MORSE_COMMAND_SET_STA_STATE = 0x0014,
    MORSE_COMMAND_SET_BSS_COLOR = 0x0015,
    MORSE_COMMAND_HEALTH_CHECK = 0x0019,
    MORSE_COMMAND_SET_CTS_SELF_PS = 0x001A,
    MORSE_COMMAND_GET_DTIM_CHANNEL = 0x001C,
    MORSE_COMMAND_GET_CURRENT_CHANNEL = 0x001D,
    MORSE_COMMAND_SET_LONG_SLEEP_CONFIG = 0x0021,
    MORSE_COMMAND_SET_DUTY_CYCLE = 0x00022,
    MORSE_COMMAND_GET_DUTY_CYCLE = 0x00023,
    MORSE_COMMAND_MAC_ADDR = 0x0029,
    MORSE_COMMAND_MPSW_CONFIG = 0x0030,
    MORSE_COMMAND_STANDBY_MODE = 0x0031,
    MORSE_COMMAND_DHCP_OFFLOAD = 0x0032,
    MORSE_COMMAND_SET_KEEP_ALIVE_OFFLOAD = 0x0033,
    MORSE_COMMAND_GET_SET_GENERIC_PARAM = 0x003E,
    MORSE_COMMAND_UAPSD_CONFIG = 0x0040,

    MORSE_COMMAND_MAC_STATS_LOG_DEPRECATED = 0x000C,
    MORSE_COMMAND_MAC_STATS_LOG = 0x200C,
    MORSE_COMMAND_MAC_STATS_RESET = 0x200D,
    MORSE_COMMAND_UPHY_STATS_LOG_DEPRECATED = 0x000E,
    MORSE_COMMAND_UPHY_STATS_LOG = 0x200E,
    MORSE_COMMAND_UPHY_STATS_RESET = 0x200F,

    MORSE_COMMAND_SET_TX_PKT_LIFETIME_US = 0x100C,
    MORSE_COMMAND_SET_PHYSM_WATCHDOG = 0x100D,

    /* Commands to driver */
    MORSE_COMMAND_DRIVER_START = 0xA000,
    MORSE_COMMAND_SET_LISTEN_INTERVAL = 0xA003,
    MORSE_COMMAND_SET_AMPDU = 0xA004,
    MORSE_COMMAND_SET_RAW_DEPRECATED = 0xA005,
    MORSE_COMMAND_COREDUMP = 0xA006,
    MORSE_COMMAND_SET_S1G_OP_CLASS = 0xA007,
    MORSE_COMMAND_SEND_WAKE_ACTION_FRAME = 0xA008,
    MORSE_COMMAND_VENDOR_IE_CONFIG = 0xA009,
    MORSE_COMMAND_TWT_SET_CONF = 0xA010,
    MORSE_COMMAND_GET_AVAILABLE_CHANNELS = 0xA011,
    MORSE_COMMAND_SET_ECSA_S1G_INFO = 0xA012,
    MORSE_COMMAND_GET_HW_VERSION = 0xA013,
    MORSE_COMMAND_CAC_SET = 0xA014,
    MORSE_COMMAND_DRIVER_SET_DUTY_CYCLE = 0xA015,
    MORSE_COMMAND_MBSSID_INFO = 0xA016,
    MORSE_COMMAND_MESH_CONFIG = 0xA018,
    MORSE_COMMAND_MBCA_SET_CONF = 0xA019,
    MORSE_COMMAND_DYNAMIC_PEERING_SET_CONF = 0xA020,
    MORSE_COMMAND_CONFIG_RAW = 0xA021,
    MORSE_COMMAND_DRIVER_END,

    MORSE_TEST_COMMAND_SET_MAX_AMPDU_LENGTH = 0x8016,
};

/**
 * Error numbers the FW may return from commands
 * Defined here as errno numbers are not portable across architectures
 */
enum morse_cmd_return_code {
    MORSE_RET_SUCCESS     = 0,
    MORSE_RET_EPERM       = -1,
    MORSE_RET_ENXIO       = -6,
    MORSE_RET_ENOMEM      = -12,
    MORSE_RET_EINVAL      = -22,
};

int morsectrl_send_command(struct morsectrl_transport *transport,
                           int message_id,
                           struct morsectrl_transport_buff *cmd,
                           struct morsectrl_transport_buff *resp);
