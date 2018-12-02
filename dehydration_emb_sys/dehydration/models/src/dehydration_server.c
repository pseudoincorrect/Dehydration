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

#include "dehydration_server.h"
#include "dehydration_common.h"

#include <stdint.h>
#include <stddef.h>

#include "access.h"
#include "nrf_mesh_assert.h"

#include "log.h"

/*88888b.  8888888888 8888888b.  888    Y88b   d88P 
888   Y88b 888        888   Y88b 888     Y88b d88P  
888    888 888        888    888 888      Y88o88P   
888   d88P 8888888    888   d88P 888       Y888P    
8888888P"  888        8888888P"  888        888     
888 T88b   888        888        888        888     
888  T88b  888        888        888        888     
888   T88b 8888888888 888        88888888   8*/

static void reply_status(const dehydration_server_t * p_server,
                         const access_message_rx_t * p_message,
                         uint32_t key, uint32_t val)
{
    dehydration_msg_status_t status;
    status.key = key;
    status.val = val;
    access_message_tx_t reply;
    reply.opcode.opcode     = DEHYDRATION_OPCODE_STATUS;
    reply.opcode.company_id = ACCESS_COMPANY_ID_NORDIC;
    reply.p_buffer          = (const uint8_t *) &status;
    reply.length            = sizeof(status);

    (void) access_model_reply(p_server->model_handle, p_message, &reply);
}


/*88888b.  888     888 888888b.   888      8888888  .d8888b.  888    888 
888   Y88b 888     888 888  "88b  888        888   d88P  Y88b 888    888 
888    888 888     888 888  .88P  888        888   Y88b.      888    888 
888   d88P 888     888 8888888K.  888        888    "Y888b.   8888888888 
8888888P"  888     888 888  "Y88b 888        888       "Y88b. 888    888 
888        888     888 888    888 888        888         "888 888    888 
888        Y88b. .d88P 888   d88P 888        888   Y88b  d88P 888    888 
888         "Y88888P"  8888888P"  88888888 8888888  "Y8888P"  888    8*/

static void publish_state(dehydration_server_t * p_server, uint32_t key, uint32_t val)
{
    dehydration_msg_status_t status;
    status.key = key;
    status.val = val;
    access_message_tx_t msg;
    msg.opcode.opcode     = DEHYDRATION_OPCODE_STATUS;
    msg.opcode.company_id = ACCESS_COMPANY_ID_NORDIC;
    msg.p_buffer          = (const uint8_t *) &status;
    msg.length            = sizeof(status);
    (void) access_model_publish(p_server->model_handle, &msg);
}


 /*8888b.  8888888888 88888888888 
d88P  Y88b 888            888     
Y88b.      888            888     
 "Y888b.   8888888        888     
    "Y88b. 888            888     
      "888 888            888     
Y88b  d88P 888            888     
 "Y8888P"  8888888888     8*/

static void handle_set_cb(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    uint32_t value;

    dehydration_server_t * p_server = p_args;
  
   __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "SET somethingg\n");

    NRF_MESH_ASSERT(p_server->set_cb != NULL);

    uint32_t key = ((dehydration_msg_set_t*) p_message->p_data)->key;
    uint32_t val = ((dehydration_msg_set_t*) p_message->p_data)->val;

    value      = p_server->set_cb(p_server, key, val);

    reply_status (p_server, p_message, key, value);
}


 /*8888b.  8888888888 88888888888 
d88P  Y88b 888            888     
888    888 888            888     
888        8888888        888     
888  88888 888            888     
888    888 888            888     
Y88b  d88P 888            888     
 "Y8888P88 8888888888     8*/

static void handle_get_cb(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    uint32_t value;

    dehydration_server_t * p_server = p_args;
  
   __LOG(LOG_SRC_APP, LOG_LEVEL_INFO, "GET somethingg\n");

    NRF_MESH_ASSERT(p_server->set_cb != NULL);

    uint32_t key = ((dehydration_msg_set_t*) p_message->p_data)->key;
    uint32_t val = ((dehydration_msg_set_t*) p_message->p_data)->val;

    value      = p_server->get_cb(p_server, key, val);

    reply_status (p_server, p_message, key, value);
}


 /*88888b.  8888888b.   .d8888b.   .d88888b.  8888888b.  8888888888 
d88P" "Y88b 888   Y88b d88P  Y88b d88P" "Y88b 888  "Y88b 888        
888     888 888    888 888    888 888     888 888    888 888        
888     888 888   d88P 888        888     888 888    888 8888888    
888     888 8888888P"  888        888     888 888    888 888        
888     888 888        888    888 888     888 888    888 888        
Y88b. .d88P 888        Y88b  d88P Y88b. .d88P 888  .d88P 888        
 "Y88888P"  888         "Y8888P"   "Y88888P"  8888888P"  88888888*/

static const access_opcode_handler_t m_opcode_handlers[] =
{
    {ACCESS_OPCODE_VENDOR(DEHYDRATION_OPCODE_SET, ACCESS_COMPANY_ID_NORDIC), handle_set_cb},
    {ACCESS_OPCODE_VENDOR(DEHYDRATION_OPCODE_GET, ACCESS_COMPANY_ID_NORDIC), handle_get_cb},
};


/*88888 888b    888 8888888 88888888888 
  888   8888b   888   888       888     
  888   88888b  888   888       888     
  888   888Y88b 888   888       888     
  888   888 Y88b888   888       888     
  888   888  Y88888   888       888     
  888   888   Y8888   888       888     
8888888 888    Y888 8888888     8*/

uint32_t dehydration_server_init(dehydration_server_t * p_server, uint16_t element_index)
{
    if (p_server == NULL ||
        p_server->get_cb == NULL ||
        p_server->set_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }

    access_model_add_params_t init_params;
    init_params.element_index       =  element_index;
    init_params.model_id.model_id   = DEHYDRATION_SERVER_MODEL_ID;
    init_params.model_id.company_id = ACCESS_COMPANY_ID_NORDIC;
    init_params.p_opcode_handlers   = &m_opcode_handlers[0];
    init_params.opcode_count        = sizeof(m_opcode_handlers) / sizeof(m_opcode_handlers[0]);
    init_params.p_args              = p_server;
    init_params.publish_timeout_cb  = NULL;
    return access_model_add(&init_params, &p_server->model_handle);
}
