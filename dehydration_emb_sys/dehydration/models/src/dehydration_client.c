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

#include "dehydration_client.h"
#include "dehydration_common.h"

#include <stdint.h>
#include <stddef.h>

#include "access.h"
#include "access_config.h"
#include "access_reliable.h"
#include "device_state_manager.h"
#include "nrf_mesh.h"
#include "nrf_mesh_assert.h"

#include "log.h"

/* Forward declarations */
static void reliable_status_cb(access_model_handle_t model_handle, void * p_args, access_reliable_status_t status);


/*8     888      d8888 888      8888888 8888888b.  
888     888     d88888 888        888   888  "Y88b 
888     888    d88P888 888        888   888    888 
Y88b   d88P   d88P 888 888        888   888    888 
 Y88b d88P   d88P  888 888        888   888    888 
  Y88o88P   d88P   888 888        888   888    888 
   Y888P   d8888888888 888        888   888  .d88P 
    Y8P   d88P     888 88888888 8888888 8888888*/

/** Returns @c true if the message received was from the address corresponding to the clients
 * publish address. */
static bool is_valid_source(const dehydration_client_t * p_client,
                            const access_message_rx_t * p_message)
{
    /* Check the originator of the status. */
    dsm_handle_t publish_handle;
    nrf_mesh_address_t publish_address;
    if (access_model_publish_address_get(p_client->model_handle, &publish_handle) != NRF_SUCCESS ||
        publish_handle == DSM_HANDLE_INVALID ||
        dsm_address_get(publish_handle, &publish_address) != NRF_SUCCESS ||
        publish_address.value != p_message->meta_data.src.value)
    {
        return false;
    }
    else
    {
        return true;
    }
}


 /*8888b.  8888888888 888b    888 8888888b.  
d88P  Y88b 888        8888b   888 888  "Y88b 
Y88b.      888        88888b  888 888    888 
 "Y888b.   8888888    888Y88b 888 888    888 
    "Y88b. 888        888 Y88b888 888    888 
      "888 888        888  Y88888 888    888 
Y88b  d88P 888        888   Y8888 888  .d88P 
 "Y8888P"  8888888888 888    Y888 8888888*/

static uint32_t send_reliable_message(const dehydration_client_t * p_client,
                                      dehydration_opcode_t opcode,
                                      const uint8_t * p_data,
                                      uint16_t length)
{
    access_reliable_t reliable;

    reliable.model_handle              = p_client->model_handle;
    reliable.message.p_buffer          = p_data;
    reliable.message.length            = length;
    reliable.message.opcode.opcode     = opcode;
    reliable.message.opcode.company_id = ACCESS_COMPANY_ID_NORDIC;
    reliable.reply_opcode.opcode       = DEHYDRATION_OPCODE_STATUS;
    reliable.reply_opcode.company_id   = ACCESS_COMPANY_ID_NORDIC;
    reliable.timeout                   = ACCESS_RELIABLE_TIMEOUT_MIN;
    reliable.status_cb                 = reliable_status_cb;

    return access_model_reliable_publish(&reliable);
}


 /*8888b. 88888888888     d8888 88888888888 888     888  .d8888b.  
d88P  Y88b    888        d88888     888     888     888 d88P  Y88b 
Y88b.         888       d88P888     888     888     888 Y88b.      
 "Y888b.      888      d88P 888     888     888     888  "Y888b.   
    "Y88b.    888     d88P  888     888     888     888     "Y88b. 
      "888    888    d88P   888     888     888     888       "888 
Y88b  d88P    888   d8888888888     888     Y88b. .d88P Y88b  d88P 
 "Y8888P"     888  d88P     888     888      "Y88888P"   "Y8888*/

static void handle_status_cb(access_model_handle_t handle, const access_message_rx_t * p_message, void * p_args)
{
    dehydration_client_t * p_client = p_args;
    NRF_MESH_ASSERT(p_client->status_cb != NULL);

    if (!is_valid_source(p_client, p_message))
    {
        return;
    }

    dehydration_msg_status_t * p_status = (dehydration_msg_status_t *) p_message->p_data;

    uint32_t rec_key  = (p_status->key);
    uint32_t rec_val  = (p_status->val);

    p_client->status_cb(p_client, rec_key, rec_val, p_client, DEHYDRATION_PUBLISH);
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
    {{DEHYDRATION_OPCODE_STATUS, ACCESS_COMPANY_ID_NORDIC}, handle_status_cb}
};


/*88888 888b    888 8888888 88888888888 
  888   8888b   888   888       888     
  888   88888b  888   888       888     
  888   888Y88b 888   888       888     
  888   888 Y88b888   888       888     
  888   888  Y88888   888       888     
  888   888   Y8888   888       888     
8888888 888    Y888 8888888     8*/

