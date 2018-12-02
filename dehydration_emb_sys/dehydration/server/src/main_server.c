/*888888888  .d88888b.  8888888b.   .d88888b.
    888     d88P" "Y88b 888  "Y88b d88P" "Y88b
    888     888     888 888    888 888     888
    888     888     888 888    888 888     888     d8b
    888     888     888 888    888 888     888     Y8P
    888     888     888 888    888 888     888
    888     Y88b. .d88P 888  .d88P Y88b. .d88P     d8b
    888      "Y88888P"  8888888P"   "Y88888P"      Y*/

// on all address
// set advertising time
// notif all
//
// on this address
// drink led/buzzer
// status led to confirm addr/name
// send a quick update of the ADCs

#include "main_server.h"


//***************************************************************************
// Static data
//***************************************************************************
volatile uint8_t             state = 1;
static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(2);
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;
static dehydration_server_t  m_server;
static int16_t               Dehydration_state;
static int16_t               Humidity_state;
static int16_t               Temperature_state;
static int                   state_msg;
static                       advertiser_t m_advertiser;
static uint8_t               m_adv_buffer[ADVERTISER_BUFFER_SIZE];
static ble_gap_addr_t        node_addr_p;

static int FLAG_sending_notif;
static int FLAG_in_notif_funct;

/*8b     d888        d8888 8888888 888b    888
8888b   d8888       d88888   888   8888b   888
88888b.d88888      d88P888   888   88888b  888
888Y88888P888     d88P 888   888   888Y88b 888
888 Y888P 888    d88P  888   888   888 Y88b888
888  Y8P  888   d88P   888   888   888  Y88888
888   "   888  d8888888888   888   888   Y8888
888       888 d88P     888 8888888 888    Y8*/

int main(void)
{
    state_msg = 0;
    FLAG_sending_notif = 0;

    nrf_gpio_range_cfg_output(LED_START, LED_STOP);
    for (uint32_t i = LED_START; i <= LED_STOP; ++i)
    {
        nrf_gpio_pin_set(i);
    }

//    while (true)
//    {
//        int i;
//        for (i = 0; i < 5; i++)
//        {
//          LEDS_ON(BSP_LED_0_MASK);
//          LEDS_ON(BSP_LED_1_MASK);
//          nrf_delay_ms(500);
//          LEDS_OFF(BSP_LED_0_MASK);
//          LEDS_OFF(BSP_LED_1_MASK);
//          nrf_delay_ms(500);
//        }
//        (void)sd_app_evt_wait();
//    }
    ERROR_CHECK(hal_buttons_init(button_event_handler));

    __LOG_INIT(LOG_SRC_APP, LOG_LEVEL_INFO, log_callback_rtt);
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Bluetooth Mesh Beacon -----\n");

    static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE]=STATIC_AUTH_DATA;
    static nrf_mesh_node_config_params_t config_params =
        {.prov_caps = NRF_MESH_PROV_OOB_CAPS_DEFAULT(ACCESS_ELEMENT_COUNT)};

    config_params.p_static_data            = static_auth_data;
    config_params.complete_callback        = provisioning_complete;
    config_params.setup_callback           = configuration_setup;
    config_params.irq_priority             = NRF_MESH_IRQ_PRIORITY_LOWEST;
    config_params.lf_clk_cfg.source        = NRF_CLOCK_LF_SRC_XTAL;
    config_params.lf_clk_cfg.xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM;

    ERROR_CHECK(nrf_mesh_node_config(&config_params));

    // Start listening for incoming packets
    nrf_mesh_rx_cb_set(rx_callback);
    // Start Advertising own beacon
    init_advertiser();
    // set the addr variable used to know if a message if for us
    set_addr();

    uint8_t * msg_str = "Here we are!";

    // advertiser_enable(&m_advertiser);
     start_advertiser(msg_str);

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initialization complete!\n");

    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();

    while (true)
    {
        (void) sd_app_evt_wait();
    }
}


/*8b     d888 8888888888  .d8888b.  888    888     8888888b.  Y88b   d88P
8888b   d8888 888        d88P  Y88b 888    888     888   Y88b  Y88b d88P
88888b.d88888 888        Y88b.      888    888     888    888   Y88o88P
888Y88888P888 8888888     "Y888b.   8888888888     888   d88P    Y888P
888 Y888P 888 888            "Y88b. 888    888     8888888P"     d888b
888  Y8P  888 888              "888 888    888     888 T88b     d88888b
888   "   888 888        Y88b  d88P 888    888     888  T88b   d88P Y88b
888       888 8888888888  "Y8888P"  888    888     888   T88b d88P   Y8*/

