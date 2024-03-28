/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <console/console.h>
#include <drivers/gpio.h>

const struct device *gpio0;

typedef enum {	Entry,Exit,TIME_TICK,INCREMENT_TIME,RESET_TIME,START_PAUSE,EVENT_TRANISITON, EVENT_HANDLED}event_t;

typedef enum {	IDLE, SET_TIME,COUNTDOWN,PAUSE,ALARM}state_t;

typedef struct { uint8_t Signal;}GenericEvent_t;

typedef struct {GenericEvent_t UserSignal;uint8_t Value;}UserGenerated_Event_t;

typedef struct {GenericEvent_t SystemSignal;uint8_t SubSecond;}SystemGenerated_Event_t;

typedef struct {int CountDownValue;state_t CurrentState;}KitchenTimer_t;

static KitchenTimer_t Main_KitchenTimer;

static GenericEvent_t EventStatus;

void Initialize_StateMachine(void);

event_t StateMachine_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event);

event_t IDLE_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event);

event_t COUNTDOWN_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event);

event_t ALARM_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event);

event_t PAUSE_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event);

event_t SET_TIME_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event);

void Event_dispatcher(KitchenTimer_t * MainObject,GenericEvent_t * e);

  
event_t ALARM_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event){

	switch (Event->Signal)	{

	case INCREMENT_TIME:
	case RESET_TIME:
	case START_PAUSE:{

	printk("Alarm cleared\n");

	MainObject->CurrentState = IDLE;

	return EVENT_TRANISITON;

	}

	case Entry:{

	printk("Alarm State Entry- Count down expired\n");

    return EVENT_HANDLED;
		
	}
	
	case TIME_TICK:{

	if(((SystemGenerated_Event_t*)Event)->SubSecond == 10){

	printk("Alarm State Tick Event - New Alarm\n");

	}

    return EVENT_HANDLED;
		
					}
	}

return 1;

}


event_t COUNTDOWN_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event){

	switch (Event->Signal){

	case Entry:{

	printk("Countdown State - Entry event - Countdown time in seconds - %d\n",MainObject->CountDownValue);

	return EVENT_HANDLED;

	}
	case TIME_TICK:{

	if(((SystemGenerated_Event_t*)Event)->SubSecond == 10){

	MainObject->CountDownValue -=1;

	printk("Countdown State Tick event- Countdown time in seconds - %d\n",MainObject->CountDownValue);

	if(MainObject->CountDownValue ==0){

		MainObject->CurrentState = ALARM;

		return EVENT_TRANISITON;

	}

	}

	return EVENT_HANDLED;
		
	}

	case START_PAUSE:{

	printk("START_PAUSE event received in Countdown state\n");

	MainObject->CurrentState = PAUSE;

	return EVENT_TRANISITON;
		
	}
	}

return 1;

}


event_t PAUSE_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event){
	
	switch (Event->Signal){

	case Entry:{

	printk("Pause State - Entry - Countdown time in seconds - %d\n",MainObject->CountDownValue);

	return EVENT_HANDLED;

	}
	
	case RESET_TIME:{

	printk("Reset time event received in Pause state\n");

	MainObject->CountDownValue =0;

	MainObject->CurrentState = IDLE;

	return EVENT_TRANISITON;
	
	}
	
	case START_PAUSE:{

	printk("START_PAUSE event received in Pause state\n");

	MainObject->CurrentState = COUNTDOWN;

	return EVENT_TRANISITON;
		
	}
	
	case INCREMENT_TIME:{

	printk("Increment time event received in Pause state\n");

	MainObject->CountDownValue +=60;

	MainObject->CurrentState = SET_TIME;

	return EVENT_TRANISITON;

						}
	}

return 1;

}


event_t IDLE_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event){

	switch (Event->Signal){

	case Entry:{

		printk("Idle State - Entry - Countdown time in seconds - %d\n",MainObject->CountDownValue);

		return EVENT_HANDLED;

	}
	case INCREMENT_TIME:{

		printk("Increment time event received in Idle state\n");

		MainObject->CountDownValue +=60;

		MainObject->CurrentState = SET_TIME;

		return EVENT_TRANISITON;

						}
	}

return 1;

}


