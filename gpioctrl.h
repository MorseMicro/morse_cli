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

#define RESET_GPIO "MM_RESET_PIN"
#define JTAG_GPIO "MM_JTAG_PIN"

int gpio_export(int pin);
int gpio_unexport(int pin);

int gpio_set_dir(int pin, const char dirc[]);
int gpio_set_val(int pin, int val);
int gpio_get_env(char env_var[]);

int path_exists(char path[]);