static void rx_callback(const nrf_mesh_adv_packet_rx_data_t * p_rx_data)
{
    LEDS_OFF(BSP_LED_0_MASK|BSP_LED_1_MASK|BSP_LED_2_MASK|BSP_LED_3_MASK);

    int i = 0;
    static int j = 0;
    char msg_addr[128];
    char msg[128];

    memset(msg_addr, 0, sizeof(msg_addr));
    (void) sprintf(msg_addr, "[%02x:%02x:%02x:%02x:%02x:%02x]",
                   p_rx_data->p_metadata->params.scanner.adv_addr.addr[0],
                   p_rx_data->p_metadata->params.scanner.adv_addr.addr[1],
                   p_rx_data->p_metadata->params.scanner.adv_addr.addr[2],
                   p_rx_data->p_metadata->params.scanner.adv_addr.addr[3],
                   p_rx_data->p_metadata->params.scanner.adv_addr.addr[4],
                   p_rx_data->p_metadata->params.scanner.adv_addr.addr[5]);

    memset(msg, 0, sizeof(msg));
    memcpy(msg,
            (uint8_t*) p_rx_data->p_payload,
            (uint8_t)  p_rx_data->length);


    // SEGGER_PRINTF("\n\n\rNew Message ADDR %s", msg_addr);
    // SEGGER_PRINTF("\n\r%d : \"", j++);
    // for (i=2; i<p_rx_data->length; i++)
    //     SEGGER_PRINTF("%c", p_rx_data->p_payload[i]);
    // SEGGER_PRINTF("\"\n\r%s", "");

    process_msg (&msg[2], p_rx_data->length-1);
}

static void process_msg (uint8_t * msg_ptr, int msg_len)
{
    char cmd_single;

    if (msg_ptr[0] == TO_ALL)
    {
        if (msg_ptr[1] == NOTIF_ALL)
        {
            SEGGER_PRINTF("%s", "process_notif_all()\n\r");
            process_notif_all();
        }
    }
    else if (msg_ptr[0] == TO_SINGLE)
    {
        if (compare_addr(&msg_ptr[1], BLE_GAP_ADDR_LEN))
        {
            cmd_single = msg_ptr[CMD_CHAR_POS];
            if (cmd_single == NOTIF)
            {
                SEGGER_PRINTF("%s", "process_notif()\n\r");
                process_notif();
            }
            else if (cmd_single == DRINK_NOTIF)
            {
                SEGGER_PRINTF("%s", "process_notif_drink()\n\r");
                process_notif_drink();
            }
            else
            {
                SEGGER_PRINTF("Unknown command %c\n\r",
                                                msg_ptr[BLE_GAP_ADDR_LEN+1]);
            }
        }
        else
        {
            ;
            // SEGGER_PRINTF("%s", "Not my address\n\r");
            // print_my_addr(msg_ptr);
        }
    }
}

static void print_my_addr(uint8_t * msg_ptr)
{
    int i;

    SEGGER_PRINTF("received addr : %02x%02x%02x%02x%02x%02x\n\r",
            *(msg_ptr+1), *(msg_ptr+2), *(msg_ptr+3),
            *(msg_ptr+4), *(msg_ptr+5), *(msg_ptr+6)   );

    SEGGER_PRINTF("My addr :       %s", "");

    for(i=0; i<BLE_GAP_ADDR_LEN; i++)
    {
        SEGGER_PRINTF("%02x", node_addr_p.addr[i]);
    }
    SEGGER_PRINTF("\n\r%s", "");
}


static int compare_addr(uint8_t* addr_ptr, int addr_len)
{
    int i;

    for(i=0 ; i < BLE_GAP_ADDR_LEN; i++)
    {
        if (addr_ptr[i] != (node_addr_p.addr)[i])
        {
            return 0;
        }
    }
    return 1;
}

static void process_notif_all(void)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        LEDS_ON(BSP_LED_0_MASK);
        LEDS_ON(BSP_LED_1_MASK);
        nrf_delay_ms(100);
        LEDS_OFF(BSP_LED_0_MASK);
        LEDS_OFF(BSP_LED_1_MASK);
        nrf_delay_ms(100);
    }
}

