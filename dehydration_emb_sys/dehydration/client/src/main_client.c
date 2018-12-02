/*888888888  .d88888b.  8888888b.   .d88888b.
    888     d88P" "Y88b 888  "Y88b d88P" "Y88b
    888     888     888 888    888 888     888
    888     888     888 888    888 888     888
    888     888     888 888    888 888     888
    888     888     888 888    888 888     888
    888     Y88b. .d88P 888  .d88P Y88b. .d88P
    888      "Y88888P"  8888888P"   "Y88888*/

// Scheduler for the uart and other operations
// https://devzone.nordicsemi.com/tutorials/b/software-development-kit/posts/scheduler-tutorial

// Implement a way to send a notif to the client to enter the name of the
// server node

// Prepare the uart for the GUI

#include "main_client.h"

//****************************************************************************
// Static data
//****************************************************************************
static int selected_server;
static int selected_OP;
static int set_val;
static int state_msg;
static advertiser_t m_advertiser;
static uint8_t m_adv_buffer[ADVERTISER_BUFFER_SIZE];
static gen_fifo_t rx_fifo;
static data_packet_t rx_fifo_buf[PRE_UART_TX_BUF_SIZE];
static int str_complete;
static int32_t str_target_iter;

/*8b     d888        d8888 8888888 888b    888
8888b   d8888       d88888   888   8888b   888
88888b.d88888      d88P888   888   88888b  888
888Y88888P888     d88P 888   888   888Y88b 888
888 Y888P 888    d88P  888   888   888 Y88b888
888  Y8P  888   d88P   888   888   888  Y88888
888   "   888  d8888888888   888   888   Y8888
888       888 d88P     888 8888888 888    Y8*/

