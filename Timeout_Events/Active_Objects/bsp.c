//============================================================================
// Product: DPP example, NUCLEO-C031C6 board, QV kernel
// Last updated for version 7.3.3
// Last updated on  2023-12-13
//
//                   Q u a n t u m  L e a P s
//                   ------------------------
//                   Modern Embedded Software
//
// Copyright (C) 2005 Quantum Leaps, LLC. <state-machine.com>
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"                 // QP/C real-time embedded framework
#include "Active_Objects/Blinky_AO.h" 
#include "bsp.h"                 // Board Support Package

#include <nrfx.h>
#include "nrf_gpio.h"

Q_DEFINE_THIS_FILE  // define the name of this file for assertions

// Local-scope objects -----------------------------------------------------
static uint32_t l_rndSeed;
#define led_1 11
#define input_button 18

static volatile uint32_t counter_10_ms;
static volatile uint32_t counter_ms;
static void uart_init(void);


#ifdef Q_SPY

    QSTimeCtr QS_tickTime_;
    QSTimeCtr QS_tickPeriod_;
    // QSpy source IDs
    static QSpyId const l_SysTick_Handler = { 0U };

#endif

//============================================================================
// Error handler and ISRs...

Q_NORETURN Q_onError(char const * const module, int_t const id) {
    // NOTE: this implementation of the assertion handler is intended only
    // for debugging and MUST be changed for deployment of the application
    // (assuming that you ship your production code with assertions enabled).
    Q_UNUSED_PAR(module);
    Q_UNUSED_PAR(id);
    QS_ASSERTION(module, id, 10000U);

#ifndef NDEBUG

    // for debugging, hang on in an endless loop...
    for (;;) {
    }
#endif

    NVIC_SystemReset();
}
//............................................................................
// assertion failure handler for the STM32 library, including the startup code
void assert_failed(char const * const module, int_t const id); // prototype
void assert_failed(char const * const module, int_t const id) {
    Q_onError(module, id);
}

//............................................................................
#ifdef __UVISION_VERSION
// dummy initialization of the ctors (not used in C)
void _init(void);
void _init(void) {
}
#endif // __UVISION_VERSION

// ISRs used in the application ============================================

void SysTick_Handler(void); // prototype
void SysTick_Handler(void) {

   //static QEvt const tickEvt = QEVT_INITIALIZER(TICK_SIG);
   //QACTIVE_PUBLISH(&tickEvt, &l_SysTick_Handler); // publish to all subscribers

    QTIMEEVT_TICK_X(0U, &l_SysTick_Handler); // time events at rate 0
    // Either use the preceding macro or assign an empty AO like in the game
    // example 

#ifdef Q_SPY
    QS_tickTime_ += QS_tickPeriod_; // account for the clock rollover
#endif

    QV_ARM_ERRATUM_838869();
}

void BSP_Toggle(void)
{
  nrf_gpio_pin_toggle(led_1);
}
//............................................................................
#ifdef QF_ON_CONTEXT_SW
// NOTE: the context-switch callback is called with interrupts DISABLED
void QF_onContextSw(QActive *prev, QActive *next) {
    QS_BEGIN_INCRIT(CONTEXT_SW, 0U) // in critical section!
        QS_OBJ(prev);
        QS_OBJ(next);
    QS_END_INCRIT()
}
#endif // QF_ON_CONTEXT_SW


//============================================================================
// BSP functions...

static bool tx_empty = true;

void uart_event_handler(app_uart_evt_t * p_event)
{

// create a varibale which will hold the characters that we are going to read over uart
  uint8_t c;


// check if some communication error occured, this event will be trigered
  if(p_event->evt_type == APP_UART_COMMUNICATION_ERROR)
  {
	// Check the reason of error by accessing the error variable and then pass it to error handler
    APP_ERROR_HANDLER(p_event->data.error_communication);
  }
  
// check if fifo error occured, this might be fifo overflow or other relevant errors  
  else if (p_event->evt_type == APP_UART_FIFO_ERROR)
  {
	// Check the reason of error by accessing the error variable and then pass it to error handler  
    APP_ERROR_HANDLER(p_event->data.error_code);
  }
// check if there is a character available to be read from the buffer
  else if (p_event->evt_type == APP_UART_DATA_READY)
  {
    app_uart_get(&c);
    QS_RX_PUT(c);
    QV_ARM_ERRATUM_838869();
  }

// check if the transmission over uart port is finished i.e. uart tx empty event was generated
  else if (p_event->evt_type == APP_UART_TX_EMPTY)
  {
	tx_empty = true;  
    
  }

}

#define input_button 18
#ifdef Q_SPY
    static uint8_t l_button_handler;
