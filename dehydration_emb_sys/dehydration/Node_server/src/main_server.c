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
// status led to confirm addr/name
// send a quick update of the ADCs

#include "main_server.h"

//***************************************************************************
// Static data
//***************************************************************************
volatile uint8_t state = 1;
static dehydration_server_t m_server;
// sensors states
static int8_t state_dehydration;
static int8_t state_ambient_humidity;
static int8_t state_ambient_temperature;
static int8_t state_skin_temperature;
static int8_t state_heart_rate;
//
static int state_msg;
static advertiser_t m_advertiser;
static uint8_t m_adv_buffer[ADVERTISER_BUFFER_SIZE];
static ble_gap_addr_t node_addr_p;
static int FLAG_sending_notif;
static dht22_data data_dht;


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
    temperature_buffer temp_buffer;
    temp_buffer.index_buffer = 0;

    state_msg          = 0;
    FLAG_sending_notif = 0;

    nodeHal_init(OUTS_MASK, OUTPUT_DIR);
    nodeHal_write(LED_DEBUG, LED_OFF);

    nodeHal_init(PIN_TEMP_VDD_MASK, OUTPUT_DIR);
    nodeHal_write(PIN_TEMP_VDD, STD_ON);

    // SENSOR
    nodeHal_init( PIN_DHT_VDD_MASK , OUTPUT_DIR);
    nodeHal_write(PIN_DHT_VDD, STD_ON);
    DHT22_Init();

    nodeHal_buttons_init(button_event_handler);

    __LOG_INIT(LOG_SRC_APP, LOG_LEVEL_INFO, log_callback_rtt);

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Bluetooth Mesh Beacon -----\n");

    static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
    static nrf_mesh_node_config_params_t config_params       = {
    .prov_caps = NRF_MESH_PROV_OOB_CAPS_DEFAULT(ACCESS_ELEMENT_COUNT)};

    config_params.p_static_data            = static_auth_data;
    config_params.complete_callback        = provisioning_complete;
    config_params.setup_callback           = configuration_setup;
    config_params.irq_priority             = NRF_MESH_IRQ_PRIORITY_LOWEST;
    config_params.lf_clk_cfg.source        = NRF_CLOCK_LF_SRC_SYNTH;
    config_params.lf_clk_cfg.xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_250_PPM;
    //    config_params.lf_clk_cfg.source        = NRF_CLOCK_LF_SRC_XTAL;
    //    config_params.lf_clk_cfg.xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM;

    ERROR_CHECK(nrf_mesh_node_config(&config_params));

    // Start listening for incoming packets
    nrf_mesh_rx_cb_set(rx_callback);

    // Start Advertising own beacon
    init_advertiser();

    // set the addr variable used to know if a message if for us
    set_addr();

    uint8_t* msg_str = "Here we are!";

    // advertiser_enable(&m_advertiser);
    start_advertiser(msg_str);

    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initialization complete!\n");

    // nodeHal_saadc_init(saadc_callback);
    nodeHal_saadc_init_simple();

    while (true) {
        (void)sd_app_evt_wait();

        get_ambient_temp_humid();
        get_temperature_mean(&temp_buffer);
        get_heart_rate();

        SEGGER_PRINTF("Ambient Temp  = %d \n\r", state_ambient_temperature);
        SEGGER_PRINTF("Ambient Humid = %d \n\r", state_ambient_humidity);
        SEGGER_PRINTF("Skin Temp     = %d \n\r", state_skin_temperature);
        SEGGER_PRINTF("Heart Rate    = %d \n\n\r", state_heart_rate);

        if (!FLAG_sending_notif) {
            change_ADC_data_advertiser();
        }
        nrf_delay_ms(1000);
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

static void rx_callback(const nrf_mesh_adv_packet_rx_data_t* p_rx_data) {
  int i        = 0;
  static int j = 0;
  char msg_addr[128];
  char msg[128];

  memset(msg_addr, 0, sizeof(msg_addr));
  (void)sprintf(msg_addr,
      "[%02x:%02x:%02x:%02x:%02x:%02x]",
      p_rx_data->p_metadata->params.scanner.adv_addr.addr[0],
      p_rx_data->p_metadata->params.scanner.adv_addr.addr[1],
      p_rx_data->p_metadata->params.scanner.adv_addr.addr[2],
      p_rx_data->p_metadata->params.scanner.adv_addr.addr[3],
      p_rx_data->p_metadata->params.scanner.adv_addr.addr[4],
      p_rx_data->p_metadata->params.scanner.adv_addr.addr[5]);

  memset(msg, 0, sizeof(msg));
  memcpy(msg, (uint8_t*)p_rx_data->p_payload, (uint8_t)p_rx_data->length);

  process_msg(&msg[2], p_rx_data->length - 1);
}

static void process_msg(uint8_t* msg_ptr, int msg_len) {
  char cmd_single;

  if (msg_ptr[0] == TO_ALL) {
    if (msg_ptr[1] == NOTIF_ALL) {
      SEGGER_PRINTF("%s", "process_notif_all()\n\r");
      process_notif_all();
    }
  } else if (msg_ptr[0] == TO_SINGLE) {
    if (compare_addr(&msg_ptr[1], BLE_GAP_ADDR_LEN)) {
      cmd_single = msg_ptr[CMD_CHAR_POS];
      if (cmd_single == NOTIF) {
        SEGGER_PRINTF("%s", "process_notif()\n\r");
        process_notif();
      } else if (cmd_single == DRINK_NOTIF) {
        SEGGER_PRINTF("%s", "process_notif_drink()\n\r");
        process_notif_drink();
      } else {
        SEGGER_PRINTF("Unknown command %c\n\r", msg_ptr[BLE_GAP_ADDR_LEN + 1]);
      }
    }
    // else not my address
  }
}

static void print_my_addr(uint8_t* msg_ptr) {
  int i;

  SEGGER_PRINTF("received addr : %02x%02x%02x%02x%02x%02x\n\r",
      *(msg_ptr + 1),
      *(msg_ptr + 2),
      *(msg_ptr + 3),
      *(msg_ptr + 4),
      *(msg_ptr + 5),
      *(msg_ptr + 6));

  SEGGER_PRINTF("My addr :       %s", "");

  for (i = 0; i < BLE_GAP_ADDR_LEN; i++) {
    SEGGER_PRINTF("%02x", node_addr_p.addr[i]);
  }
  SEGGER_PRINTF("\n\r%s", "");
}

static int compare_addr(uint8_t* addr_ptr, int addr_len) {
  int i;

  for (i = 0; i < BLE_GAP_ADDR_LEN; i++) {
    if (addr_ptr[i] != (node_addr_p.addr)[i]) {
      return 0;
    }
  }
  return 1;
}

static void set_addr(void) {
  advertiser_address_default_get(&node_addr_p);
}

/*8888b.   8888888888 88888888888            d8888 8888888b.  888     888
d88P  Y88b 888            888               d88888 888  "Y88b 888     888
Y88b.      888            888              d88P888 888    888 888     888
"Y888b.    8888888        888             d88P 888 888    888 Y88b   d88P
   "Y88b.  888            888            d88P  888 888    888  Y88b d88P
     "888  888            888           d88P   888 888    888   Y88o88P
Y88b  d88P 888            888          d8888888888 888  .d88P    Y888P
"Y8888P"   8888888888     888         d88P     888 8888888P"      Y*/

static void mesh_send_notif(void) {
  static char adv_msg_notif[2];
  FLAG_sending_notif = 1;

  advertiser_interval_set(&m_advertiser, ADVERTISER_PERIOD_FAST);

  adv_msg_notif[0] = NOTIF_CMD;
  adv_msg_notif[1] = 1;    // to be implemented

  set_advertiser(adv_msg_notif, sizeof(adv_msg_notif), ADVERTISER_REPEAT_ONCE);

  FLAG_sending_notif = 0;
}

static void change_ADC_data_advertiser(void) {
  int t_h_diff;
  static char adv_msg_update[7];
  adv_msg_update[0] = UPDATE_CMD;
  adv_msg_update[1] = (int8_t)(state_dehydration);
  adv_msg_update[2] = (int8_t)(state_ambient_humidity);
  adv_msg_update[3] = (int8_t)(state_ambient_temperature);
  adv_msg_update[4] = (int8_t)(state_skin_temperature);
  adv_msg_update[5] = (int8_t)(state_heart_rate);
  adv_msg_update[6] = 0;

  advertiser_interval_set(&m_advertiser, ADVERTISER_PERIOD_FAST);

  set_advertiser(adv_msg_update, sizeof(adv_msg_update), ADVERTISER_REPEAT_ONCE);
}

static void set_advertiser(char* msg_str, int len_msg, int repeat) {
  static uint8_t adv_data[128];

  memset(adv_data, 0, sizeof(adv_data));
  adv_data[0] = len_msg + 1;
  adv_data[1] = 0x2A;
  memcpy(&adv_data[2], msg_str, len_msg);

  /* Allocate packet */
  adv_packet_t* p_packet = advertiser_packet_alloc(&m_advertiser, len_msg + 2);

  if (p_packet) {
    /* Construct packet contents */
    memcpy(p_packet->packet.payload, adv_data, len_msg + 2);

    /* Repeat forever */
    p_packet->config.repeats = repeat;

    advertiser_packet_send(&m_advertiser, p_packet);
  }
}

static void init_advertiser(void) {
  advertiser_instance_dehydration_init(
      &m_advertiser, NULL, m_adv_buffer, ADVERTISER_BUFFER_SIZE, ADVERTISER_PERIOD_SLOW);
}

static void start_advertiser(uint8_t* msg_str) {
  advertiser_enable(&m_advertiser);

  static uint8_t adv_data[128];

  memset(adv_data, 0, sizeof(adv_data));
  adv_data[0] = ((uint8_t)strlen(msg_str) + 1);
  adv_data[1] = 0x2A;
  strncpy(&adv_data[2], msg_str, strlen(msg_str));

  /* Allocate packet */
  adv_packet_t* p_packet = advertiser_packet_alloc(&m_advertiser, strlen(msg_str) + 2);
  if (p_packet) {
    /* Construct packet contents */
    memcpy(p_packet->packet.payload, adv_data, strlen(msg_str) + 2);
    /* Repeat forever */
    p_packet->config.repeats = ADVERTISER_REPEAT_INFINITE;
    advertiser_packet_send(&m_advertiser, p_packet);
  }
}

static void configuration_setup(void* p_unused) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
  // Add model initialization here, if you wish to support
  // a mesh model on this node.
  nodeHal_blink(LED_STATUS, 200, 4);
}