int main(void) {
  static uint8_t received_str[64];
  static command_return_t wait_set;
  state_msg       = 0;
  str_target_iter = 0;
  int ret;

  ERROR_CHECK(hal_buttons_init(button_event_handler));

  __LOG_INIT(LOG_SRC_APP, LOG_LEVEL_INFO, log_callback_rtt);
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "----- Bluetooth Mesh Beacon -----\n");

  static const uint8_t static_auth_data[NRF_MESH_KEY_SIZE] = STATIC_AUTH_DATA;
  static nrf_mesh_node_config_params_t config_params       = {
      .prov_caps = NRF_MESH_PROV_OOB_CAPS_DEFAULT(ACCESS_ELEMENT_COUNT)};

  config_params.p_static_data            = static_auth_data;
  config_params.complete_callback        = provisioning_complete;
  config_params.setup_callback           = configuration_setup;
  config_params.irq_priority             = NRF_MESH_IRQ_PRIORITY_LOWEST;
  config_params.lf_clk_cfg.source        = NRF_CLOCK_LF_SRC_XTAL;
  config_params.lf_clk_cfg.xtal_accuracy = NRF_CLOCK_LF_XTAL_ACCURACY_20_PPM;

  ERROR_CHECK(nrf_mesh_node_config(&config_params));

  /* Start listening for incoming packets */
  nrf_mesh_rx_cb_set(rx_callback);
  /* Start Advertising own beacon */
  init_advertiser();

  uint8_t *msg_str = "Here we are!";

  start_advertiser(msg_str);

  uart_init();

  ret = gen_fifo_init(&rx_fifo, rx_fifo_buf, PRE_UART_TX_BUF_SIZE, PRE_UART_TX_ELMT_SIZE, sizeof(rx_fifo_buf));
  ERROR_CHECK(ret);

  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initialization complete!\n");

  memset(received_str, 0, sizeof(received_str));
  str_complete = 0;

  while (true) {
    //__LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Waiting for a UART message\r\n");
    uart_rx_check(received_str);

    if (str_complete == 1) {
      wait_set = select_command(received_str);
      memset(received_str, 0, sizeof(received_str));
      str_complete = 0;
    }

    if (gen_fifo_length(&rx_fifo)) {
      send_data_pc();
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

static void rtt_input_handler(int key) {
  if (key >= '0' && key <= '3') {
    uint32_t button_number = key - '0';
    button_event_handler(button_number);
  }
}

/*8b     d888 8888888888  .d8888b.  888    888          8888888b.  Y88b   d88P
8888b   d8888 888        d88P  Y88b 888    888          888   Y88b  Y88b d88P
88888b.d88888 888        Y88b.      888    888          888    888   Y88o88P
888Y88888P888 8888888     "Y888b.   8888888888          888   d88P    Y888P
888 Y888P 888 888            "Y88b. 888    888          8888888P"     d888b
888  Y8P  888 888              "888 888    888          888 T88b     d88888b
888   "   888 888        Y88b  d88P 888    888          888  T88b   d88P Y88b
888       888 8888888888  "Y8888P"  888    888 88888888 888   T88b d88P   Y8*/

static void rx_callback(const nrf_mesh_adv_packet_rx_data_t *p_rx_data) {
  static data_packet_t data_to_fifo;
  char msg_str[128];
  char cmd_from_node;
  int i;
  int ret;

  // LEDS_OFF(BSP_LED_0_MASK | BSP_LED_1_MASK | BSP_LED_2_MASK | BSP_LED_3_MASK);

  memset(msg_str, 0, sizeof(msg_str));

  memcpy(msg_str, (uint8_t *)p_rx_data->p_metadata->params.scanner.adv_addr.addr, (uint8_t)D_ADDR_SIZE);

  memcpy(&msg_str[D_ADDR_SIZE], (uint8_t *)&((p_rx_data->p_payload)[2]), (uint8_t)((p_rx_data->length) - 2));

  // prepare data for the fifo
  for (i = 0; i < D_ADDR_SIZE; i++) {
    data_to_fifo.d_addr[i] = msg_str[i];
  }

  cmd_from_node          = msg_str[D_COMMAND_POS];
  data_to_fifo.d_command = cmd_from_node;

  if (cmd_from_node == UPDATE_CMD) {
    data_to_fifo.d_dehydration         = msg_str[D_DEHYDRATION];
    data_to_fifo.d_ambient_humidity    = msg_str[D_AMBIENT_HUMIDITY];
    data_to_fifo.d_ambient_temperature = msg_str[D_AMBIENT_TEMPERATURE];
    data_to_fifo.d_skin_temperature    = msg_str[D_SKIN_TEMPERATURE];
    data_to_fifo.d_heart_rate          = msg_str[D_HEART_RATE];
    data_to_fifo.d_dummy               = msg_str[D_DUMMY];
    // data_to_fifo.d_dehydration = ((msg_str[D_DEHYDRATION_MSB] << 8) | (msg_str[D_DEHYDRATION_LSB]));
    // data_to_fifo.d_humidity    = ((msg_str[D_HUMIDITY_MSB] << 8) | (msg_str[D_HUMIDITY_LSB]));
    // data_to_fifo.d_temperature = ((msg_str[D_TEMPERATURE_MSB] << 8) | (msg_str[D_TEMPERATURE_LSB]));
  } else if (cmd_from_node == NOTIF_CMD) {
    data_to_fifo.d_dehydration         = 0;
    data_to_fifo.d_ambient_humidity    = msg_str[D_NOTIF];
    data_to_fifo.d_ambient_temperature = 0;
    data_to_fifo.d_skin_temperature    = 0;
    data_to_fifo.d_heart_rate          = 0;
    data_to_fifo.d_dummy               = 0;
    // data_to_fifo.d_dehydration = msg_str[D_DEHYDRATION_MSB];
    // data_to_fifo.d_humidity    = 0;
    // data_to_fifo.d_temperature = 0;
  } else {
    SEGGER_PRINTF("Unknown mesh command\r\n", "");
  }

  ret = gen_fifo_write(&rx_fifo, &data_to_fifo);

  if (ret != NRF_SUCCESS) {
    SEGGER_PRINTF("%s\r\n", "write failed, flushing fifo");
    gen_fifo_flush(&rx_fifo);
  }
}

/*8888b.   8888888888 88888888888            d8888 8888888b.  888     888
d88P  Y88b 888            888               d88888 888  "Y88b 888     888
Y88b.      888            888              d88P888 888    888 888     888
"Y888b.    8888888        888             d88P 888 888    888 Y88b   d88P
   "Y88b.  888            888            d88P  888 888    888  Y88b d88P
     "888  888            888           d88P   888 888    888   Y88o88P
Y88b  d88P 888            888          d8888888888 888  .d88P    Y888P
"Y8888P"   8888888888     888         d88P     888 8888888P"      Y*/

static void change_advertiser(void) {
  char *M_message = "Here we are!";
  set_advertiser(M_message, strlen(M_message));
}

static void set_advertiser(char *msg_str, int len_msg) {
  static uint8_t adv_data[128];

  memset(adv_data, 0, sizeof(adv_data));
  adv_data[0] = len_msg + 1;
  adv_data[1] = 0x2A;
  memcpy(&adv_data[2], msg_str, len_msg);
  /* Allocate packet */
  adv_packet_t *p_packet = advertiser_packet_alloc(&m_advertiser, len_msg + 2);

  if (p_packet) {
    /* Construct packet contents */
    memcpy(p_packet->packet.payload, adv_data, len_msg + 2);
    /* Repeat forever */
    p_packet->config.repeats = ADVERTISER_REPEAT_ONCE;
    advertiser_packet_send(&m_advertiser, p_packet);
  }
}

static void init_advertiser(void) {
  advertiser_instance_dehydration_init(&m_advertiser, NULL, m_adv_buffer, ADVERTISER_BUFFER_SIZE, ADVERTISER_PERIOD);
}

static void start_advertiser(uint8_t *msg_str) {
  static uint8_t adv_data[128];

  advertiser_enable(&m_advertiser);

  memset(adv_data, 0, sizeof(adv_data));
  adv_data[0] = ((uint8_t)strlen(msg_str) + 1);
  adv_data[1] = 0x2A;
  strncpy(&adv_data[2], msg_str, strlen(msg_str));

  /* Allocate packet */
  adv_packet_t *p_packet = advertiser_packet_alloc(&m_advertiser, strlen(msg_str) + 2);

  if (p_packet) {
    /* Construct packet contents */
    memcpy(p_packet->packet.payload, adv_data, strlen(msg_str) + 2);

    /* Repeat forever */
    p_packet->config.repeats = ADVERTISER_REPEAT_ONCE;

    advertiser_packet_send(&m_advertiser, p_packet);
  }
}

static void configuration_setup(void *p_unused) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Initializing and adding models\n");
  // Add model initialization here, if you wish to support
  // a mesh model on this node.
  hal_led_mask_set(LEDS_MASK, true);
}

static void provisioning_complete(void *p_unused) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Successfully provisioned\n");
  hal_led_mask_set(LEDS_MASK, false);
  hal_led_blink_ms(LED_PIN_MASK, 200, 4);
}

