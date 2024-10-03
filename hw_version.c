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

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include "morsectrl.h"
#include "portable_endian.h"

#include "command.h"

/** Structure for a get hw_version confirm */
struct PACKED get_hw_version_response
{
    /** The version string */
    uint8_t hw_version[64];
};

static void usage(struct morsectrl *mors) {
    mctrl_print("\thw_version\t\tprints hardware version\n");
}

int hw_version(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct get_hw_version_response *hw_version;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    if (argc == 0)
    {
        usage(mors);
        return 0;
    }

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, 0);
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*hw_version));

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    hw_version = TBUFF_TO_RSP(rsp_tbuff, struct get_hw_version_response);

    if (argc > 1)
    {
        mctrl_err("Invalid command parameters\n");
        usage(mors);
        ret = -1;
        goto exit;
    }

    ret = morsectrl_send_command(mors->transport, MORSE_COMMAND_GET_HW_VERSION,
                                 cmd_tbuff, rsp_tbuff);
exit:
    if (ret < 0)
    {
        mctrl_err("Get hardware version failed %d\n", ret);
    }
    else
    {
        mctrl_print("HW Version: %s\n", hw_version->hw_version);
    }

    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(hw_version, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