static void provisioning_complete(void* p_unused) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Successfully provisioned\n");
  nodeHal_blink(LED_STATUS, 200, 4);
}

/*8888b.          d8888        d8888 8888888b.   .d8888b.
d88P  Y88b       d88888       d88888 888  "Y88b d88P  Y88b
Y88b.           d88P888      d88P888 888    888 888    888
"Y888b.        d88P 888     d88P 888 888    888 888
   "Y88b.     d88P  888    d88P  888 888    888 888
     "888    d88P   888   d88P   888 888    888 888    888
Y88b  d88P  d8888888888  d8888888888 888  .d88P Y88b  d88P
"Y8888P"   d88P     888 d88P     888 8888888P"   "Y8888*/

// void saadc_callback(nrf_drv_saadc_evt_t const* p_event) {
//   if (p_event->type == NRF_DRV_SAADC_EVT_DONE) {
//     ret_code_t err_code;

//     err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
//     ERROR_CHECK(err_code);
//     SEGGER_PRINTF("SAADC = %d \n\r", p_event->data.done.p_buffer[0]);
//   }
//   if (!FLAG_sending_notif) {
//     change_ADC_data_advertiser();
//   }
// }

void get_temperature_mean(temperature_buffer* temp_buff)
{
    char temp_str[32];
    int16_t temp_result = 0;
    int16_t temp_mean_result = 0;
    int index = 0;
    int i = 0;

    float temperature_f;
    int   temperature_i;

    index = temp_buff->index_buffer;

    nodeHal_saadc_sample_simple(NRF_SAADC_INPUT_AIN4, (int16_t *) &temp_result);
    // nodeHal_saadc_sample_simple(SAADC_CH_PSELP_PSELP_VDD, (int16_t *) &temp_result);

    temp_buff->buffer[index] = temp_result;
    index = (index >= TEMP_BUFFER_SIZE) ? 0 : index + 1;

    temp_buff->index_buffer = index;

    for (i=0; i < TEMP_BUFFER_SIZE; i++){
        temp_mean_result += temp_buff->buffer[i];
    }
    temp_mean_result = temp_mean_result / TEMP_BUFFER_SIZE;

    temperature_f = (((float) temp_mean_result /4563) - 0.500)*100;
    temperature_i = (int) temperature_f;

    state_skin_temperature = 0x00FF & temperature_i;

    // sprintf(temp_str, "%f", temperature_f);
    // SEGGER_PRINTF("temperature_f = %s \n\r", temp_str);
    // SEGGER_PRINTF("temperature_i = %d \n\r", temperature_i);
}