/*8b     d888           .d8888b.  888b     d888 8888888b.
8888b   d8888          d88P  Y88b 8888b   d8888 888  "Y88b
88888b.d88888          888    888 88888b.d88888 888    888
888Y88888P888          888        888Y88888P888 888    888
888 Y888P 888          888        888 Y888P 888 888    888
888  Y8P  888          888    888 888  Y8P  888 888    888
888   "   888          Y88b  d88P 888   "   888 888  .d88P
888       888 88888888  "Y8888P"  888       888 8888888*/

void send_command_all(uint8_t *received_str) {
  static uint8_t adv_data[2];

  adv_data[0] = received_str[0];
  adv_data[1] = received_str[1];

  set_advertiser(adv_data, 2);
}

void send_command_node(uint8_t *received_str) {
  static uint8_t adv_data[9];
  char msg_addr_MSB[9];
  char msg_addr_LSB[5];
  char *ptr;
  unsigned int addr_l_MSB;
  unsigned int addr_l_LSB;

  // ex of received msg "s4734cb0b8fe3n"
  // copy the MSB of the addr to a temp str
  memcpy(msg_addr_MSB, &received_str[1], 8);
  msg_addr_MSB[8] = 0;    // set the end of string to the null value
  // copy the LSB of the addr to a temp str
  memcpy(msg_addr_LSB, &received_str[9], 4);
  msg_addr_LSB[5] = 0;    // set the end of string to the null value
  // convert the MSB/LSB string to double and place in the long buffer
  addr_l_MSB = strtoul(msg_addr_MSB, NULL, 16);
  addr_l_LSB = strtoul(msg_addr_LSB, NULL, 16);

  // filling the advertiser buffer
  // message type set to single node
  adv_data[0] = received_str[0];
  // copy the double value of the MSB addr to advertiser buffer
  adv_data[1] = addr_l_MSB >> 24;
  adv_data[2] = addr_l_MSB >> 16;
  adv_data[3] = addr_l_MSB >> 8;
  adv_data[4] = addr_l_MSB;
  // copy the double value of the LSB addr to advertiser buffer
  adv_data[5] = addr_l_LSB >> 8;
  adv_data[6] = addr_l_LSB;
  // set the command operand
  adv_data[7] = received_str[13];    // cmd
  // set the end of string to the null value
  adv_data[8] = 0;    // end of str

  // SEGGER_PRINTF("to mesh_send : %c%02x%02x%02x%02x%02x%02x%c size %d\n\r",
  SEGGER_PRINTF("to mesh_send : %c%02x%02x%02x%02x%02x%02x%c size %d\n\r",
                  adv_data[0],
                  adv_data[1],
                  adv_data[2],
                  adv_data[3],
                  adv_data[4],
                  adv_data[5],
                  adv_data[6],
                  adv_data[7],
                  sizeof(adv_data)-1);

  // update the advertiser with the new message
  set_advertiser(adv_data, 8);
}

