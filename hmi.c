/*
 * Copyright 2026 Morse Micro
 * SPDX-License-Identifier: GPL-2.0-or-later OR LicenseRef-MorseMicroCommercial
 */

#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "morse_commands.h"
#include "utilities.h"

static struct
{
    struct arg_rex *command;
} args;

static struct mm_argtable trigger;
static struct mm_argtable action;
static struct mm_argtable link_cmd;

static struct
{
    struct arg_rex *subcmd;
} trigger_args;
static struct mm_argtable trigger_gpio;
static struct mm_argtable trigger_user;

static struct
{
    struct arg_rex *subcmd;
} action_args;
static struct mm_argtable action_standby_exit;
static struct mm_argtable action_user_action;
static struct mm_argtable action_gpio_pattern;

static struct
{
    struct arg_int *gpio_num;
    struct arg_int *debounce_ms;
    struct arg_rex *pullup;
    struct arg_rex *edge;
} gpio_args;

static struct
{
    struct arg_int *gpio_num;
    struct arg_str *pattern;
    struct arg_int *repeat;
} gpio_pattern_args;

static struct
{
    struct arg_int *id1;
    struct arg_int *id2;
} link_args;

static struct mm_argtable fire_trigger_cmd;
static struct
{
    struct arg_int *trigger_id;
} fire_trigger_args;

static struct mm_argtable *subcmds[] = { &trigger,
                                         &trigger_gpio,
                                         &trigger_user,
                                         &action,
                                         &action_standby_exit,
                                         &action_user_action,
                                         &action_gpio_pattern,
                                         &link_cmd,
                                         &fire_trigger_cmd };

int hmi_init(struct morsectrl *mors, struct mm_argtable *mm_args)
{
    MM_INIT_ARGTABLE(mm_args,
                     "Human-Machine Interface (HMI) commands",
                     args.command = arg_rex1(NULL,
                                             NULL,
                                             "(create_trigger|create_action|"
                                             "create_link|fire_trigger)",
                                             "{create_trigger|create_action|"
                                             "create_link|fire_trigger}",
                                             0,
                                             "HMI subcommand"),
                     arg_rem(NULL, "create_trigger - Hardware or software event source"),
                     arg_rem(NULL, "create_action  - Operation to perform when a trigger fires"),
                     arg_rem(NULL, "create_link    - Link a trigger with an action"),
                     arg_rem(NULL, "fire_trigger   - Fire a user trigger"));
    args.command->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(
        &trigger,
        "HMI trigger creation",
        trigger_args.subcmd = arg_rex1(NULL, NULL, "(gpio|user)", "{gpio|user}", 0, "Trigger type"),
        arg_rem(NULL, "gpio  - GPIO edge trigger"),
        arg_rem(NULL, "user  - Software-fired user trigger"));
    trigger_args.subcmd->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(
        &trigger_gpio,
        "GPIO edge trigger",
        gpio_args.gpio_num = arg_int1(NULL, "gpio", "<n>", "GPIO number"),
        gpio_args.debounce_ms =
            arg_int0(NULL, "debounce_ms", "<n>", "Debounce time in milliseconds (default: 0)"),
        gpio_args.pullup = arg_rex0(NULL,
                                    "pullup",
                                    "(enable|disable)",
                                    "{enable|disable}",
                                    0,
                                    "Pullup (default: disable)"),
        gpio_args.edge = arg_rex0(NULL,
                                  "edge",
                                  "(rising|falling)",
                                  "{rising|falling}",
                                  0,
                                  "Edge trigger type (default: rising)"));

    MM_INIT_ARGTABLE(&trigger_user, "Software-fired user trigger");

    MM_INIT_ARGTABLE(&action,
                     "HMI action creation",
                     action_args.subcmd = arg_rex1(NULL, NULL,
                            "(standby_exit|user|gpio_pattern)",
                            "{standby_exit|user|gpio_pattern}",
                            0, "Action type"),
                     arg_rem(NULL, "standby_exit - Exit standby mode"),
                     arg_rem(NULL, "user         - Send user action event to host"),
                     arg_rem(NULL, "gpio_pattern - Drive a GPIO output pattern"));
    action_args.subcmd->hdr.flag |= ARG_STOPPARSE;

    MM_INIT_ARGTABLE(&action_standby_exit, "Exit standby mode");
    MM_INIT_ARGTABLE(&action_user_action, "Send user action event to host");

    MM_INIT_ARGTABLE(
        &action_gpio_pattern,
        "Drive a GPIO output pattern",
        gpio_pattern_args.gpio_num = arg_int1(NULL, "gpio", "<n>", "GPIO number to drive"),
        gpio_pattern_args.pattern = arg_str0(NULL,
                                             "pattern",
                                             "\"<s1> <d1>...\"",
                                             "State/duration pairs, 0..16 pairs "
                                             "(s: 0-1, d: milliseconds)"),
        gpio_pattern_args.repeat = arg_int0(
            NULL, "repeat", "<n>", "Number of times to repeat the pattern "
            "(0: forever, default: 1)"),
        arg_rem(NULL, ""),
        arg_rem(NULL, "  --pattern is a list of <state> <duration> pairs."),
        arg_rem(NULL, "  State 0 = drive low, 1 = drive high."),
        arg_rem(NULL, "  Each entry is held for its duration. A zero duration drives"),
        arg_rem(NULL, "  the state and advances to the next entry without delay."),
        arg_rem(NULL, "  At the end of the pattern, the GPIO stays driven to the last"),
        arg_rem(NULL, "  state it had. Omitting --pattern cancels any active pattern "),
        arg_rem(NULL, "  and leaves the pin at its current level."));

    MM_INIT_ARGTABLE(&link_cmd,
                     "Link a trigger with an action",
                     link_args.id1 = arg_int1(NULL, NULL, "<trigger ID>", "Trigger ID"),
                     link_args.id2 = arg_int1(NULL, NULL, "<action ID>", "Action ID"));

    MM_INIT_ARGTABLE(&fire_trigger_cmd,
                     "Fire a user trigger",
                     fire_trigger_args.trigger_id =
                         arg_int1(NULL, NULL, "<trigger_id>", "Trigger ID"));

    return 0;
}

