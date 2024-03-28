/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <drivers/gpio.h>
#include "csrc/u8g2.h"
#include <drivers/spi.h>
#include <time.h>
#include <hal/nrf_gpio.h>
#include<stdio.h>
#include "qpn.h"
#include "ClockAlarm_SM.h"



extern u8g2_t u8g2; // a structure which will contain all the data for one display

Q_DEFINE_THIS_FILE;

uint8_t* SPiTxPointer ;
uint32_t milliSecondCounter;

void InitializeSPI(void);
void DisplayInit(void);

void ScanRotaryEncoder(void);

const struct device *gpio0;

uint8_t u8x8_byte_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,U8X8_UNUSED void *arg_ptr);

#define SPIM_SCK_PIN 26
#define SPIM_MOSI_PIN 27
#define SPIM_MISO_PIN 28
#define SPIM_CSN_PIN 29
#define DC_PIN 30
#define RESET_PIN 31
#define Rotary_Clk 2
#define Rotary_Data 3
#define Rotary_Button 12

int Counter;  
int CurrentState;
int LastState;

Q_NORETURN Q_onAssert(char_t const * const module, int_t const loc) {
    /*
    * NOTE: add here your application-specific error handling
    */
    (void)module;
    (void)loc;
}


void main(void){

  gpio0 = device_get_binding("GPIO_0");
  gpio_pin_configure(gpio0, 30, GPIO_OUTPUT);
  gpio_pin_configure(gpio0, 31, GPIO_OUTPUT);
  gpio_pin_configure(gpio0, Rotary_Clk, GPIO_INPUT);
  gpio_pin_configure(gpio0, Rotary_Data, GPIO_INPUT);
  gpio_pin_configure(gpio0, Rotary_Button, GPIO_INPUT | (GPIO_ACTIVE_HIGH | GPIO_PULL_UP));

  LastState = gpio_pin_get(gpio0,Rotary_Clk);

  InitializeSPI();

  DisplayInit();

  Clock_Alarm_Constructor();  
  QHSM_INIT(SuperClass_Qhsm_Clock);

  struct k_timer milliSecondInterrupt;
  k_timer_init(&milliSecondInterrupt,MilliSecondInterrupt_Cb,NULL);
  k_timer_start(&milliSecondInterrupt,K_USEC(1000),K_USEC(1000));

  while (1){

  ScanRotaryEncoder();

  if (milliSecondCounter >= 1000){
    Q_SIG(SuperClass_Qhsm_Clock) = TICK_SIG;
    QHSM_DISPATCH(SuperClass_Qhsm_Clock);
    milliSecondCounter = 0;
   }
   else if ( (milliSecondCounter%500) == 1)
   {
    Q_SIG(SuperClass_Qhsm_Clock) = ALARM_SIG;
    QHSM_DISPATCH(SuperClass_Qhsm_Clock);
   }
  }
}

void ScanRotaryEncoder(void){

  int Button = gpio_pin_get(gpio0,Rotary_Button);

  if (Button == 0){
    Q_SIG(SuperClass_Qhsm_Clock) = OK_SIG;
    QHSM_DISPATCH(SuperClass_Qhsm_Clock);
   }

  CurrentState =  gpio_pin_get(gpio0,Rotary_Clk);

  if(CurrentState != LastState){

    int TempState = gpio_pin_get(gpio0,Rotary_Data);

    if (TempState != CurrentState){
      Q_SIG(SuperClass_Qhsm_Clock) = SET_SIG; 
      QHSM_DISPATCH(SuperClass_Qhsm_Clock);
    }
     else {
      Q_SIG(SuperClass_Qhsm_Clock) = OK_SIG;
      QHSM_DISPATCH(SuperClass_Qhsm_Clock);
     }
  }
  LastState = CurrentState;
}

void DisplayInit(void){
	u8g2_Setup_st7565_zolen_128x64_1(&u8g2, U8G2_MIRROR, u8x8_byte_hw_spi, u8x8_gpio_and_delay);  
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0);
  u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
  u8g2_SetDrawColor(&u8g2,1);
}

