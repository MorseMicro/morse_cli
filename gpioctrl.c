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

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "utilities.h"
#include "gpioctrl.h"

static int gpio_sysfs_write(int value, const char *entry)
{
    char buff[8];
    int fd;

    fd = open(entry, O_WRONLY);
    if (fd == -1)
    {
        mctrl_err("Unable to open %s\n", entry);
        return -1;
    }

    snprintf(buff, sizeof(buff), "%d", value);
    if (write(fd, buff, 2) != 2)
    {
        mctrl_err("Error writing to %s\n", entry);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int gpio_export(int pin)
{
    char path_buff[40];

    snprintf(path_buff, sizeof(path_buff), "%s%d", "/sys/class/gpio/gpio", pin);
    if (!path_exists(path_buff))
    {
        return gpio_sysfs_write(pin, "/sys/class/gpio/export");
    }

    return 0;
}

int gpio_unexport(int pin)
{
    char path_buff[40];

    snprintf(path_buff, sizeof(path_buff), "%s%d", "/sys/class/gpio/gpio", pin);
    if (path_exists(path_buff))
    {
        return gpio_sysfs_write(pin, "/sys/class/gpio/unexport");
    }

    return 0;
}


int gpio_set_dir(int pin, const char dirc[])
{
    char path_buff[40];
    int fd;

    snprintf(path_buff, sizeof(path_buff), "%s%d%s", "/sys/class/gpio/gpio", pin, "/direction");
    fd = open(path_buff, O_WRONLY);
    if (fd == -1)
    {
        mctrl_err("Unable to open %s\n", path_buff);
        return -1;
    }
    if (write(fd, dirc , 3) != 3)
    {
        mctrl_err("Error writing to %s\n", path_buff);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int gpio_set_val(int pin, int val_int)
{
    char path_buff[40];
    char val[2];
    int fd;

    snprintf(path_buff, sizeof(path_buff), "%s%d%s", "/sys/class/gpio/gpio", pin, "/value");
    fd = open(path_buff, O_WRONLY);
    if (fd == -1)
    {
        mctrl_err("Unable to open %s\n", path_buff);
        return -1;
    }

    snprintf(val, sizeof(val), "%d", val_int);
    if (write(fd, val, 1) != 1)
    {
        mctrl_err("Error writing to %s\n", path_buff);
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int gpio_get_env(char env_var[])
{
    char* gpio = getenv(env_var);
    int tmp;

    if (gpio == NULL)
    {
        return -1;
    }
    else
    {
        if (str_to_int32(gpio, &tmp))
        {
            return -1;
        }

        return tmp;
    }
}

int path_exists(char path[])
{
    DIR* dir = opendir(path);

    if (dir)
    {
        closedir(dir);
        return 1;
    }

    return 0;
}
