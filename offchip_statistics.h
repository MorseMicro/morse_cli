/*
 * Copyright 2022 Morse Micro
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
 *
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "morsectrl.h"


typedef uint16_t stats_tlv_tag_t;
typedef uint16_t stats_tlv_len_t;
#define STATS_TLV_OVERHEAD (sizeof(stats_tlv_tag_t) + sizeof(stats_tlv_len_t))

/* New format specifiers get added here.
 * Must be kept in sync with the firmware.
 */
enum morse_statistics_format {
    MORSE_STATS_FMT_DEC = 0,
    MORSE_STATS_FMT_U_DEC = 1,
    MORSE_STATS_FMT_HEX = 2,
    MORSE_STATS_FMT_0_HEX = 3,
    MORSE_STATS_FMT_AMPDU_AGGREGATES = 4,
    MORSE_STATS_FMT_AMPDU_BITMAP = 5,
    MORSE_STATS_FMT_TXOP = 6,
    MORSE_STATS_FMT_PAGESET = 7,
    MORSE_STATS_FMT_RETRIES = 8,
    MORSE_STATS_FMT_RAW = 9,            /* Restricted Access Window */
    MORSE_STATS_FMT_CALIBRATION = 10,
    MORSE_STATS_FMT_DUTY_CYCLE = 11,
    MORSE_STATS_FMT_MAC_STATE = 12,

    MORSE_STATS_FMT_LAST, /* Used as default print, make sure this is last */
    MORSE_STATS_FMT_END = 0xFFFFFFFF,
};
#define STATS_OFFCHIP_STRING_TYPE_MAX 50
#define STATS_OFFCHIP_STRING_NAME_MAX 50
#define STATS_OFFCHIP_STRING_KEY_MAX 100

struct __attribute__((packed)) statistics_offchip_data {
    const char type_str[STATS_OFFCHIP_STRING_TYPE_MAX];
    const char name[STATS_OFFCHIP_STRING_NAME_MAX];
    const char key[STATS_OFFCHIP_STRING_KEY_MAX];
    enum morse_statistics_format format;
    stats_tlv_tag_t tag;
};

#define OLD_STATS_COMMAND_MASK 0xDF

struct statistics_offchip_data *get_stats_offchip(const struct morsectrl *mors,
                                                    stats_tlv_tag_t tag);
int64_t get_signed_value_as_int64(const uint8_t *buf, uint32_t size);
uint64_t get_unsigned_value_as_uint64(const uint8_t *buf, uint32_t size);
