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

#pragma once

#include <stdint.h>

#include "transport.h"
#include "../utilities.h"

/**
 * @brief Read a 32bit register (or word aligned memory location).
 *
 * @param transport The transport structure.
 * @param addr      The address to read from.
 * @param data      Pointer to where to write the value.
 * @return          0 on success otherwise relevant error.
 */
int sdio_over_spi_read_reg_32bit(struct morsectrl_transport *transport,
                                 uint32_t addr,
                                 uint32_t *data);

/**
 * @brief Write a 32bit register (or word aligned memory location).
 *
 * @param transport The transport structure.
 * @param addr      The address to write to.
 * @param data      Value to write.
 * @return          0 on success otherwise relevant error.
 */
int sdio_over_spi_write_reg_32bit(struct morsectrl_transport *transport,
                                  uint32_t addr,
                                  uint32_t data);

/**
 * @brief Read a block of word aligned memory.
 *
 * @param transport The transport structure.
 * @param buff      Buffer to read memory into.
 * @param addr      Address to read from.
 * @return          0 on success otherwise relevant error.
 */
int sdio_over_spi_read_memblock(struct morsectrl_transport *transport,
                                struct morsectrl_transport_buff *buff,
                                uint32_t addr);

/**
 * @brief Write a block of word aligned memory.
 *
 * @param transport The transport structure.
 * @param buff      Buffer to write memory from.
 * @param addr      Address to write to.
 * @return          0 on success otherwise relevant error.
 */
int sdio_over_spi_write_memblock(struct morsectrl_transport *transport,
                                 struct morsectrl_transport_buff *buff,
                                 uint32_t addr);

/**
 * @brief Perform actions required after a hard reset (i.e. before firmware can be loaded).
 *
 * @param transport The transport structure.
 * @return          0 on success otherwise relevant error.
 */
int sdio_over_spi_post_hard_reset(struct morsectrl_transport *transport);
