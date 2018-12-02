## RECEIVE (RX) CONSTANT
MSG_UPDATE = 'u'
MSG_NOTIF  = 'n'
## UART packet sections sizes in char
CMD_LENGHT 	  = 1 # 1 char for the command
SERVER_LENGHT = 4 # 4 chars for the server index number
VAL_LENGHT 	  = 4 # 4 chars for the val number
## indexes for the received data packet
I_ADD  = 0
I_CMD  = 12
I_DEHID = 13
I_A_HUM = 15
I_A_TEM = 17
I_S_TEM = 19
I_HEART = 21
I_DUMMY = 23
DATA_SIZE = 2

##SENDING (TX) CONSTANT
## command type
TO_SINGLE   = 's'
TO_ALL      = 'a'
## command to single node
DRINK_NOTIF = 'd'
NOTIF       = 'n'
## command to all
NOTIF_ALL   = 'n'
## Port Test constant messages
TEST_MSG        = 't\r'
ANSWER_MSG      = 'yes\n'
