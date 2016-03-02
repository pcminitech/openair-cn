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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include "3gpp_24.007.h"
#include "3gpp_24.301.h"
#include "TLVEncoder.h"
#include "TLVDecoder.h"
#include "EsmInformationResponse.h"

int
decode_esm_information_response (
  esm_information_response_msg * esm_information_response,
  uint8_t * buffer,
  uint32_t len)
{
  uint32_t                                decoded = 0;
  int                                     decoded_result = 0;

  // Check if we got a NULL pointer and if buffer length is >= minimum length expected for the message.
  CHECK_PDU_POINTER_AND_LENGTH_DECODER (buffer, ESM_INFORMATION_RESPONSE_MINIMUM_LENGTH, len);

  /*
   * Decoding mandatory fields
   */
  /*
   * Decoding optional fields
   */
  while (len - decoded > 0) {
    uint8_t                                 ieiDecoded = *(buffer + decoded);

    /*
     * Type | value iei are below 0x80 so just return the first 4 bits
     */
    if (ieiDecoded >= 0x80)
      ieiDecoded = ieiDecoded & 0xf0;

    switch (ieiDecoded) {
    case ESM_INFORMATION_RESPONSE_ACCESS_POINT_NAME_IEI:
      if ((decoded_result = decode_access_point_name (&esm_information_response->accesspointname, ESM_INFORMATION_RESPONSE_ACCESS_POINT_NAME_IEI, buffer + decoded, len - decoded)) <= 0)
        return decoded_result;

      decoded += decoded_result;
      /*
       * Set corresponding mask to 1 in presencemask
       */
      esm_information_response->presencemask |= ESM_INFORMATION_RESPONSE_ACCESS_POINT_NAME_PRESENT;
      break;

    case ESM_INFORMATION_RESPONSE_PROTOCOL_CONFIGURATION_OPTIONS_IEI:
      if ((decoded_result = decode_protocol_configuration_options (&esm_information_response->protocolconfigurationoptions, ESM_INFORMATION_RESPONSE_PROTOCOL_CONFIGURATION_OPTIONS_IEI, buffer + decoded, len - decoded)) <= 0)
        return decoded_result;

      decoded += decoded_result;
      /*
       * Set corresponding mask to 1 in presencemask
       */
      esm_information_response->presencemask |= ESM_INFORMATION_RESPONSE_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT;
      break;

    default:
      errorCodeDecoder = TLV_DECODE_UNEXPECTED_IEI;
      return TLV_DECODE_UNEXPECTED_IEI;
    }
  }

  return decoded;
}

int
encode_esm_information_response (
  esm_information_response_msg * esm_information_response,
  uint8_t * buffer,
  uint32_t len)
{
  int                                     encoded = 0;
  int                                     encode_result = 0;

  /*
   * Checking IEI and pointer
   */
  CHECK_PDU_POINTER_AND_LENGTH_ENCODER (buffer, ESM_INFORMATION_RESPONSE_MINIMUM_LENGTH, len);

  if ((esm_information_response->presencemask & ESM_INFORMATION_RESPONSE_ACCESS_POINT_NAME_PRESENT)
      == ESM_INFORMATION_RESPONSE_ACCESS_POINT_NAME_PRESENT) {
    if ((encode_result = encode_access_point_name (&esm_information_response->accesspointname, ESM_INFORMATION_RESPONSE_ACCESS_POINT_NAME_IEI, buffer + encoded, len - encoded)) < 0)
      // Return in case of error
      return encode_result;
    else
      encoded += encode_result;
  }

  if ((esm_information_response->presencemask & ESM_INFORMATION_RESPONSE_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT)
      == ESM_INFORMATION_RESPONSE_PROTOCOL_CONFIGURATION_OPTIONS_PRESENT) {
    if ((encode_result = encode_protocol_configuration_options (&esm_information_response->protocolconfigurationoptions, ESM_INFORMATION_RESPONSE_PROTOCOL_CONFIGURATION_OPTIONS_IEI, buffer + encoded, len - encoded)) < 0)
      // Return in case of error
      return encode_result;
    else
      encoded += encode_result;
  }

  return encoded;
}