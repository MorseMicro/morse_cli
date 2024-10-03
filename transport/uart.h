/*
 * Copyright 2024 Morse Micro
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

/*
 * UART platform abstration API.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

struct uart_ctx;

#define UART_MAX_DEVICE_NAME_LEN         (256)

struct uart_config
{
    char dev_name[UART_MAX_DEVICE_NAME_LEN];

    int baudrate;
};

struct uart_ctx *uart_init(const struct uart_config *cfg);

int uart_deinit(struct uart_ctx *ctx);

int uart_read(struct uart_ctx *ctx, uint8_t *buf, size_t len);

int uart_write(struct uart_ctx *ctx, const uint8_t *buf, size_t len);
