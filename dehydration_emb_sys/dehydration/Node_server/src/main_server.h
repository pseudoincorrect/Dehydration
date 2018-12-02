#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdint.h>
#include <string.h>

/* HAL */
#include "node_board.h"
#include "nrf.h"
#include "nrf_mesh_sdk.h"
#include "nrf_delay.h"

/* Hal of custom board (node) */
#include "nodeHal_gpio.h"
//#include "nodeHal_rtc.h"
#include "nodeHal_saadc.h"
#include "nodeHal_oneWire.h"

/* Core */
#include "nrf_mesh.h"
#include "nrf_mesh_events.h"
#include "log.h"

#include "access.h"
#include "access_config.h"
#include "device_state_manager.h"
#include "nrf_mesh_node_config.h"

#include "dehydration_common.h"
#include "dehydration_server.h"
#include "command_def.h"

/* For beaconing advertiser */
#include "advertiser.h"
#include "ble.h"
#include "SEGGER_RTT.h"



//***************************************************************************
// Definitions
//***************************************************************************

#define LED_PIN_NUMBER         (BSP_LED_0)
#define LED_PIN_MASK           (1u << LED_PIN_NUMBER)
#define SAMPLES_IN_BUFFER      (1)
#define ADVERTISER_BUFFER_SIZE (128)
#define LED_PIN_NUMBER         (BSP_LED_0)
#define LED_PIN_MASK           (1u << LED_PIN_NUMBER)
#define STATIC_AUTH_DATA       { 0xc7, 0xf7, 0x9b, 0xec, 0x9c, 0xf9, 0x74, \
                     0xdd, 0xb9, 0x62, 0xbd, 0x9f, 0xd1, 0x72, 0xdd, 0x73 }
#define SEGGER_PRINTF(X, ...)  SEGGER_RTT_printf(0, X, __VA_ARGS__)
#define ADVERTISER_PERIOD_SLOW          2000  // in ms, in nrf_mesh_config_bearer.h
#define ADVERTISER_PERIOD_FAST          75  // in ms, in nrf_mesh_config_bearer.h

#define TEMP_BUFFER_SIZE 4
typedef struct
{
    int16_t buffer[TEMP_BUFFER_SIZE];
    int index_buffer;
} temperature_buffer;

//***************************************************************************
// Forward Declaration
//***************************************************************************
static uint32_t get_cb                      (const dehydration_server_t * p_server, uint32_t key, uint32_t val);
static uint32_t set_cb                      (const dehydration_server_t * p_server, uint32_t key, uint32_t val);
static void     rx_callback                 (const nrf_mesh_adv_packet_rx_data_t * p_rx_data);
static void     configuration_setup         (void * p_unused);
static void     init_advertiser             (void);
static void     start_advertiser            (uint8_t * msg_str);
static void     provisioning_complete       (void * p_unused);
static void     change_ADC_data_advertiser  (void);
static void     set_advertiser              (char* msg_str, int len_msg, int repeat);
void            timer_handler               (nrf_timer_event_t event_type, void * p_context);
void            saadc_sampling_event_init   (void);
void            saadc_sampling_event_enable (void);
void            saadc_callback              (nrf_drv_saadc_evt_t const * p_event);
void            saadc_init                  (void);
static void     process_msg                 (uint8_t * msg, int msg_len);
static void     process_notif_all           (void);
static void     process_notif               (void);
static void     process_notif_drink         (void);
static int      compare_addr                (uint8_t*  addr_ptr, int addr_len);
static void     set_addr                    (void);
static void     print_my_addr               (uint8_t * msg_ptr);
static void     button_event_handler        (void);
static void     mesh_send_notif             (void);
//static void     rtc_handler                 (nrf_drv_rtc_int_type_t int_type);
void            get_temperature_mean        (temperature_buffer* temp_buff);
void            get_heart_rate              (void);
void            get_ambient_temp_humid      (void);

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
