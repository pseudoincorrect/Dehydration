#ifndef __COMMAND_DEF_H__
#define __COMMAND_DEF_H__

// UART Commands
#define PORT_TEST_MSG 't'
#define TO_ALL        'a'
#define TO_SINGLE     's'

// CLIENT
// command to single node
#define DRINK_NOTIF   'd'
#define NOTIF         'n'
// command to all
#define NOTIF_ALL     'n'
// position of the command char in a "sigle cmd" message
#define CMD_CHAR_POS  (BLE_GAP_ADDR_LEN+1)

// SERVER
#define UPDATE_CMD  'u'
#define NOTIF_CMD   'n'


#define RECV_ADDR_SIZE (BLE_GAP_ADDR_LEN*2)

#define D_ADDR_SIZE           (BLE_GAP_ADDR_LEN)
#define D_COMMAND_POS         (D_ADDR_SIZE+0)
#define D_DEHYDRATION         (D_ADDR_SIZE+1)
#define D_AMBIENT_HUMIDITY    (D_ADDR_SIZE+2)
#define D_AMBIENT_TEMPERATURE (D_ADDR_SIZE+3)
#define D_SKIN_TEMPERATURE    (D_ADDR_SIZE+4)
#define D_HEART_RATE          (D_ADDR_SIZE+5)
#define D_DUMMY               (D_ADDR_SIZE+6)

// #define D_DEHYDRATION_MSB (D_ADDR_SIZE+1)
// #define D_DEHYDRATION_LSB (D_ADDR_SIZE+2)
// #define D_HUMIDITY_MSB    (D_ADDR_SIZE+3)
// #define D_HUMIDITY_LSB    (D_ADDR_SIZE+4)
// #define D_TEMPERATURE_MSB (D_ADDR_SIZE+5)
// #define D_TEMPERATURE_LSB (D_ADDR_SIZE+6)

#define D_NOTIF (D_ADDR_SIZE+1)


typedef struct
{
	uint8_t  d_addr [BLE_GAP_ADDR_LEN];
    uint8_t  d_command;
    int8_t   d_dehydration;
    int8_t   d_ambient_humidity;
    int8_t   d_ambient_temperature;
    int8_t   d_skin_temperature;
    int8_t   d_heart_rate;
    int8_t   d_dummy;
} data_packet_t;


// typedef struct
// {
//     uint8_t  d_addr   [BLE_GAP_ADDR_LEN];
//     uint8_t  d_command;
//     uint16_t d_dehydration;
//     uint16_t d_humidity;
//     uint16_t d_temperature;
// } data_packet_t;

typedef enum
{
    COMMAND_ERROR,
    NO_WAIT,
    WAIT_REPLY
} command_return_t;

#endif
