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
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "command.h"
#include "utilities.h"

struct PACKED command_otp_req
{
    /** Bool, 1=enabled, 0=disabled */
    uint8_t write_otp;
    uint8_t bank_num;
    uint32_t bank_val;
};

struct PACKED command_otp_cfm
{
    uint32_t bank_val;
};

static void usage(struct morsectrl *mors)
{
    mctrl_print("\totp <bank_num> [-w <bank_val>]\n"
           "\t\t\t\tread/write OTP bank given from chip\n");
    mctrl_print("\t\tbank_num\tbank number to read/write from/to. eg.: for 610x [0-7]\n");
}

int otp(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    uint32_t bank_num;
    int option;

    struct command_otp_req *cmd;
    struct command_otp_cfm *resp;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    if (argc == 0)
    {
            usage(mors);
            return 0;
    }

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*cmd));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*resp));

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    cmd = TBUFF_TO_CMD(cmd_tbuff, struct command_otp_req);
    resp = TBUFF_TO_RSP(rsp_tbuff, struct command_otp_cfm);
    cmd->write_otp = 0;

    switch (argc)
    {
        case 2:
        case 4:
            if (str_to_uint32(argv[1], &bank_num))
            {
                mctrl_err("Invalid OTP bank number\n");
                usage(mors);
                ret = -1;
                goto exit;
            }
            cmd->bank_num = bank_num;
            while ((option = getopt(argc-1, argv+1, "w:")) != -1)
            {
                switch (option)
                {
                    default :
                        usage(mors);
                        ret = -1;
                        goto exit;
                }
            }
            break;
        default:
            mctrl_err("Invalid arguments\n");
            usage(mors);
            ret = -1;
            goto exit;
    }

    ret = morsectrl_send_command(mors->transport, MORSE_COMMAND_OTP,
                                 cmd_tbuff, rsp_tbuff);

exit:
    if (ret)
        mctrl_err("Command OTP Failed(%d)\n", ret);
    else if (!cmd->write_otp)
        mctrl_print("OTP Bank(%d): 0x%x\n", bank_num, resp->bank_val);

    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

MM_CLI_HANDLER(otp, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
