#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>
#include <string.h>

/* HAL */
#include "nrf.h"
//#include "nrf_sdm.h"
#include "boards.h"
#include "nrf_mesh_sdk.h"
#include "nrf_delay.h"

/* Core */
#include "nrf_mesh.h"
//#include "nrf_mesh_events.h"
//#include "nrf_mesh_prov.h"
//#include "nrf_mesh_assert.h"
#include "log.h"

#include "SEGGER_RTT.h"

#include "access.h"
#include "access_config.h"
#include "device_state_manager.h"

//#include "config_client.h"
#include "dehydration_client.h"

#include "simple_hal.h"
//#include "provisioner.h"

#include "rtt_input.h"
#include "gen_fifo.h"
#include "app_fifo.h"
#include "app_uart.h"
#include "nrf_uart.h"
#include "command_def.h"

/* For beaconing advertiser */
#include "advertiser.h"
#include "nrf_mesh_node_config.h"


//****************************************************************************
// Definitions
//****************************************************************************
#define BUTTON_NUMBER_GROUP      (3)
#define RTT_INPUT_POLL_PERIOD_MS (100)
#define UART_PRINTF              (printf)
#define UART_TX_BUF_SIZE         (256)  // UART TX buffer size.
#define UART_RX_BUF_SIZE         (256)  // UART RX buffer size.
#define UART_HWFC                (APP_UART_FLOW_CONTROL_DISABLED)
#define ADVERTISER_BUFFER_SIZE   (128)
#define LED_PIN_NUMBER           (BSP_LED_0)
#define LED_PIN_MASK             (1u << LED_PIN_NUMBER)
#define STATIC_AUTH_DATA         { 0xc7, 0xf7, 0x9b, 0xec, 0x9c, 0xf9, 0x74, \
                       0xdd, 0xb9, 0x62, 0xbd, 0x9f, 0xd1, 0x72, 0xdd, 0x73 }
#define SEGGER_PRINTF(X, ...)    SEGGER_RTT_printf(0, X, __VA_ARGS__)
#define PRE_UART_TX_BUF_SIZE     (8)
#define PRE_UART_TX_ELMT_SIZE    (sizeof(data_packet_t))

#define ADVERTISER_REPEAT_ONCE     0x01
#define ADVERTISER_REPEAT_FIVE     0x05
#define ADVERTISER_PERIOD          200  // in ms, in nrf_mesh_config_bearer.h


//***************************************************************************
// Static Declaration
//***************************************************************************
static void     button_event_handler   (uint32_t button_number);
static void     get_nodes              (void);
static void     set_advertiser         (char * msg_str, int len_msg);
static void     rx_callback            (const nrf_mesh_adv_packet_rx_data_t * p_rx_data);
static void     change_advertiser      (void);
static void     init_advertiser        (void);
static void     start_advertiser       (uint8_t * msg_str);
static void     configuration_setup    (void * p_unused);
static void     provisioning_complete  (void * p_unused);
static void     send_data_pc           (void);
static uint32_t uart_rx_check          (char * str_target);
static void     uart_init              (void);
static void     button_event_handler   (uint32_t button_number);
static command_return_t select_command (uint8_t * received_str);
// static uint32_t         process_message       (uint32_t key, uint32_t is_OP, uint32_t val, uint32_t server);

#endif


/*8b    888  .d88888b. 88888888888 8888888888  .d8888b.
8888b   888 d88P" "Y88b    888     888        d88P  Y88b
88888b  888 888     888    888     888        Y88b.
888Y88b 888 888     888    888     8888888     "Y888b.
888 Y88b888 888     888    888     888            "Y88b.
888  Y88888 888     888    888     888              "888
888   Y8888 Y88b. .d88P    888     888        Y88b  d88P
888    Y888  "Y88888P"     888     8888888888  "Y8888*/

// changes made in the SDK
/*
Two functions modified to have a parametrized advertising time intervall
advertiser.c (don't forget to change the .h as well)

static inline void set_advertiser_configuration_dehydration(advertiser_config_t * p_config,
                                            uint32_t time_intervalle)
{
    advertiser_address_default_get(&p_config->adv_addr);

    // p_config->advertisement_interval_us = MS_TO_US(BEARER_ADV_INT_DEFAULT_MS);
    p_config->advertisement_interval_us = MS_TO_US(time_intervalle);
    uint8_t channel_count;
    for (channel_count = 0;
         channel_count < BEARER_ADV_CHANNELS_MAX && channel_count < sizeof(m_ble_adv_channels) / sizeof(m_ble_adv_channels[0]);
         ++channel_count)
    {
        p_config->channels.channel_map[channel_count] = m_ble_adv_channels[channel_count];
    }
    p_config->channels.count = channel_count;
    p_config->channels.randomize_order = false;
}

void advertiser_instance_dehydration_init(advertiser_t * p_adv,
                              advertiser_tx_complete_cb_t tx_complete_cb,
                              uint8_t * p_buffer,
                              uint32_t buffer_size,
                              uint32_t time_intervalle)
{
    NRF_MESH_ASSERT(p_adv != NULL && p_buffer != NULL);
    NRF_MESH_ASSERT(buffer_size > (BLE_ADV_PACKET_MIN_LENGTH + sizeof(packet_buffer_packet_t)));
    packet_buffer_init(&p_adv->buf, p_buffer, buffer_size);
    set_advertiser_configuration_dehydration(&p_adv->config, time_intervalle);
    set_default_broadcast_configuration(&p_adv->broadcast);
    p_adv->broadcast.params.p_channels     = p_adv->config.channels.channel_map;
    p_adv->broadcast.params.channel_count  = p_adv->config.channels.count;
    p_adv->broadcast.params.tx_complete_cb = broadcast_complete_cb;
    p_adv->broadcast.params.p_packet       = NULL;
    p_adv->tx_complete_callback            = tx_complete_cb;
    p_adv->timer.cb                        = timeout_event;
    p_adv->timer.p_context                 = p_adv;
    p_adv->enabled                         = false;

    if (tx_complete_cb != NULL)
    {
        bearer_event_sequential_add(&p_adv->tx_complete_event, tx_complete_event_callback, p_adv);
    }
}
*/