static void hmi_error_code_hint(int error)
{
    switch (error)
    {
        case MORSE_RET_ENOENT:
            mctrl_err("ENOENT: Unknown trigger/action ID\n");
            break;
        case MORSE_RET_E2BIG:
            mctrl_err("E2BIG: Argument list too long\n");
            break;
        case MORSE_RET_ENOBUFS:
            mctrl_err("ENOBUFS: No buffer available. You reached one of:\n");
            mctrl_err("- The maximum number of triggers or actions\n");
            mctrl_err("- The maximum number of triggers or actions of a specific type\n");
            mctrl_err("- The maximum number of actions a specific trigger can trigger\n");
            break;
        case MORSE_RET_ENOSYS:
            mctrl_err("ENOSYS: Feature is disabled in firmware\n");
            break;
        case MORSE_RET_EEXIST:
            mctrl_err("EEXIST: Resource exists. "
                "For GPIO triggers it means there is already a trigger on this GPIO\n");
            break;
        case MORSE_RET_EINVAL:
            mctrl_err("EINVAL: Invalid argument. Example: GPIO doesn't exist\n");
            break;
        default:
            break;
    }
}

static int hmi_trigger_user(struct morse_cmd_req_hmi_create_trigger *req, int argc, char *argv[])
{
    return mm_parse_argtable("hmi create_trigger user", &trigger_user, argc, argv) ? -1 : 0;
}

/* Returns trigger_id (>= 0) on success, < 0 on error */
static int hmi_trigger_gpio(struct morse_cmd_req_hmi_create_trigger *req, int argc, char *argv[])
{
    int ret = mm_parse_argtable("hmi create_trigger gpio", &trigger_gpio, argc, argv);
    if (ret)
    {
        return -1;
    }

    struct morse_cmd_hmi_trigger_params_gpio *gpio_params =
        (struct morse_cmd_hmi_trigger_params_gpio *)req->param_buff;
    gpio_params->gpio = (uint8_t)gpio_args.gpio_num->ival[0];
    gpio_params->debounce_ms =
        htole32((gpio_args.debounce_ms->count > 0) ? gpio_args.debounce_ms->ival[0] : 0);
    gpio_params->pullup =
        (uint8_t)(gpio_args.pullup->count > 0 && strcmp(gpio_args.pullup->sval[0], "enable") == 0) ?
            1 :
            0;
    gpio_params->edge =
        (uint8_t)(gpio_args.edge->count > 0 && strcmp(gpio_args.edge->sval[0], "falling") == 0) ?
            2 :
            1;

    return 0;
}