#endif

static void uart_init(void)
{
    uint32_t err_code;
    const app_uart_comm_params_t comm_params = { 8, 6, 7, 5, false, false, NRF_UARTE_BAUDRATE_115200 };
    APP_UART_FIFO_INIT(&comm_params, 2048, 2048, uart_event_handler, APP_IRQ_PRIORITY_LOWEST, err_code);
    APP_ERROR_CHECK(err_code);
}

static int counter = 0;
static void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)\
{
  //static QEvt const btn_pressed = QEVT_INITIALIZER(BTN_PRESSED_SIG);
  //QACTIVE_POST(Blinky_AO, &btn_pressed, &l_button_handler);
  //ButtonPressed_Evt * evt = Q_NEW(ButtonPressed_Evt,BTN_PRESSED_SIG);
  //evt->blink_frequency = counter++;
  //QACTIVE_POST(Blinky_AO, &evt->super, &l_button_handler);
}

void BSP_init(void) {

    ret_code_t error;
    error = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(error);
    nrf_drv_gpiote_in_config_t input_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    input_config.pull =  NRF_GPIO_PIN_PULLUP;
    error = nrf_drv_gpiote_in_init(input_button,&input_config,button_handler);
    APP_ERROR_CHECK(error);
        nrf_drv_gpiote_in_event_enable(input_button, true);
    nrf_gpio_cfg_output(led_1);

    // NOTE: SystemInit() has been already called from the startup code
    // but SystemCoreClock needs to be updated
    SystemCoreClockUpdate();

    // initialize the QS software tracing...
    if (!QS_INIT((void *)0)) {
        Q_ERROR();
    }

    uart_init();
    // dictionaries...
    QS_OBJ_DICTIONARY(&l_SysTick_Handler);
    QS_OBJ_DICTIONARY(&l_button_handler);

    //QS_ONLY(produce_sig_dict());

    // setup the QS filters...
    QS_GLB_FILTER(QS_SM_RECORDS);   // all records
    QS_GLB_FILTER(QS_AO_RECORDS);   // all records
    QS_GLB_FILTER(QS_EQ_RECORDS); 
    QS_GLB_FILTER(QS_MP_RECORDS); 
    //QS_GLB_FILTER(QS_TE_RECORDS); 
    QS_GLB_FILTER(QS_QF_RECORDS); 
    //QS_GLB_FILTER(QS_SC_RECORDS); 
    QS_GLB_FILTER(-QS_QF_TICK);      // exclude the clock tick

    
}
//............................................................................
void BSP_start(void) {

    static QEvt const *Blinky_Queue_Storage[10];
    Blinky_ctor();
    QACTIVE_START(Blinky_AO,
        2,                // QP prio. of the AO
        Blinky_Queue_Storage,               // event queue storage
        Q_DIM(Blinky_Queue_Storage),        // queue length [events]
        (void *)0, 0U,               // no stack storage
        (void *)0);                  // no initialization param
}

//............................................................................
void BSP_terminate(int16_t result) {
    Q_UNUSED_PAR(result);
}

//============================================================================
// QF callbacks...
void QF_onStartup(void) {
    // set up the SysTick timer to fire at BSP_TICKS_PER_SEC rate
    SysTick_Config(SystemCoreClock / BSP_TICKS_PER_SEC);

    // assign all priority bits for preemption-prio. and none to sub-prio.
    // NOTE: this might have been changed by STM32Cube.
    NVIC_SetPriorityGrouping(0U);

    // set priorities of ALL ISRs used in the system, see NOTE1
    NVIC_SetPriority(SysTick_IRQn,   QF_AWARE_ISR_CMSIS_PRI + 1U);

}
//............................................................................
void QF_onCleanup(void) {
}

//............................................................................
void QV_onIdle(void) { // CATION: called with interrupts DISABLED, see NOTE0

//nrf_gpio_pin_toggle(led_1);

#ifdef Q_SPY
    QF_INT_ENABLE();
    QS_rxParse();  // parse all the received bytes

    if (tx_empty == true) { // TXE empty?
        QF_INT_DISABLE();
        uint16_t b = QS_getByte();
        QF_INT_ENABLE();

        if (b != QS_EOD) {   // not End-Of-Data?
            app_uart_put(b);
            //tx_empty = false;
        }
    }
#elif defined NDEBUG
    // Put the CPU and peripherals to the low-power mode.
    // you might need to customize the clock management for your application,
    // see the datasheet for your particular Cortex-M MCU.
    QV_CPU_SLEEP(); // atomically go to sleep and enable interrupts
#else
    QF_INT_ENABLE(); // just enable interrupts
#endif
}

