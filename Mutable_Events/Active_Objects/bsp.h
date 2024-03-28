//============================================================================
// Product: DPP example
// Last Updated for Version: 7.3.0
// Date of the Last Update:  2023-06-27
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-QL-commercial
//
// This software is dual-licensed under the terms of the open source GNU
// General Public License version 3 (or any later version), or alternatively,
// under the terms of one of the closed source Quantum Leaps commercial
// licenses.
//
// The terms of the open source GNU General Public License version 3
// can be found at: <www.gnu.org/licenses/gpl-3.0>
//
// The terms of the closed source Quantum Leaps commercial licenses
// can be found at: <www.state-machine.com/licensing>
//
// Redistributions in source code must retain this top-level comment block.
// Plagiarizing this software to sidestep the license obligations is illegal.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#ifndef BSP_extra_H_
#define BSP_extra_H_

#include "app_uart.h"
#include "app_error.h"
#include "nrf_uarte.h"
#include "nrf_drv_gpiote.h"

#define BSP_TICKS_PER_SEC    100U

void BSP_init(void);
void BSP_start(void);
void BSP_terminate(int16_t result);
void BSP_Toggle(void);

#endif // BSP_extra_H_
