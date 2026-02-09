#pragma once

#include "os.h"

#define BUTTON_UP_IO_CLOSE BIT0
#define BUTTON_UP_IO_OPEN BIT1
#define BUTTON_DOWN_IO_CLOSE BIT2
#define BUTTON_DOWN_IO_OPEN BIT3
#define BUTTON_SELECT_IO_CLOSE BIT4
#define BUTTON_SELECT_IO_OPEN BIT5
#define INPUT_0_IO_CLOSE BIT6
#define INPUT_0_IO_OPEN BIT7
#define INPUT_1_IO_CLOSE BIT8
#define INPUT_1_IO_OPEN BIT9
#define INPUT_2_IO_CLOSE BIT10
#define INPUT_2_IO_OPEN BIT11
#define INPUT_3_IO_CLOSE BIT12
#define INPUT_3_IO_OPEN BIT13

#define BUTTONS_EVENTS_BITS                                                                        \
    (BUTTON_UP_IO_CLOSE | BUTTON_UP_IO_OPEN | BUTTON_DOWN_IO_CLOSE | BUTTON_DOWN_IO_OPEN           \
     | BUTTON_SELECT_IO_CLOSE | BUTTON_SELECT_IO_OPEN)

#define INPUTS_EVENTS_BITS                                                                         \
    (INPUT_0_IO_CLOSE | INPUT_0_IO_OPEN | INPUT_1_IO_CLOSE | INPUT_1_IO_OPEN | INPUT_2_IO_CLOSE    \
     | INPUT_2_IO_OPEN | INPUT_3_IO_CLOSE | INPUT_3_IO_OPEN)

#define TOUCHSCREEN_EVENTS_BITS 0

#define GPIO_ALL_EVENTS_BITS (BUTTONS_EVENTS_BITS | INPUTS_EVENTS_BITS)

typedef enum { OUTPUT_0 = 0x01, OUTPUT_1 = 0x02, OUTPUT_2 = 0x03, OUTPUT_3 = 0x04 } gpio_output;
typedef enum { INPUT_0 = 0x01, INPUT_1 = 0x02, INPUT_2 = 0x03, INPUT_3 = 0x04 } gpio_input;
typedef enum { VBAT = 0x01, CHAN_0 = 0x02, CHAN_1 = 0x03, CHAN_2 = 0x04 } adc_input;

event_t gpio_init();
bool get_digital_output_value(gpio_output gpio);
void set_digital_output_value(gpio_output gpio, bool value);

int get_adc_value(adc_input adc);

bool get_digital_input_value(gpio_input gpio);
bool maintenance_button_pressed();