/*8888b.           888    888        d8888 888b    888 8888888b.
888  "88b          888    888       d88888 8888b   888 888  "Y88b
888  .88P          888    888      d88P888 88888b  888 888    888
8888888K.          8888888888     d88P 888 888Y88b 888 888    888
888  "Y88b         888    888    d88P  888 888 Y88b888 888    888
888    888         888    888   d88P   888 888  Y88888 888    888
888   d88P         888    888  d8888888888 888   Y8888 888  .d88P
8888888P" 88888888 888    888 d88P     888 888    Y888 8888888*/

static void button_event_handler(uint32_t button_number) {
  __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Button %u pressed\n", button_number);

  uint32_t nrf_status = NRF_SUCCESS;

  switch (button_number) {
  case 0:
    break;
  case 1:
    break;
  case 2:
    break;
  case 3:
    break;
  default:
    break;
  }

  hal_led_blink_ms(LEDS_MASK, 50, 4);
}

/*8     888        d8888 8888888b. 88888888888
888     888       d88888 888   Y88b    888
888     888      d88P888 888    888    888
888     888     d88P 888 888   d88P    888
888     888    d88P  888 8888888P"     888
888     888   d88P   888 888 T88b      888
Y88b. .d88P  d8888888888 888  T88b     888
 "Y88888P"  d88P     888 888   T88b    8*/

void uart_error_handle(app_uart_evt_t *p_event) {
  if (p_event->evt_type == APP_UART_COMMUNICATION_ERROR) {
    ERROR_CHECK(p_event->data.error_communication);
  } else if (p_event->evt_type == APP_UART_FIFO_ERROR) {
    ERROR_CHECK(p_event->data.error_code);
  }
}

static void uart_init(void) {
  uint32_t err_code;

  const app_uart_comm_params_t comm_params = {RX_PIN_NUMBER,
      TX_PIN_NUMBER,
      RTS_PIN_NUMBER,
      CTS_PIN_NUMBER,
      UART_HWFC,
      false,
      UARTE_BAUDRATE_BAUDRATE_Baud115200};

  APP_UART_FIFO_INIT(
      &comm_params, UART_RX_BUF_SIZE, UART_TX_BUF_SIZE, uart_error_handle, APP_IRQ_PRIORITY_LOWEST, err_code);

  ERROR_CHECK(err_code);
}

/*8888b.  8888888888 888b    888 8888888b.           8888888b.   .d8888b.
d88P  Y88b 888        8888b   888 888  "Y88b          888   Y88b d88P  Y88b
Y88b.      888        88888b  888 888    888          888    888 888    888
"Y888b.   8888888    888Y88b 888 888    888          888   d88P 888
   "Y88b. 888        888 Y88b888 888    888          8888888P"  888
     "888 888        888  Y88888 888    888          888        888    888
Y88b  d88P 888        888   Y8888 888  .d88P          888        Y88b  d88P
"Y8888P"  8888888888 888    Y888 8888888P"  88888888 888         "Y8888*/

