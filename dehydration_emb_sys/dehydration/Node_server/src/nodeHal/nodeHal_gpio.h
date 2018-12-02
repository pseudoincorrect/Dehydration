#ifndef MYHAL_GPIO_H
#define MYHAL_GPIO_H

#include <stdint.h>
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "node_board.h"
#include "timer.h"
#include "SEGGER_RTT.h"

#define LED_ON  0 // due to pull-up resistor  VDD => LED => PIN_LED
#define LED_OFF 1 // due to pull-up resistor  VDD => LED => PIN_LED
#define MOT_ON  1
#define MOT_OFF 0
#define STD_ON  1
#define STD_OFF 0
#define OUTPUT_DIR 1
#define INPUT_DIR 0

#define HAL_BUTTON_PRESS_FREQUENCY (6554) /* 200ms */
#define GPIOTE_IRQ_LEVEL (6)
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP
#define COMPARE_COUNTERTIME  (3UL) 

//***************************************************************************
// Typedef
//***************************************************************************
typedef void (*hal_button_handler_cb_t)(void);

//***************************************************************************
// Public functions
//***************************************************************************

void      nodeHal_init    (uint32_t pin_mask, uint32_t direction);
void      nodeHal_write   (uint32_t out_mask, uint32_t  value);
void      nodeHal_toggle  (uint32_t pin_nb);
void      nodeHal_blink   (uint32_t pin_nb, uint32_t delay_ms, uint32_t repeat);
void      nodeHal_notif   (uint32_t pin_nb, uint32_t delay_ms, uint32_t repeat);
uint32_t  nodeHal_buttons_init  (hal_button_handler_cb_t cb);
void      GPIOTE_IRQHandler   (void);


#endif