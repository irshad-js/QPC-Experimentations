//$file${Active_Objects::Blinky_AO.h} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: Mutable_Events.qm
// File:  ${Active_Objects::Blinky_AO.h}
//
// This code has been generated by QM 6.1.1 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${Active_Objects::Blinky_AO.h} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#ifndef BLINKY_AO
#define BLINKY_AO

#include "qpc.h"               // QP/C real-time embedded framework
#include "bsp.h"               // Board Support Package

//$declare${Shared} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Shared::Blinky_AO} .......................................................
extern QActive *const Blinky_AO;

//${Shared::APP_signals} .....................................................
enum APP_signals {
    TIMEOUT_SIG = Q_USER_SIG,
    BTN_PRESSED_SIG,
    MAX_SIG         // the last signal
};

//${Shared::Blinky_ctor} .....................................................
void Blinky_ctor(void);

//${Shared::ButtonPressed_Evt} ...............................................
typedef struct {
// protected:
    QEvt super;

// public:
    uint8_t blink_frequency;
} ButtonPressed_Evt;
//$enddecl${Shared} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#endif
