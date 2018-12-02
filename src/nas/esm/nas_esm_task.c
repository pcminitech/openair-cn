/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under 
 * the Apache License, Version 2.0  (the "License"); you may not use this file
 * except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

/*! \file nas_mme_task.c
   \brief
   \author  Sebastien ROUX, Lionel GAUTHIER
   \date
   \email: lionel.gauthier@eurecom.fr
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "bstrlib.h"
#include "log.h"
#include "msc.h"
#include "assertions.h"
#include "common_defs.h"
#include "intertask_interface.h"
#include "itti_free_defined_msg.h"
#include "mme_config.h"
#include "nas_network.h"
#include "emm_main.h"
#include "nas_esm.h"
#include "nas_esm_proc.h"
#include "nas_timer.h"

static void nas_esm_exit(void);

//------------------------------------------------------------------------------
static void *nas_esm_intertask_interface (void *args_p)
{
  itti_mark_task_ready (TASK_NAS_ESM);

  while (1) {
    MessageDef                             *received_message_p = NULL;

    itti_receive_msg (TASK_NAS_ESM, &received_message_p);

    switch (ITTI_MSG_ID (received_message_p)) {
    case MESSAGE_TEST:{
        OAI_FPRINTF_INFO("TASK_NAS_ESM received MESSAGE_TEST\n");
      }
      break;

    case MME_APP_ACTIVATE_EPS_BEARER_CTX_REQ:
      nas_proc_activate_dedicated_bearer(&MME_APP_ACTIVATE_EPS_BEARER_CTX_REQ (received_message_p));
      break;

    case MME_APP_MODIFY_EPS_BEARER_CTX_REQ:
      nas_proc_modify_eps_bearer_ctx(&MME_APP_MODIFY_EPS_BEARER_CTX_REQ (received_message_p));
      break;

    case MME_APP_DEACTIVATE_EPS_BEARER_CTX_REQ:
      nas_proc_deactivate_dedicated_bearer(&MME_APP_DEACTIVATE_EPS_BEARER_CTX_REQ (received_message_p));
      break;

    case MME_APP_UPDATE_ESM_BEARER_CTXS_REQ:
      nas_proc_establish_bearer_update(&MME_APP_UPDATE_ESM_BEARER_CTXS_REQ (received_message_p));
      break;

    case MME_APP_E_RAB_FAILURE:
      nas_proc_e_rab_failure(MME_APP_E_RAB_FAILURE (received_message_p).mme_ue_s1ap_id, MME_APP_E_RAB_FAILURE (received_message_p).ebi,
          MME_APP_E_RAB_FAILURE (received_message_p).modify, MME_APP_E_RAB_FAILURE (received_message_p).remove);
      break;

    /** Messages sent directly from MME_APP to NAS_ESM layer for S11 session responses. */
    case NAS_PDN_CONNECTIVITY_RSP:{
        nas_proc_pdn_connectivity_res (&NAS_PDN_CONNECTIVITY_RSP (received_message_p));
      }
      break;

    case NAS_PDN_CONNECTIVITY_FAIL:{
        nas_proc_pdn_connectivity_fail (&NAS_PDN_CONNECTIVITY_FAIL (received_message_p));
      }
      break;

    case NAS_PDN_DISCONNECT_RSP:{
        nas_proc_pdn_disconnect_res (&NAS_PDN_DISCONNECT_RSP (received_message_p));
      }
      break;

    case TERMINATE_MESSAGE:{
        nas_esm_exit();
        OAI_FPRINTF_INFO("TASK_NAS_ESM terminated\n");
        itti_free_msg_content(received_message_p);
        itti_free (ITTI_MSG_ORIGIN_ID (received_message_p), received_message_p);
        itti_exit_task ();
      }
      break;

    case TIMER_HAS_EXPIRED:{
        /*
         * Call the NAS timer api
         */
        nas_timer_handle_signal_expiry (TIMER_HAS_EXPIRED (received_message_p).timer_id, TIMER_HAS_EXPIRED (received_message_p).arg);
      }
      break;

    default:{
        OAILOG_DEBUG (LOG_NAS, "Unknown message ID %d:%s from %s\n", ITTI_MSG_ID (received_message_p), ITTI_MSG_NAME (received_message_p), ITTI_MSG_ORIGIN_NAME (received_message_p));
      }
      break;
    }

    itti_free_msg_content(received_message_p);
    itti_free (ITTI_MSG_ORIGIN_ID (received_message_p), received_message_p);
    received_message_p = NULL;

  }

  return NULL;
}

//------------------------------------------------------------------------------
int nas_esm_init ()
{
  OAILOG_DEBUG (LOG_NAS, "Initializing NAS ESM task interface\n");
  esm_main_initialize();

  if (itti_create_task (TASK_NAS_ESM, &nas_esm_intertask_interface, NULL) < 0) {
    OAILOG_ERROR (LOG_NAS, "Create NAS ESM task failed");
    return -1;
  }

  OAILOG_DEBUG (LOG_NAS, "Initializing NAS ESM task interface: DONE\n");
  return 0;
}

//------------------------------------------------------------------------------
static void nas_esm_exit(void)
{
  OAILOG_DEBUG (LOG_NAS, "Cleaning NAS ESM task interface\n");
  esm_main_cleanup ();
  OAILOG_DEBUG (LOG_NAS, "Cleaning NAS ESM task interface: DONE\n");
}