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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "portable_endian.h"

#include "command.h"
#include "mm_argtable3.h"

/** Structure for a get version confirm */
struct PACKED get_version_response
{
    /** Length of version */
    int32_t length;
    /** The version string */
    uint8_t version[128];
};

int version_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args, "Read the software versions");
    return 0;
}

int version(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    uint32_t len;
    struct get_version_response *version;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, 0);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*version));

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    version = TBUFF_TO_RSP(rsp_tbuff, struct get_version_response);

    ret = morsectrl_send_command(mors->transport, MORSE_COMMAND_GET_VERSION,
                                 cmd_tbuff, rsp_tbuff);
exit:
    if (ret < 0)
    {
        mctrl_err("Get firmware version failed (%d)\n", ret);
    }
    else
    {
        len = le32toh(version->length);
        version->version[len] = '\0';
        mctrl_print("FW Version: %s\n", version->version);
    }

    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(version, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