static void process_notif(void)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        LEDS_ON(BSP_LED_2_MASK);
        nrf_delay_ms(100);
        LEDS_OFF(BSP_LED_2_MASK);
        nrf_delay_ms(100);
    }
}

static void process_notif_drink(void)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        LEDS_ON(BSP_LED_3_MASK);
        nrf_delay_ms(100);
        LEDS_OFF(BSP_LED_3_MASK);
        nrf_delay_ms(100);
    }
}

static void set_addr(void)
{
    advertiser_address_default_get (&node_addr_p);
}


 /*8888b.  8888888888 88888888888            d8888 8888888b.  888     888
d88P  Y88b 888            888               d88888 888  "Y88b 888     888
Y88b.      888            888              d88P888 888    888 888     888
 "Y888b.   8888888        888             d88P 888 888    888 Y88b   d88P
    "Y88b. 888            888            d88P  888 888    888  Y88b d88P
      "888 888            888           d88P   888 888    888   Y88o88P
Y88b  d88P 888            888          d8888888888 888  .d88P    Y888P
 "Y8888P"  8888888888     888         d88P     888 8888888P"      Y*/

static void mesh_send_notif(void)
{
    static char adv_msg_notif[2];
    FLAG_sending_notif = 1;

    // advertiser_disable(&m_advertiser);
    advertiser_interval_set(&m_advertiser, ADVERTISER_PERIOD_FAST);
    // advertiser_enable(&m_advertiser);

    adv_msg_notif[0] = NOTIF_CMD;
    adv_msg_notif[1] = 1; // to be implemented

    set_advertiser(adv_msg_notif, sizeof(adv_msg_notif),
                                                    ADVERTISER_REPEAT_ONCE);

    FLAG_sending_notif = 0;
}

static void change_ADC_data_advertiser (void)
{
    int t_h_diff;
    static char adv_msg_update[7];
    adv_msg_update[0] = UPDATE_CMD;
    adv_msg_update[1] = (uint8_t) ( Dehydration_state >> 8) & 0x00FF;
    adv_msg_update[2] = (uint8_t) ( Dehydration_state)      & 0x00FF;
    adv_msg_update[3] = (uint8_t) ( Humidity_state    >> 8) & 0x00FF;
    adv_msg_update[4] = (uint8_t) ( Humidity_state)         & 0x00FF;
    adv_msg_update[5] = (uint8_t) ( Temperature_state >> 8) & 0x00FF;
    adv_msg_update[6] = (uint8_t) ( Temperature_state)      & 0x00FF;

    // SEGGER_PRINTF("%d : %d\n\r", m_advertiser.buf.head, m_advertiser.buf.tail);
    // t_h_diff = m_advertiser.buf.head - m_advertiser.buf.tail;
    // SEGGER_PRINTF("set adv\n\r", "");

    // advertiser_disable(&m_advertiser);
    advertiser_interval_set(&m_advertiser, ADVERTISER_PERIOD_FAST);
    // advertiser_enable(&m_advertiser);

    set_advertiser(adv_msg_update, sizeof(adv_msg_update),
                                         ADVERTISER_REPEAT_ONCE);

}

static void set_advertiser (char * msg_str, int len_msg, int repeat)
{
    static uint8_t adv_data[128];

    memset(adv_data, 0, sizeof(adv_data));
    adv_data[0] = len_msg + 1;
    adv_data[1] = 0x2A;
    memcpy(&adv_data[2], msg_str, len_msg);

    /* Allocate packet */
    adv_packet_t * p_packet = advertiser_packet_alloc(
                                            &m_advertiser,  len_msg + 2);

    if (p_packet)
    {
        /* Construct packet contents */
        memcpy(p_packet->packet.payload, adv_data,  len_msg + 2);

        /* Repeat forever */
        p_packet->config.repeats = repeat;

        advertiser_packet_send(&m_advertiser, p_packet);
    }
}

static void init_advertiser(void)
{
    advertiser_instance_dehydration_init(&m_advertiser, NULL, m_adv_buffer,
                               ADVERTISER_BUFFER_SIZE, ADVERTISER_PERIOD_SLOW);
}