void InitializeSPI(void){

    NRF_GPIO->OUTCLR = 1UL << SPIM_SCK_PIN;

    NRF_GPIO->PIN_CNF[SPIM_SCK_PIN] = (uint32_t) NRF_GPIO_PIN_DIR_OUTPUT << GPIO_PIN_CNF_DIR_Pos
            | (uint32_t) NRF_GPIO_PIN_INPUT_CONNECT << GPIO_PIN_CNF_INPUT_Pos
            | (uint32_t) NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos
            | (uint32_t) NRF_GPIO_PIN_S0S1 << GPIO_PIN_CNF_DRIVE_Pos
            | (uint32_t) NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos;
    
    NRF_SPIM1->PSEL.SCK = SPIM_SCK_PIN;

    NRF_GPIO->OUTCLR = 1UL << SPIM_MOSI_PIN;
    
    NRF_GPIO->PIN_CNF[SPIM_MOSI_PIN] =
            (uint32_t) NRF_GPIO_PIN_DIR_OUTPUT << GPIO_PIN_CNF_DIR_Pos
            | (uint32_t) NRF_GPIO_PIN_INPUT_DISCONNECT << GPIO_PIN_CNF_INPUT_Pos
            | (uint32_t) NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos
            | (uint32_t) NRF_GPIO_PIN_S0S1 << GPIO_PIN_CNF_DRIVE_Pos
            | (uint32_t) NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos;
    
    NRF_SPIM1->PSEL.MOSI = SPIM_MOSI_PIN;

    NRF_GPIO->PIN_CNF[SPIM_MISO_PIN] =
            (uint32_t) NRF_GPIO_PIN_DIR_INPUT << GPIO_PIN_CNF_DIR_Pos
            | (uint32_t) NRF_GPIO_PIN_INPUT_CONNECT << GPIO_PIN_CNF_INPUT_Pos
            | (uint32_t) NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos
            | (uint32_t) NRF_GPIO_PIN_S0S1 << GPIO_PIN_CNF_DRIVE_Pos
            | (uint32_t) NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos;
    
    NRF_SPIM1->PSEL.MISO = SPIM_MISO_PIN;

    NRF_GPIO->OUTSET = 1UL << SPIM_CSN_PIN;
    
    NRF_GPIO->PIN_CNF[SPIM_CSN_PIN] =
            (uint32_t) NRF_GPIO_PIN_DIR_OUTPUT << GPIO_PIN_CNF_DIR_Pos
            | (uint32_t) NRF_GPIO_PIN_INPUT_DISCONNECT << GPIO_PIN_CNF_INPUT_Pos
            | (uint32_t) NRF_GPIO_PIN_NOPULL << GPIO_PIN_CNF_PULL_Pos
            | (uint32_t) NRF_GPIO_PIN_S0S1 << GPIO_PIN_CNF_DRIVE_Pos
            | (uint32_t) NRF_GPIO_PIN_NOSENSE << GPIO_PIN_CNF_SENSE_Pos;

    NRF_SPIM1->CONFIG = 0; // CPOL = 0 - ActiveHigh, CPHA = 0 - Leading edge, ByteOrder = 0 - MSB  
    NRF_SPIM1->ORC = 0x55;
    NRF_SPIM1->FREQUENCY = SPIM_FREQUENCY_FREQUENCY_M8;
    NRF_SPIM1->ENABLE = SPIM_ENABLE_ENABLE_Enabled;

}


void SPI_TX_Data(uint8_t* data){

  SPiTxPointer = data;

  NRF_SPIM1->TXD.MAXCNT = 1;
  NRF_SPIM1->TXD.PTR = &SPiTxPointer;
  NRF_GPIO->OUTCLR = 1UL << SPIM_CSN_PIN;
  NRF_SPIM1->TASKS_START = 1;
	while(NRF_SPIM1->EVENTS_ENDTX != 1 ){

	}
  NRF_SPIM1->EVENTS_ENDTX = 0;
  NRF_SPIM1->TASKS_STOP = 1;
  NRF_GPIO->OUTSET = 1UL << SPIM_CSN_PIN;
}


uint8_t u8x8_byte_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {

  uint8_t *data;
  
  switch(msg) {
  
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 ) {
        SPI_TX_Data((uint8_t)*data);
        data++;
        arg_int--;
      }  
      break;
  
    case U8X8_MSG_BYTE_SET_DC:
      if(arg_int==0){
        gpio_pin_set(gpio0,DC_PIN,0);
      }else 
        gpio_pin_set(gpio0,DC_PIN,1);
      
      break;

    default:return 0;
  }  
  return 1;
}

uint8_t u8x8_gpio_and_delay(U8X8_UNUSED u8x8_t *u8x8,U8X8_UNUSED uint8_t msg, U8X8_UNUSED uint8_t arg_int,U8X8_UNUSED void *arg_ptr){

  switch (msg){

    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      k_msleep(1);
      break;
    
    case U8X8_MSG_DELAY_MILLI:
      k_msleep(arg_int);
      break;
    
    case U8X8_MSG_GPIO_DC:
      if(arg_int==0){
        gpio_pin_set(gpio0,DC_PIN,0);
      }else gpio_pin_set(gpio0,DC_PIN,1);
      
      break;
    
    case U8X8_MSG_GPIO_RESET:  
      if(arg_int==0){
        gpio_pin_set(gpio0,RESET_PIN,0);
      }else gpio_pin_set(gpio0,RESET_PIN,1);
      
      break;
  
  }
  return 1;
}


#if 0


void RefreshDisplay(char * Buffer,int64_t Custom){

  char m_str[3];

  strcpy(m_str, u8x8_u8toa(m, 2));		/* convert m to a string with two digits */

  u8g2_FirstPage(&u8g2);

  do {

    u8g2_SetFont(&u8g2,u8g2_font_logisoso62_tn);

    u8g2_DrawStr(&u8g2,0,63,"9");

    u8g2_DrawStr(&u8g2,33,63,":");

    u8g2_DrawStr(&u8g2,50,63,m_str);

  } while ( u8g2_NextPage(&u8g2) );

  k_msleep(1000);

  m++;

  if ( m == 60 )

    m = 0;

}

    //u8g2_DrawStr(&u8g2,2,16,"State - Clock");

    //u8g2_DrawStr(&u8g2,0+Offset,16+Offset,m_str);

    //u8g2_DrawStr(&u8g2,24+Offset,16+Offset,"-");

    //u8g2_DrawStr(&u8g2,30+Offset,16+Offset,m_str);

    //u8g2_DrawStr(&u8g2,54+Offset,16+Offset,"-");

    //u8g2_DrawStr(&u8g2,60+Offset,16+Offset,m_str);


#endif
