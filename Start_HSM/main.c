#include <stdbool.h>
#include <stdint.h>
#include "qpc.h"                 // QP/C real-time embedded framework
#include "dpp.h"                 // DPP Application interface
#include "Active_Objects/bsp.h"                 // Board Support Package

int main() {

    QF_init();       // initialize the framework and the underlying RT kernel
    BSP_init();      // initialize the BSP
    BSP_start();     // start the AOs/Threads
    return QF_run(); // run the QF application
}


/*
#define input_button 18
nrf_gpio_cfg_input(input_button,NRF_GPIO_PIN_PULLUP);
if(nrf_gpio_pin_read(input_button) == 0) 
*/