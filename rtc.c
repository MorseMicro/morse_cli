/*
 * Copyright 2026 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

#include "command.h"
#include "mm_argtable3.h"
#include "utilities.h"

static struct
{
    struct arg_str *set_val;
} args;

int rtc_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args,
        "Get or set the real-time clock",
        args.set_val = arg_str0("s", "set", "<epoch time>",
            "Set the real-time clock to the given value (seconds)"),
        arg_rem(NULL, "If omitted, the current value is printed"));
    return 0;
}

int rtc(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    struct morse_cmd_req_rtc *req;
    struct morse_cmd_resp_rtc *rsp;
    struct morsectrl_transport_buff *cmd_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;

    cmd_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));

    if (!cmd_tbuff || !rsp_tbuff)
        goto exit;

    req = TBUFF_TO_REQ(cmd_tbuff, struct morse_cmd_req_rtc);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_rtc);

    req->write = 0;
    req->epoch_time_us = 0;

    if (args.set_val->count)
    {
        uint64_t epoch_s;

        if (str_to_uint64(args.set_val->sval[0], &epoch_s) < 0)
        {
            mctrl_err("Invalid argument - %s\n", args.set_val->sval[0]);
            goto exit;
        }

        req->write = 1;
        req->epoch_time_us = htole64(epoch_s * 1000000ULL);
    }

    ret = morsectrl_send_command(mors->transport, MORSE_CMD_ID_RTC,
                                 cmd_tbuff, rsp_tbuff);

exit:
    if (!ret && !req->write)
    {
        mctrl_print("%" PRIu64 "\n", le64toh(rsp->epoch_time_us) / 1000000ULL);
    }

    morsectrl_transport_buff_free(cmd_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);

    return ret;
}

MM_CLI_HANDLER(rtc, MM_INTF_REQUIRED, MM_DIRECT_CHIP_SUPPORTED);