void get_heart_rate(void)
{
    int16_t heart_buff;
    nodeHal_saadc_sample_simple(NRF_SAADC_INPUT_AIN5, (int16_t *) &heart_buff);
    state_heart_rate = 0x00FF & (heart_buff / 120);
}

void get_ambient_temp_humid(void)
{
    char humidity[32];
    char temperature[32];
    bool err_dht;
    ret_code_t err_code;

    err_dht = DHT22_Read (&data_dht);
    if (!err_dht) {
        SEGGER_PRINTF("DHT status = %d \n\r", err_dht);
    }
    else {
        sprintf(temperature, "%f", data_dht.temperature);
        sprintf(humidity,    "%f", data_dht.humidity);
        state_dehydration = 0;
        state_ambient_humidity    = 0x000000FF & (int) data_dht.humidity;
        state_ambient_temperature = 0x000000FF & (int) data_dht.temperature;
    }
}


/*8888b.           888    888        d8888 888b    888 8888888b.
888  "88b          888    888       d88888 8888b   888 888  "Y88b
888  .88P          888    888      d88P888 88888b  888 888    888
8888888K.          8888888888     d88P 888 888Y88b 888 888    888
888  "Y88b         888    888    d88P  888 888 Y88b888 888    888
888    888         888    888   d88P   888 888  Y88888 888    888
888   d88P         888    888  d8888888888 888   Y8888 888  .d88P
8888888P" 88888888 888    888 d88P     888 888    Y888 8888888*/

static void button_event_handler(void) {
  uint32_t nrf_status = NRF_SUCCESS;
  SEGGER_PRINTF("Button pressed %s\r\n", "");
  mesh_send_notif();
  nodeHal_blink(LED_STATUS, 50, 5);
  nodeHal_notif(MOT_NOTIF, 600, 1);
}

static void process_notif_all(void) {
  nodeHal_blink(LED_STATUS, 100, 5);
  nodeHal_notif(MOT_NOTIF, 600, 3);
}

static void process_notif(void) {
  nodeHal_blink(LED_STATUS, 100, 5);
  nodeHal_notif(MOT_NOTIF, 100, 10);
}

static void process_notif_drink(void) {
  nodeHal_blink(LED_STATUS, 100, 5);
  nodeHal_notif(MOT_NOTIF, 300, 4);
}
