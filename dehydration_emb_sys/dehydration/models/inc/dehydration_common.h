/* Copyright (c) 2010 - 2017, Nordic Semiconductor ASA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DEHYDRATION_COMMON_H__
#define DEHYDRATION_COMMON_H__

#include <stdint.h>

//
//#define NAME_OP_BASE = 0x6;
//#define NAME_CHAR_PER_PACKET = 4;
#define OP_SIZE BUZZER_OP

/**
 * @defgroup dehydration_MODEL dehydration model
 * Example model implementing basic behavior for sending on and off messages.
 * @ingroup MESH_API_GROUP_MODELS
 * @{
 * @defgroup dehydration_COMMON Common dehydration definitions
 * Types and definitions shared between the two dehydration models.
 * @{
 */

/*lint -align_max(push) -align_max(1) */

typedef enum
{
    DEHYDRATION_PUBLISH        = 0x1,  
    DEHYDRATION_REPLY          = 0x2,  
    DEHYDRATION_NO_REPLY_ERROR = 0x3
} dehydration_message_state_t;

/** dehydration opcodes. */
typedef enum
{
    DEHYDRATION_OPCODE_SET    = 0xC1,  
    DEHYDRATION_OPCODE_GET    = 0xC2,  
    DEHYDRATION_OPCODE_STATUS = 0xC3
} dehydration_opcode_t;

/** dehydration opcodes. */
typedef enum
{
    DEHYDRATION_OP = 0x1,  
    HUMIDITY_OP    = 0x2,  
    TEMPERATURE_OP = 0x3,
    LED_OP         = 0x4,
    BUZZER_OP      = 0x5 
//    NAME_0_OP      = NAME_OP_BASE + 0x0,
//    NAME_1_OP      = NAME_OP_BASE + 0x1,
//    NAME_2_OP      = NAME_OP_BASE + 0x2,
//    NAME_3_OP      = NAME_OP_BASE + 0x3,
//    NAME_4_OP      = NAME_OP_BASE + 0x4,
//    NAME_5_OP      = NAME_OP_BASE + 0x5,
//    NAME_6_OP      = NAME_OP_BASE + 0x6
} STATE_OP_t;

//#define NAME_OP_LENGHT = NAME_6_OP - NAME_0_OP + 1;

/** Message format for the dehydration Set message. */
typedef struct __attribute((packed))
{
    uint32_t key; 
    uint32_t val; 
} dehydration_msg_set_t;


/** Message format for the dehydration Status message. */
typedef struct __attribute((packed))
{
    uint32_t key; 
    uint32_t val; 
} dehydration_msg_status_t;

/*lint -align_max(pop) */

/** @} end of dehydration_COMMON */
/** @} end of dehydration_MODEL */
#endif /* dehydration_COMMON_H__ */