//============================================================================
// QS callbacks...
#ifdef Q_SPY

//............................................................................
static uint16_t const UARTPrescTable[12] = {
    1U, 2U, 4U, 6U, 8U, 10U, 12U, 16U, 32U, 64U, 128U, 256U
};

#define UART_DIV_SAMPLING16(__PCLK__, __BAUD__, __CLOCKPRESCALER__) \
  ((((__PCLK__)/UARTPrescTable[(__CLOCKPRESCALER__)]) \
  + ((__BAUD__)/2U)) / (__BAUD__))

#define UART_PRESCALER_DIV1  0U

// USART2 pins PA.2 and PA.3
#define USART2_TX_PIN 2U
#define USART2_RX_PIN 3U

//............................................................................
uint8_t QS_onStartup(void const *arg) {
    Q_UNUSED_PAR(arg);

    static uint8_t qsTxBuf[2*1024]; // buffer for QS-TX channel
    QS_initBuf(qsTxBuf, sizeof(qsTxBuf));

    static uint8_t qsRxBuf[100];    // buffer for QS-RX channel
    QS_rxInitBuf(qsRxBuf, sizeof(qsRxBuf));

    QS_tickPeriod_ = SystemCoreClock / BSP_TICKS_PER_SEC;
    QS_tickTime_ = QS_tickPeriod_; // to start the timestamp at zero

    return 1U; // return success
}
//............................................................................
void QS_onCleanup(void) {
}
//............................................................................
QSTimeCtr QS_onGetTime(void) { // NOTE: invoked with interrupts DISABLED
    if ((SysTick->CTRL & 0x00010000U) == 0U) { // not set?
        return QS_tickTime_ - (QSTimeCtr)SysTick->VAL;
    }
    else { // the rollover occurred, but the SysTick_ISR did not run yet
        return QS_tickTime_ + QS_tickPeriod_ - (QSTimeCtr)SysTick->VAL;
    }
}
//............................................................................
// NOTE:
// No critical section in QS_onFlush() to avoid nesting of critical sections
// in case QS_onFlush() is called from Q_onError().
void QS_onFlush(void) {
    for (;;) {
        uint16_t b = QS_getByte();
        if (b != QS_EOD) 
        {
          if (tx_empty == true)
           { // TXE empty?
            app_uart_put(b);
            //tx_empty = false;
            }
        }
        else {
            break;
        }
    }
}
//............................................................................
void QS_onReset(void) {
    NVIC_SystemReset();
}
//............................................................................
void QS_onCommand(uint8_t cmdId,
                  uint32_t param1, uint32_t param2, uint32_t param3)
{
    Q_UNUSED_PAR(cmdId);
    Q_UNUSED_PAR(param1);
    Q_UNUSED_PAR(param2);
    Q_UNUSED_PAR(param3);
}

#endif // Q_SPY
//----------------------------------------------------------------------------

//============================================================================
// NOTE0:
// The QV_onIdle() callback is called with interrupts disabled, because the
// determination of the idle condition might change by any interrupt posting
// an event. QV_onIdle() must internally enable interrupts, ideally
// atomically with putting the CPU to the power-saving mode.
//
// NOTE1:
// The QF_AWARE_ISR_CMSIS_PRI constant from the QF port specifies the highest
// ISR priority that is disabled by the QF framework. The value is suitable
// for the NVIC_SetPriority() CMSIS function.
//
// Only ISRs prioritized at or below the QF_AWARE_ISR_CMSIS_PRI level (i.e.,
// with the numerical values of priorities equal or higher than
// QF_AWARE_ISR_CMSIS_PRI) are allowed to call the QV_ISR_ENTRY/
// QV_ISR_ENTRY macros or any other QF/QV services. These ISRs are
// "QF-aware".
//
// Conversely, any ISRs prioritized above the QF_AWARE_ISR_CMSIS_PRI priority
// level (i.e., with the numerical values of priorities less than
// QF_AWARE_ISR_CMSIS_PRI) are never disabled and are not aware of the kernel.
// Such "QF-unaware" ISRs cannot call ANY QF/QV services. In particular they
// can NOT call the macros QV_ISR_ENTRY/QV_ISR_ENTRY. The only mechanism
// by which a "QF-unaware" ISR can communicate with the QF framework is by
// triggering a "QF-aware" ISR, which can post/publish events.
//
// NOTE2:
// The User LED is used to visualize the idle loop activity. The brightness
// of the LED is proportional to the frequency of the idle loop.
// Please note that the LED is toggled with interrupts locked, so no interrupt
// execution time contributes to the brightness of the User LED.
//
