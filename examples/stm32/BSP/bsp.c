/***************************************************************************
 * Copyright (c) 2025 SIMCom Wireless Ltd 
 * 
 * This program and the accompanying materials are made available under the
 * terms of the MIT License which is available at
 * https://opensource.org/licenses/MIT.
 * 
 * SPDX-License-Identifier: MIT
 **************************************************************************/

#include "bsp.h"

/**
 * @brief Hardware BSP init.
 * 
 */
void bsp_init() {
    bsp_uart_init();
}