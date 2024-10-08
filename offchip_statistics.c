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
 */

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "offchip_statistics.h"
#include "command.h"


/*
* Get the offchip data for this tag,
* or NULL if none can be found.
*/
struct statistics_offchip_data *get_stats_offchip(const struct morsectrl *mors, stats_tlv_tag_t tag)
{
#ifdef ENABLE_TRANS_NL80211
    struct statistics_offchip_data *res = NULL;
    for (int i = 0; i < mors->n_stats; i++)
    {
        if (mors->stats[i].tag == tag)
        {
            res = mors->stats + i;
            break;
        }
    }
    return res;
#else
    return NULL;
#endif
}


int64_t get_signed_value_as_int64(const uint8_t *buf, uint32_t size)
{
    int64_t n = 0;
    switch (size)
    {
        case 1:
        {
            int8_t x;
            memcpy(&x, buf, size);
            n = (int64_t)x;
            break;
        }
        case 2:
        {
            int16_t x;
            memcpy(&x, buf, size);
            n = (int64_t)x;
            break;
        }
        case 4:
        {
            int32_t x;
            memcpy(&x, buf, size);
            n = (int64_t)x;
            break;
        }
        case 8:
            memcpy(&n, buf, size);
            break;
        default:
            mctrl_err("get_signed_value_as_int64 can't convert %d-byte quantity\n", size);
            break;
    }
    return n;
}

uint64_t get_unsigned_value_as_uint64(const uint8_t *buf, uint32_t size)
{
    uint64_t n = 0;
    switch (size)
    {
        case 1:
        {
            uint8_t x;
            memcpy(&x, buf, size);
            n = (uint64_t)x;
            break;
        }
        case 2:
        {
            uint16_t x;
            memcpy(&x, buf, size);
            n = (uint64_t)x;
            break;
        }
        case 4:
        {
            uint32_t x;
            memcpy(&x, buf, size);
            n = (uint64_t)x;
            break;
        }
        case 8:
            memcpy(&n, buf, size);
            break;
        default:
            mctrl_err("get_unsigned_value_as_uint64 can't convert %d-byte quantity\n", size);
            break;
    }
    return n;
}