event_t SET_TIME_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event){

	switch (Event->Signal){

	case Entry:{

	printk("SET_TIME State - Countdown time in seconds - %d\n",MainObject->CountDownValue);

	return EVENT_HANDLED;

	}

	case INCREMENT_TIME:{

	if(MainObject->CountDownValue<(60*25)){

	MainObject->CountDownValue +=60;

	printk("Increment time event received in Set_Time state - Countdown time in seconds - %d\n",MainObject->CountDownValue);

	return EVENT_HANDLED;
		
		}

	return EVENT_HANDLED;
		
	}

	case RESET_TIME:{

	printk("Reset time event received in Set_Time state\n");

	MainObject->CountDownValue =0;

	MainObject->CurrentState = IDLE;

	return EVENT_TRANISITON;
		
	}

	case START_PAUSE:{

	printk("START_PAUSE event received in set_time state\n");

	MainObject->CurrentState = COUNTDOWN;

	return EVENT_TRANISITON;
		
					}

	}

return 1;

}

void Event_dispatcher(KitchenTimer_t * MainObject,GenericEvent_t  * DispatchEvent){
  
  event_t HandledStatus;

  state_t PreTransitionState,PostTransitionState;
  
  PreTransitionState = MainObject->CurrentState;
  
  HandledStatus = StateMachine_Handler(MainObject,DispatchEvent);

  if(HandledStatus == EVENT_TRANISITON){
  
    PostTransitionState = MainObject->CurrentState;
  
    GenericEvent_t TransitionEvent;
  
    //1. run the exit action for the PreTransitionState state
    TransitionEvent.Signal = Exit;
  
    MainObject->CurrentState = PreTransitionState;
  
    StateMachine_Handler(MainObject,&TransitionEvent);

    //2. run the entry action for the target state
    TransitionEvent.Signal = Entry;
  
    MainObject->CurrentState = PostTransitionState;
  
    StateMachine_Handler(MainObject,&TransitionEvent);
  
}

} 
void Initialize_StateMachine(void){

	Main_KitchenTimer.CountDownValue =(60*0);

	EventStatus.Signal = Entry;

	Main_KitchenTimer.CurrentState = IDLE;

	StateMachine_Handler(&Main_KitchenTimer,&EventStatus);

}


event_t StateMachine_Handler(KitchenTimer_t * MainObject, GenericEvent_t * Event){

switch (MainObject->CurrentState){

case IDLE:{

	return IDLE_Handler(MainObject,Event);

	//return EVENT_HANDLED;

}

case COUNTDOWN:{

	return COUNTDOWN_Handler(MainObject,Event);

	//return EVENT_HANDLED;

}

case ALARM:{

	return ALARM_Handler(MainObject,Event);

	//return EVENT_HANDLED;

}

case PAUSE:{

	return PAUSE_Handler(MainObject,Event);

	//return EVENT_HANDLED;

}

case SET_TIME:{

	return SET_TIME_Handler(MainObject,Event);

	//return EVENT_HANDLED;

}

}

return 1;

}



void main(void){

	//console_init();

	Initialize_StateMachine();

	int64_t Current_time = k_uptime_get();

	static SystemGenerated_Event_t SystemTick;

	static UserGenerated_Event_t UserActivity;

	gpio0 = device_get_binding("GPIO_0");

   	gpio_pin_configure(gpio0, 2, GPIO_INPUT);

   	gpio_pin_configure(gpio0, 3, GPIO_INPUT);

   	gpio_pin_configure(gpio0, 30, GPIO_INPUT);
	
	while (1) {
		
	if(k_uptime_get()-Current_time>=100){

	int button_state = gpio_pin_get(gpio0, 3);

	int button_state2 = gpio_pin_get(gpio0, 2);

	int button_state3 = gpio_pin_get(gpio0, 30);

	if((button_state2 !=1)|(button_state3 !=1)|(button_state !=1)){

	if(button_state2 !=1){

		UserActivity.UserSignal.Signal = INCREMENT_TIME;

	}

	if(button_state3 !=1){

	UserActivity.UserSignal.Signal = START_PAUSE;

	}

	if(button_state !=1){

	UserActivity.UserSignal.Signal = RESET_TIME;

	}

	Event_dispatcher(&Main_KitchenTimer,&UserActivity.UserSignal);

	}

	Current_time = k_uptime_get();

	SystemTick.SystemSignal.Signal = TIME_TICK;

	if(SystemTick.SubSecond>10){

		SystemTick.SubSecond=1;
	}
	else SystemTick.SubSecond++;

	// control reaches every 100 milli second

	Event_dispatcher(&Main_KitchenTimer,&SystemTick.SystemSignal);

										}
	}
}

