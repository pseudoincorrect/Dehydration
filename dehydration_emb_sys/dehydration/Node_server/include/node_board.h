#ifndef NODE_BOARD_H
#define NODE_BOARD_H

//#include "pca10040.h"

// Low frequency clock source to be used by the SoftDevice
//#define NRF_CLOCK_LFCLKSRC      {.source       = NRF_CLOCK_LF_SRC_XTAL,      \
//                                 .rc_ctiv      = 0,                          \
//                                 .rc_temp_ctiv = 0,                          \
//                                 .accuracy     = NRF_CLOCK_LF_ACCURACY_20_PPM}


//#define NRF_CLOCK_LFCLKSRC      {.source       = NRF_CLOCK_LF_SRC_RC,      \
//                                 .rc_ctiv      = 16,                          \
//                                 .rc_temp_ctiv = 0,                          \
//                                 .accuracy     = 0}

// outputs
#define LED_STATUS 12
#define PIN_LED_STATUS LED_STATUS
#define PIN_LED_STATUS_MASK (1 << PIN_LED_STATUS)

#define LED_DEBUG 18
#define PIN_LED_DEBUG LED_DEBUG
#define PIN_LED_DEBUG_MASK (1 << PIN_LED_DEBUG)

#define MOT_NOTIF 5
#define PIN_MOT_NOTIF MOT_NOTIF
#define PIN_MOT_NOTIF_MASK (1 << PIN_MOT_NOTIF)

#define OUTS_MASK (PIN_LED_STATUS_MASK | PIN_LED_DEBUG_MASK | PIN_MOT_NOTIF_MASK )

#define BUTTON 20
#define PIN_BUTTON BUTTON
#define PIN_BUTTON_MASK (1 << PIN_BUTTON)

#define DHT_VDD 3
#define PIN_DHT_VDD DHT_VDD
#define PIN_DHT_VDD_MASK (1 << PIN_DHT_VDD)

#define DHT_DATA 2
#define PIN_DHT_DATA DHT_DATA
#define PIN_DHT_DATA_MASK (1 << PIN_DHT_DATA)

#define TEMP_VDD 27
#define PIN_TEMP_VDD TEMP_VDD
#define PIN_TEMP_VDD_MASK (1 << PIN_TEMP_VDD)

#define TEMP_SKIN 28
#define PIN_TEMP_SKIN TEMP_SKIN
#define PIN_TEMP_SKIN_MASK (1 << PIN_TEMP_SKIN)
#define TEMP_SKIN_AIN  4

#define DHT22_TIMER   NRF_TIMER3

//#define OUT_ON(out_mask) do {  ASSERT(sizeof(out_mask) == 4);                 \
//                        NRF_GPIO->OUTSET = (out_mask) & (OUTS_MASK); } while (0)
//
//#define OUT_OFF(out_mask) do {  ASSERT(sizeof(out_mask) == 4);                \
//                       NRF_GPIO->OUTCLR = (out_mask) & (OUTS_MASK);} while (0)

//#define OUT_ON(out_mask) do { nrf_gpio_pin_write(out_mask, 1);  } while (0)
//
//#define OUT_OFF(out_mask) do { nrf_gpio_pin_write(out_mask, 0); } while (0)
//
//#define OUTS_INVERT(out_mask) do { uint32_t gpio_state = NRF_GPIO->OUT;      \
//                              ASSERT(sizeof(out_mask) == 4);                 \
//                              NRF_GPIO->OUTSET = ((out_mask) & ~gpio_state); \
//                              NRF_GPIO->OUTCLR = ((out_mask) & gpio_state); } while (0)
//
//#define OUTS_CONFIGURE(out_mask) do { uint32_t pin;                  \
//                                  ASSERT(sizeof(out_mask) == 4);     \
//                                  for (pin = 0; pin < 32; pin++)      \
//                                      if ( (out_mask) & (1 << pin) ) \
//                                          nrf_gpio_cfg_output(pin); } while (0)

#endif