void send_data_pc(void) {
  static data_packet_t data_from_fifo;

  char msg_str[128];
  memset(msg_str, 0, sizeof(msg_str));

  gen_fifo_read(&rx_fifo, &data_from_fifo);

  // sprintf( msg_str, "From [%02x:%02x:%02x:%02x:%02x:%02x]  "
  //                           "Cmd \'%c\'  D %04d  H %04d  T %04d",

  sprintf(msg_str,
      "%02x%02x%02x%02x%02x%02x%c%02x%02x%02x%02x%02x%02x",
      data_from_fifo.d_addr[0],
      data_from_fifo.d_addr[1],
      data_from_fifo.d_addr[2],
      data_from_fifo.d_addr[3],
      data_from_fifo.d_addr[4],
      data_from_fifo.d_addr[5],
      data_from_fifo.d_command,
      data_from_fifo.d_dehydration,
      data_from_fifo.d_ambient_humidity,
      data_from_fifo.d_ambient_temperature,
      data_from_fifo.d_skin_temperature,
      data_from_fifo.d_heart_rate,
      data_from_fifo.d_dummy);

  // if (data_from_fifo.d_command == NOTIF_CMD) {
  //   SEGGER_PRINTF("to PC: %s\n", msg_str);
  // }

  // SEGGER_PRINTF("\
  //   command               = %c \n\r\
  //   d_dehydration         = %d \n\r\
  //   d_ambient_humidity    = %d \n\r\
  //   d_ambient_temperature = %d \n\r\
  //   d_skin_temperature    = %d \n\r\
  //   d_heart_rate          = %d \n\r\
  //   d_dummy               = %d \n\n\r",
  //   data_from_fifo.d_command,
  //   data_from_fifo.d_dehydration,
  //   data_from_fifo.d_ambient_humidity,
  //   data_from_fifo.d_ambient_temperature,
  //   data_from_fifo.d_skin_temperature,
  //   data_from_fifo.d_heart_rate,
  //   data_from_fifo.d_dummy
  // );

  UART_PRINTF("%s\n", msg_str);
}

/*8888b.  888    888 8888888888  .d8888b.  888    d8P
d88P  Y88b 888    888 888        d88P  Y88b 888   d8P
888    888 888    888 888        888    888 888  d8P
888        8888888888 8888888    888        888d88K
888        888    888 888        888        8888888b
888    888 888    888 888        888    888 888  Y88b
Y88b  d88P 888    888 888        Y88b  d88P 888   Y88b
"Y8888P"  888    888 8888888888  "Y8888P"  888    Y8*/

static uint32_t uart_rx_check(char *str_target) {
  static uint8_t cr;

  if (app_uart_get(&cr) == NRF_SUCCESS) {
    if (cr == '\177')    // \177 = backspace
    {
      if (str_target_iter > 0)    // check underflow
      {
        str_target_iter--;
      }
    } else {
      str_target[str_target_iter++] = cr;
    }

    if (cr == '\r') {
      if (str_target_iter) {
        str_target_iter--;
      }
      str_target[str_target_iter] = 0x00;    // set last char to \0
      str_target_iter             = 0;
      str_complete                = 1;
    } else {
      str_complete = 0;
    }
  }
  return 1;
}

/*88888b.   .d8888b.            .d8888b.  888b     d888 8888888b.
888   Y88b d88P  Y88b          d88P  Y88b 8888b   d8888 888  "Y88b
888    888 888    888          888    888 88888b.d88888 888    888
888   d88P 888                 888        888Y88888P888 888    888
8888888P"  888                 888        888 Y888P 888 888    888
888        888    888          888    888 888  Y8P  888 888    888
888        Y88b  d88P          Y88b  d88P 888   "   888 888  .d88P
888         "Y8888P"  88888888  "Y8888P"  888       888 8888888*/

static command_return_t select_command(uint8_t *received_str) {
  uint32_t nrf_status = NRF_SUCCESS;
  uint8_t cmd_char;
  command_return_t command_return;

  cmd_char = received_str[0];

  // SEGGER_PRINTF("\r\n\nMessage received from PC: \"%s\" ", received_str);

  switch (cmd_char) {
  case (PORT_TEST_MSG):
    UART_PRINTF("yes\n");
    command_return = NO_WAIT;
    break;

  case (TO_ALL):
    // SEGGER_PRINTF("Command to ALL nodes   %s\n\r", received_str);
    send_command_all(received_str);
    command_return = WAIT_REPLY;
    break;

  case (TO_SINGLE):
    // SEGGER_PRINTF("Command to SINGLE node %s\n\r", received_str);
    send_command_node(received_str);
    command_return = WAIT_REPLY;
    break;

  default:
    SEGGER_PRINTF("Command NOT recognized: %s\n\r", received_str);
    command_return = COMMAND_ERROR;
    break;
  }

  if (nrf_status == NRF_ERROR_INVALID_STATE || nrf_status == NRF_ERROR_NO_MEM || nrf_status == NRF_ERROR_BUSY) {
    __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "Cannot send. Device is busy.\n");
    hal_led_blink_ms(LEDS_MASK, 50, 4);
  } else {
    (nrf_status);
  }

  return command_return;
}