static int process_hmi_trigger(struct morsectrl *mors, int argc, char *argv[])
{
    struct morse_cmd_req_hmi_create_trigger *req;
    struct morse_cmd_resp_hmi_create_trigger *rsp;

    int ret = mm_parse_argtable("hmi create_trigger", &trigger, argc, argv);
    if (ret)
    {
        return ret;
    }

    enum morse_cmd_hmi_trigger_type trigger_type = 0;
    uint16_t param_size = 0;
    int (*trigger_type_parser)(
        struct morse_cmd_req_hmi_create_trigger *req, int argc, char *argv[]);

    if (strcmp("gpio", trigger_args.subcmd->sval[0]) == 0)
    {
        trigger_type = MORSE_CMD_HMI_TRIGGER_TYPE_GPIO;
        param_size = sizeof(struct morse_cmd_hmi_trigger_params_gpio);
        trigger_type_parser = hmi_trigger_gpio;
    }
    else if (strcmp("user", trigger_args.subcmd->sval[0]) == 0)
    {
        trigger_type = MORSE_CMD_HMI_TRIGGER_TYPE_USER;
        param_size = 0;
        trigger_type_parser = hmi_trigger_user;
    }
    else
    {
        /* Shouldn't be reachable. This case is caught by argtable */
        MCTRL_ASSERT(false, "not reached");
        return -1;
    }

    struct morsectrl_transport_buff *req_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req) + param_size);
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));
    if (!req_tbuff || !rsp_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return -1;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_hmi_create_trigger);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_hmi_create_trigger);
    req->trigger_type = trigger_type;
    req->param_len = htole16(param_size);
    ret = trigger_type_parser(req,
                              argc - trigger_args.subcmd->hdr.idx,
                              argv + trigger_args.subcmd->hdr.idx);
    if (ret)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return ret;
    }

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_HMI_CREATE_TRIGGER,
                                 req_tbuff,
                                 rsp_tbuff);
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    if (ret)
    {
        hmi_error_code_hint(ret);
        return ret;
    }

    uint8_t trigger_id = rsp->trigger_id;
    mctrl_print("%d\n", trigger_id);

    return 0;
}

static int hmi_action_standby_exit(struct morse_cmd_req_hmi_create_action *req,
                                   int argc,
                                   char *argv[])
{
    return mm_parse_argtable("hmi create_action standby_exit", &action_standby_exit, argc, argv) ?
               -1 :
               0;
}

static int hmi_action_user_action(struct morse_cmd_req_hmi_create_action *req,
                                  int argc,
                                  char *argv[])
{
    return mm_parse_argtable("hmi create_action user", &action_user_action, argc, argv) ?
               -1 :
               0;
}

/**
 * Parse a whitespace-separated list of <state> <duration> pairs into
 * the two parallel output arrays. Writes the pair count to @p out_count on
 * success. The list must contain at least one pair; an empty list is an error
 * (to cancel a pattern, omit --pattern rather than passing an empty one).
 *
 * Returns 0 on success, -1 on any parse error (and prints the reason).
 */
#ifndef UNIT_TESTS
static
#endif
int parse_pattern_entries(const char *str,
                          uint8_t *intensities_out,
                          uint32_t *durations_out,
                          uint32_t *out_count)
{
    const char *cursor = str;
    int token_index = 0;
    int pair_count = 0;
    char *endptr;
    uint64_t value;

    for (;;)
    {
        while (isspace((unsigned char)*cursor))
        {
            cursor++;
        }
        if (*cursor == '\0')
        {
            break;
        }
        if (pair_count == MORSE_CMD_HMI_GPIO_PATTERN_MAX_ENTRIES)
        {
            mctrl_err("Too many pattern pairs (max %d)\n",
                      MORSE_CMD_HMI_GPIO_PATTERN_MAX_ENTRIES);
            return -1;
        }
        if (!isdigit((unsigned char)*cursor))
        {
            mctrl_err("Invalid pattern value: must be space-separated non-negative integers\n");
            return -1;
        }


        errno = 0;
        value = strtoul(cursor, &endptr, 10);
        if (token_index % 2 == 0)
        {
            if (errno == ERANGE || value > 1)
            {
                mctrl_err("Pattern state out of range (max %u)\n", 1);
                return -1;
            }
            intensities_out[pair_count] = (uint8_t)value;
        }
        else
        {
            if (errno == ERANGE || value > UINT32_MAX)
            {
                mctrl_err("Pattern duration out of range (max %u)\n", UINT32_MAX);
                return -1;
            }
            durations_out[pair_count] = (uint32_t)value;
            pair_count++;
        }
        token_index++;
        cursor = endptr;
    }

    if (token_index % 2 != 0)
    {
        mctrl_err("Pattern must contain pairs of <state> <duration>\n");
        return -1;
    }

    if (pair_count == 0)
    {
        mctrl_err("Pattern is empty: omit --pattern to cancel an active pattern\n");
        return -1;
    }

    *out_count = (uint8_t)pair_count;
    return 0;
}