static void start_advertiser(uint8_t * msg_str)
{
    advertiser_enable(&m_advertiser);

    static uint8_t adv_data[128];

    memset(adv_data, 0, sizeof(adv_data));
    adv_data[0] = ((uint8_t) strlen(msg_str)+1);
    adv_data[1] = 0x2A;
    strncpy(&adv_data[2], msg_str, strlen(msg_str));

    // Some Printing
    // SEGGER_PRINTF("\n\r\n\radv_data[0]=%d\n\r", adv_data[0]);
    // SEGGER_PRINTF("Adv_msg=\"%d %d %s\" len=%d \n\r\n\r",
    //    adv_data[0], adv_data[1], (uint8_t *) &adv_data[2], strlen(msg_str));

    /* Allocate packet */
    adv_packet_t * p_packet = advertiser_packet_alloc(
                                            &m_advertiser,  strlen(msg_str)+2);
    if (p_packet)
    {
        /* Construct packet contents */
        memcpy(p_packet->packet.payload, adv_data,  strlen(msg_str)+2);
        /* Repeat forever */
        p_packet->config.repeats = ADVERTISER_REPEAT_INFINITE;
        advertiser_packet_send(&m_advertiser, p_packet);
    }
}

static void configuration_setup(void * p_unused)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
    // Add model initialization here, if you wish to support
    // a mesh model on this node.
    hal_led_mask_set(LEDS_MASK, true);
}

static void provisioning_complete(void * p_unused)
{
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Successfully provisioned\n");
    hal_led_mask_set(LEDS_MASK, false);
    hal_led_blink_ms(LED_PIN_MASK, 200, 4);
}

/*888888888 8888888 888b     d888 8888888888 8888888b.
    888       888   8888b   d8888 888        888   Y88b
    888       888   88888b.d88888 888        888    888
    888       888   888Y88888P888 8888888    888   d88P
    888       888   888 Y888P 888 888        8888888P"
    888       888   888  Y8P  888 888        888 T88b
    888       888   888   "   888 888        888  T88b
    888     8888888 888       888 8888888888 888   T8*/

void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    ;
}

void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    // setup m_timer for compare event
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 100);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr =
        nrf_drv_timer_compare_event_address_get(
                                            &m_timer,NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    // setup timer compare event is triggering sample task in SAADC
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


 /*8888b.         d8888        d8888 8888888b.   .d8888b.
d88P  Y88b       d88888       d88888 888  "Y88b d88P  Y88b
Y88b.           d88P888      d88P888 888    888 888    888
 "Y888b.       d88P 888     d88P 888 888    888 888
    "Y88b.    d88P  888    d88P  888 888    888 888
      "888   d88P   888   d88P   888 888    888 888    888
Y88b  d88P  d8888888888  d8888888888 888  .d88P Y88b  d88P
 "Y8888P"  d88P     888 d88P     888 8888888P"   "Y8888*/

void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;

        err_code = nrf_drv_saadc_buffer_convert(
                            p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        ERROR_CHECK(err_code);

        int i = 0;

        Dehydration_state = p_event->data.done.p_buffer[i];
        Humidity_state    = Dehydration_state / 4;
        Temperature_state = Dehydration_state * 3;
    }
    if (!FLAG_sending_notif)
    {
        change_ADC_data_advertiser();
    }
}


void saadc_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config =
        NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(
                                        m_buffer_pool[0], SAMPLES_IN_BUFFER);
    ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(
                                        m_buffer_pool[1], SAMPLES_IN_BUFFER);
    ERROR_CHECK(err_code);

}


/*8888b.           888    888        d8888 888b    888 8888888b.
888  "88b          888    888       d88888 8888b   888 888  "Y88b
888  .88P          888    888      d88P888 88888b  888 888    888
8888888K.          8888888888     d88P 888 888Y88b 888 888    888
888  "Y88b         888    888    d88P  888 888 Y88b888 888    888
888    888         888    888   d88P   888 888  Y88888 888    888
888   d88P         888    888  d8888888888 888   Y8888 888  .d88P
8888888P" 88888888 888    888 d88P     888 888    Y888 8888888*/

static void button_event_handler(uint32_t button_number)
{
    uint32_t nrf_status = NRF_SUCCESS;
    SEGGER_PRINTF("Button %u pressed\r\n", button_number);
    mesh_send_notif();
    hal_led_blink_ms(LEDS_MASK, 50, 4);
}
