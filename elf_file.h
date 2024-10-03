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

#include <stddef.h>
#include <stdio.h>
#include "offchip_statistics.h"
#include "transport/transport.h"

int morse_stats_load(struct statistics_offchip_data **stats_handle,
                     size_t *n_rec,
                     const uint8_t *data);

int load_elf(struct morsectrl *mors, int argc, char *argv[]);