uint32_t dehydration_client_init(dehydration_client_t * p_client, uint16_t element_index)
{
    if (p_client == NULL ||
        p_client->status_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }

    access_model_add_params_t init_params;

    init_params.model_id.model_id   = DEHYDRATION_CLIENT_MODEL_ID;
    init_params.model_id.company_id = ACCESS_COMPANY_ID_NORDIC;
    init_params.element_index       = element_index;
    init_params.p_opcode_handlers   = &m_opcode_handlers[0];
    init_params.opcode_count        = sizeof(m_opcode_handlers) / sizeof(m_opcode_handlers[0]);
    init_params.p_args              = p_client;
    init_params.publish_timeout_cb  = NULL;

    return access_model_add(&init_params, &p_client->model_handle);
}

/** Keeps a single global TID for all transfers. */
static uint8_t m_tid;


/*88888b.           .d8888b. 88888888888     d8888 88888888888 888     888  .d8888b.  
888   Y88b         d88P  Y88b    888        d88888     888     888     888 d88P  Y88b 
888    888         Y88b.         888       d88P888     888     888     888 Y88b.      
888   d88P          "Y888b.      888      d88P 888     888     888     888  "Y888b.   
8888888P"              "Y88b.    888     d88P  888     888     888     888     "Y88b. 
888 T88b                 "888    888    d88P   888     888     888     888       "888 
888  T88b          Y88b  d88P    888   d8888888888     888     Y88b. .d88P Y88b  d88P 
888   T88b 88888888 "Y8888P"     888  d88P     888     888      "Y88888P"   "Y8888*/

static void reliable_status_cb(access_model_handle_t model_handle,
                               void * p_args,
                               access_reliable_status_t status)
{
    dehydration_client_t * p_client = p_args;
    NRF_MESH_ASSERT(p_client->status_cb != NULL);

    p_client->state.reliable_transfer_active = false;

    switch (status)
    {
        case ACCESS_RELIABLE_TRANSFER_SUCCESS:
//            p_client->status_cb(p_client, 
//                                p_client->state.data.key, 
//                                p_client->state.data.val, 
//                                p_client, // TODO get the address
//                                //p_message->meta_data.src.value, 
//                                DEHYDRATION_REPLY);
            break;

        case ACCESS_RELIABLE_TRANSFER_TIMEOUT:
            p_client->status_cb(p_client, 
                                0, 
                                0, 
                                (dehydration_client_t  *) NULL, 
                                DEHYDRATION_NO_REPLY_ERROR);
            break;

        default:
            /* Should not be possible. */
            NRF_MESH_ASSERT(false);
            break;
    }
}


 /*8888b.  8888888888 88888888888 
d88P  Y88b 888            888     
Y88b.      888            888     
 "Y888b.   8888888        888     
    "Y88b. 888            888     
      "888 888            888     
Y88b  d88P 888            888     
 "Y8888P"  8888888888     8*/

uint32_t dehydration_client_set(dehydration_client_t * p_client, uint32_t key, uint32_t val)
{
    if (p_client == NULL || p_client->status_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }
    else if (p_client->state.reliable_transfer_active)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    p_client->state.data.key = key;
    p_client->state.data.val = val;

    uint32_t status = send_reliable_message(p_client,
                                            DEHYDRATION_OPCODE_SET,
                                            (const uint8_t *)&p_client->state.data,
                                            sizeof(dehydration_msg_set_t));
    if (status == NRF_SUCCESS)
    {
        p_client->state.reliable_transfer_active = true;
    }
    return status;

}


 /*8888b.  8888888888 88888888888 
d88P  Y88b 888            888     
888    888 888            888     
888        8888888        888     
888  88888 888            888     
888    888 888            888     
Y88b  d88P 888            888     
 "Y8888P88 8888888888     8*/

uint32_t dehydration_client_get(dehydration_client_t * p_client, uint32_t key)
{
    if (p_client == NULL || p_client->status_cb == NULL)
    {
        return NRF_ERROR_NULL;
    }
    else if (p_client->state.reliable_transfer_active)
    {
        return NRF_ERROR_INVALID_STATE;
    }

    p_client->state.data.key = key;
    p_client->state.data.val = 0;

    uint32_t status = send_reliable_message(p_client,
                                            DEHYDRATION_OPCODE_GET,
                                            (const uint8_t *)&p_client->state.data,
                                            sizeof(dehydration_msg_set_t));
    if (status == NRF_SUCCESS)
    {
        p_client->state.reliable_transfer_active = true;
    }
    return status;

}