_Static_assert(__builtin_offsetof(struct morse_cmd_hmi_action_params_gpio_pattern, count)
    % sizeof(__le32) ==0, "count must be 32 bits aligned");
_Static_assert(__builtin_offsetof(struct morse_cmd_hmi_action_params_gpio_pattern, durations_ms)
    % sizeof(__le32) == 0, "durations_ms must be 32 bits aligned");

static int hmi_action_gpio_pattern(struct morse_cmd_req_hmi_create_action *req,
                                   int argc,
                                   char *argv[])
{
    uint32_t durations[MORSE_CMD_HMI_GPIO_PATTERN_MAX_ENTRIES];
    uint32_t count = 0;

    int ret = mm_parse_argtable("hmi create_action gpio_pattern", &action_gpio_pattern, argc, argv);
    if (ret)
    {
        return -1;
    }

    struct morse_cmd_hmi_action_params_gpio_pattern *params =
        (struct morse_cmd_hmi_action_params_gpio_pattern *)req->param_buff;
    params->gpio = (uint8_t)gpio_pattern_args.gpio_num->ival[0];
    params->repeat = htole32(
        (gpio_pattern_args.repeat->count > 0) ? gpio_pattern_args.repeat->ival[0] : 1);
    params->version = 1;

    /* Omitting --pattern leaves count == 0, which cancels any active pattern. */
    if (gpio_pattern_args.pattern->count > 0 &&
        parse_pattern_entries(gpio_pattern_args.pattern->sval[0],
                              params->intensities, durations, &count))
    {
        return -1;
    }

    params->count = htole32(count);
    for (uint32_t i = 0; i < count; i++)
    {
        params->durations_ms[i] = htole32(durations[i]);
    }

    return 0;
}

static int process_hmi_action(struct morsectrl *mors, int argc, char *argv[])
{
    struct morse_cmd_req_hmi_create_action *req;
    struct morse_cmd_resp_hmi_create_action *rsp;

    int ret = mm_parse_argtable("hmi create_action", &action, argc, argv);
    if (ret)
    {
        return ret;
    }

    enum morse_cmd_hmi_action_type action_type = 0;
    uint16_t param_size = 0;
    int (*action_type_parser)(struct morse_cmd_req_hmi_create_action *req, int argc, char *argv[]);

    if (strcmp("standby_exit", action_args.subcmd->sval[0]) == 0)
    {
        action_type = MORSE_CMD_HMI_ACTION_TYPE_STANDBY_EXIT;
        param_size = 0;
        action_type_parser = hmi_action_standby_exit;
    }
    else if (strcmp("user", action_args.subcmd->sval[0]) == 0)
    {
        action_type = MORSE_CMD_HMI_ACTION_TYPE_USER;
        param_size = 0;
        action_type_parser = hmi_action_user_action;
    }
    else if (strcmp("gpio_pattern", action_args.subcmd->sval[0]) == 0)
    {
        action_type = MORSE_CMD_HMI_ACTION_TYPE_GPIO_PATTERN;
        param_size = sizeof(struct morse_cmd_hmi_action_params_gpio_pattern);
        action_type_parser = hmi_action_gpio_pattern;
    }
    else
    {
        /* Shouldn't be reachable. This case is caught by argtable */
        MCTRL_ASSERT(false, "not reached");
        return -1;
    }

    struct morsectrl_transport_buff *req_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req) + param_size);
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));
    if (!req_tbuff || !rsp_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return -1;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_hmi_create_action);
    rsp = TBUFF_TO_RSP(rsp_tbuff, struct morse_cmd_resp_hmi_create_action);
    req->action_type = action_type;
    req->param_len = htole16(param_size);
    ret = action_type_parser(req,
                             argc - action_args.subcmd->hdr.idx,
                             argv + action_args.subcmd->hdr.idx);
    if (ret)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return ret;
    }

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_HMI_CREATE_ACTION,
                                 req_tbuff,
                                 rsp_tbuff);
    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    if (ret)
    {
        hmi_error_code_hint(ret);
        return ret;
    }

    uint8_t action_id = rsp->action_id;
    mctrl_print("%d\n", action_id);

    return 0;
}

