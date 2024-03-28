#include <stdbool.h>
#include <stdint.h>


#include "qpc.h"                 // QP/C real-time embedded framework
#include "Active_Objects/Blinky_AO.h"           
#include "Active_Objects/bsp.h"  // Board Support Package

int main() {

    QF_init();       // initialize the framework and the underlying RT kernel
    BSP_init();      // initialize the BSP
    BSP_start();     // start the AOs/Threads
    return QF_run(); // run the QF application
}
