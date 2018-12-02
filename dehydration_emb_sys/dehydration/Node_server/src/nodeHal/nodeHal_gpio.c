#include "nodeHal_gpio.h"

static uint32_t m_last_button_press;
static hal_button_handler_cb_t m_button_handler_cb;

void nodeHal_write (uint32_t pin_nb, uint32_t  value)
{
   if (value)
      nrf_gpio_pin_write(pin_nb, 1);
   else
      nrf_gpio_pin_write(pin_nb, 0);
}


void nodeHal_init (uint32_t pin_mask, uint32_t direction)
{
    uint32_t pin;

    if (direction == OUTPUT_DIR){
        for (pin = 0; pin < 32; pin++)
            if ( (pin_mask) & (1 << pin) )
                nrf_gpio_cfg_output(pin);
    }
    else {
        for (pin = 0; pin < 32; pin++)
            if ( (pin_mask) & (1 << pin) )
                nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);
    }
}

void nodeHal_toggle(uint32_t pin_nb)
{
  nrf_gpio_pin_toggle(pin_nb);
}


void nodeHal_blink(uint32_t pin_nb, uint32_t delay_ms, uint32_t repeat)
{
    repeat &= 0xFE;

    repeat = repeat*2+2; // get the right amount of blinks

    for (uint32_t i = 0; i < repeat; ++i)
    {
        if (i & ((uint32_t) 1 << 0))
          nodeHal_write(pin_nb, 0);
        else
          nodeHal_write(pin_nb, 1);

        nrf_delay_ms(delay_ms);
    }
    nodeHal_write(pin_nb, 1);
}


void nodeHal_notif(uint32_t pin_nb, uint32_t delay_ms, uint32_t repeat)
{
    repeat &= 0xFE;

    repeat = repeat*2+2; // get the right amount of blinks

    for (uint32_t i = 0; i < repeat; ++i)
    {
        if (i & ((uint32_t) 1 << 0))
          nodeHal_write(pin_nb, 1);
        else
          nodeHal_write(pin_nb, 0);

        nrf_delay_ms(delay_ms);
    }
    nodeHal_write(pin_nb, 0);
}


uint32_t nodeHal_buttons_init(hal_button_handler_cb_t cb)
{
    uint32_t BUTTON_PIN_CONFIG =
       ((GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos)     |
        (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos)    |
        (BUTTON_PULL << GPIO_PIN_CNF_PULL_Pos)                 |
        (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
        (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos));

    if (cb == NULL)
        return NRF_ERROR_NULL;

    m_button_handler_cb = cb;

    NRF_GPIO->PIN_CNF[PIN_BUTTON] = BUTTON_PIN_CONFIG;

    NRF_GPIOTE->INTENSET = GPIOTE_INTENSET_PORT_Msk;
    NRF_GPIOTE->EVENTS_PORT  = 0;

    NVIC_SetPriority(GPIOTE_IRQn, GPIOTE_IRQ_LEVEL);
    NVIC_EnableIRQ(GPIOTE_IRQn);
    return NRF_SUCCESS;
}

/*****************************************************************************
 * IRQ handler(s)
 *****************************************************************************/


void GPIOTE_IRQHandler(void)
{
    NRF_GPIOTE->EVENTS_PORT = 0;

    SEGGER_RTT_printf(0, "RTC counter = %d\r\n", NRF_RTC0->COUNTER);

    if ((~NRF_GPIO->IN & (1 << (PIN_BUTTON))) &&
        TIMER_DIFF(m_last_button_press, NRF_RTC0->COUNTER) > 6200)
    {
        m_last_button_press = NRF_RTC0->COUNTER;
        m_button_handler_cb();
    }
}