static int process_hmi_link(struct morsectrl *mors, int argc, char *argv[])
{
    struct morsectrl_transport_buff *req_tbuff;
    struct morsectrl_transport_buff *rsp_tbuff;
    struct morse_cmd_req_hmi_create_link *req;

    int ret = mm_parse_argtable("hmi create_link", &link_cmd, argc, argv);
    if (ret)
    {
        return ret;
    }

    req_tbuff = morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    rsp_tbuff = morsectrl_transport_resp_alloc(mors->transport,
                                               sizeof(struct morse_cmd_resp_hmi_create_link));
    if (!req_tbuff || !rsp_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return -1;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_hmi_create_link);
    req->trigger_id = (uint8_t)link_args.id1->ival[0];
    req->action_id = (uint8_t)link_args.id2->ival[0];

    ret =
        morsectrl_send_command(mors->transport, MORSE_CMD_ID_HMI_CREATE_LINK, req_tbuff, rsp_tbuff);
    hmi_error_code_hint(ret);

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

static int process_hmi_fire_trigger(struct morsectrl *mors, int argc, char *argv[])
{
    struct morse_cmd_req_hmi_fire_trigger *req;
    struct morse_cmd_resp_hmi_fire_trigger *rsp;

    int ret = mm_parse_argtable("hmi fire_trigger", &fire_trigger_cmd, argc, argv);
    if (ret)
    {
        return ret;
    }

    struct morsectrl_transport_buff *req_tbuff =
        morsectrl_transport_cmd_alloc(mors->transport, sizeof(*req));
    struct morsectrl_transport_buff *rsp_tbuff =
        morsectrl_transport_resp_alloc(mors->transport, sizeof(*rsp));
    if (!req_tbuff || !rsp_tbuff)
    {
        morsectrl_transport_buff_free(req_tbuff);
        morsectrl_transport_buff_free(rsp_tbuff);
        return -1;
    }

    req = TBUFF_TO_REQ(req_tbuff, struct morse_cmd_req_hmi_fire_trigger);
    req->trigger_id = (uint8_t)fire_trigger_args.trigger_id->ival[0];

    ret = morsectrl_send_command(mors->transport,
                                 MORSE_CMD_ID_HMI_FIRE_TRIGGER,
                                 req_tbuff,
                                 rsp_tbuff);
    hmi_error_code_hint(ret);

    morsectrl_transport_buff_free(req_tbuff);
    morsectrl_transport_buff_free(rsp_tbuff);
    return ret;
}

int hmi(struct morsectrl *mors, int argc, char *argv[])
{
    int ret = -1;
    const char *subcmd = args.command->sval[0];

    if (strcmp("create_trigger", subcmd) == 0)
    {
        ret = process_hmi_trigger(mors, argc, argv);
    }
    else if (strcmp("create_action", subcmd) == 0)
    {
        ret = process_hmi_action(mors, argc, argv);
    }
    else if (strcmp("create_link", subcmd) == 0)
    {
        ret = process_hmi_link(mors, argc, argv);
    }
    else if (strcmp("fire_trigger", subcmd) == 0)
    {
        ret = process_hmi_fire_trigger(mors, argc, argv);
    }

    if (mm_check_help_argtable(subcmds, MORSE_ARRAY_SIZE(subcmds)))
    {
        ret = 0;
    }

    for (int i = 0; i < MORSE_ARRAY_SIZE(subcmds); i++)
    {
        mm_free_argtable(subcmds[i]);
    }

    return ret;
}

int hmi_help(void)
{
    mm_help_argtable("hmi create_trigger", &trigger);
    mm_help_argtable("hmi create_action", &action);
    mm_help_argtable("hmi create_link", &link_cmd);
    mm_help_argtable("hmi fire_trigger", &fire_trigger_cmd);
    return 0;
}

MM_CLI_HANDLER_CUSTOM_HELP(hmi, MM_INTF_REQUIRED, MM_DIRECT_CHIP_NOT_SUPPORTED);
