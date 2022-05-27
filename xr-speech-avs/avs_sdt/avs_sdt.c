 /*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Copyright 2020 Metrological
 * Licensed under the Apache License, Version 2.0
 *
 */

//#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>    /* For O_RDWR */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "avs_sdt_private.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "../AVS.h"

#define AVS_SDT_IDENTIFIER (0xC11FB9C2)

uint8_t audio_buffer[3840];

static bool     avs_sdt_object_is_valid(avs_sdt_obj_t *obj);

static void avs_sdt_handler_session_begin(void *data, const uuid_t uuid, xrsr_src_t src, uint32_t dst_index, xrsr_keyword_detector_result_t *detector_result, xrsr_session_configuration_t *configuration, rdkx_timestamp_t *timestamp);
static void avs_sdt_handler_session_end(void *data, const uuid_t uuid, xrsr_session_stats_t *stats, rdkx_timestamp_t *timestamp);
static void avs_sdt_handler_stream_begin(void *data, const uuid_t uuid, xrsr_src_t src, rdkx_timestamp_t *timestamp);
static void avs_sdt_handler_stream_kwd(void *data, const uuid_t uuid, rdkx_timestamp_t *timestamp);
static void avs_sdt_handler_stream_end(void *data, const uuid_t uuid, xrsr_stream_stats_t *stats, rdkx_timestamp_t *timestamp);
static void avs_sdt_handler_connected(void *data, const uuid_t uuid, xrsr_handler_send_t send, void *param, rdkx_timestamp_t *timestamp);
static void avs_sdt_handler_disconnected(void *data, const uuid_t uuid, xrsr_session_end_reason_t reason, bool retry, bool *detect_resume, rdkx_timestamp_t *timestamp);
static int avs_recv_audiodata(unsigned char* frame,uint32_t sample_qty);

bool avs_sdt_object_is_valid(avs_sdt_obj_t *obj) {
   if(obj != NULL && obj->identifier == AVS_SDT_IDENTIFIER) {
      return(true);
   }
   return(false);
}

avs_sdt_object_t avs_sdt_create(const avs_sdt_params_t *params)
{
   XLOGD_DEBUG(" Create avs sdt V2.0");

   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)malloc(sizeof(avs_sdt_obj_t));

   if(obj == NULL) {
      XLOGD_ERROR("Out of memory.");
      return(NULL);
   }

   memset(obj, 0, sizeof(*obj));

   obj->identifier     = AVS_SDT_IDENTIFIER;
   obj->user_data      = params->user_data;
   
   //AVS_Initialize();

  return(obj) ;
}

bool avs_sdt_handlers(avs_sdt_object_t object, const avs_sdt_handlers_t *handlers_in, xrsr_handlers_t *handlers_out) {
  
   XLOGD_DEBUG(" Set avs sdt handlers ");
  
   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)object;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return(false);
   }
   bool ret = true;
   handlers_out->data          = obj;
   //handlers_out->session_begin = (xrsr_handler_session_begin_t)avs_sdt_handler_session_begin;
   handlers_out->session_begin = avs_sdt_handler_session_begin;
   handlers_out->session_end   = avs_sdt_handler_session_end;
   handlers_out->stream_begin  = avs_sdt_handler_stream_begin;
   handlers_out->stream_kwd    = avs_sdt_handler_stream_kwd;
   handlers_out->stream_end    = avs_sdt_handler_stream_end;
   //handlers_out->connected     = (xrsr_handler_connected_t)avs_sdt_handler_connected;
   handlers_out->connected     = avs_sdt_handler_connected;
   handlers_out->disconnected  = avs_sdt_handler_disconnected;
   handlers_out->stream_audio  = avs_recv_audiodata;

   obj->handlers = *handlers_in;

   return(ret);
}

int avs_recv_audiodata(unsigned char* data, uint32_t size)
{
  uint16_t length = size;
  XLOGD_DEBUG("Received Buffer Size:%d",size);
  memcpy(audio_buffer,data,size);
  /***AVS DATA***/
  Voice_Data(0,audio_buffer,length);
}


void avs_sdt_destroy(avs_sdt_object_t object) {

   XLOGD_DEBUG(" Destroy avs sdt ");   
	
   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)object;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }
   XLOGD_INFO("");
   obj->identifier                     = 0;
   free(obj);
   
   AVS_DeInitialize();
}

void avs_sdt_handler_session_begin(void *data, const uuid_t uuid, xrsr_src_t src, uint32_t dst_index, xrsr_keyword_detector_result_t *detector_result, xrsr_session_configuration_t *configuration, rdkx_timestamp_t *timestamp) {

   XLOGD_DEBUG(" Begin avs sdt session .....");

   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)data;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }

   char uuid_str[37] = {'\0'};
   int rc = 0;
   avs_sdt_stream_params_t stream_params;
   stream_params.keyword_sample_begin               = (detector_result != NULL ? detector_result->offset_kwd_begin - detector_result->offset_buf_begin : 0);
   stream_params.keyword_sample_end                 = (detector_result != NULL ? detector_result->offset_kwd_end   - detector_result->offset_buf_begin : 0);
   stream_params.keyword_doa                        = (detector_result != NULL ? detector_result->doa : 0);
   stream_params.keyword_sensitivity                = 0;
   stream_params.keyword_sensitivity_triggered      = false;
   stream_params.keyword_sensitivity_high           = 0;
   stream_params.keyword_sensitivity_high_support   = false;
   stream_params.keyword_sensitivity_high_triggered = false;
   stream_params.dynamic_gain                       = 0.0;
   stream_params.linear_confidence                  = 0.0;
   stream_params.nonlinear_confidence               = 0;
   stream_params.signal_noise_ratio                 = 255.0; // Invalid;
   stream_params.push_to_talk                       = false;

   if(obj->handlers.session_begin != NULL) {
      (*obj->handlers.session_begin)(uuid, src, dst_index, configuration, &stream_params, timestamp,obj->user_data);
   }
   

}

void avs_sdt_handler_session_end(void *data, const uuid_t uuid, xrsr_session_stats_t *stats, rdkx_timestamp_t *timestamp) {
   
   XLOGD_DEBUG(" End avs sdt session .....");
   
   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)data;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }
   if(obj->handlers.session_end != NULL) {
      (*obj->handlers.session_end)(uuid, stats, timestamp,obj->user_data);
   }
   


}

void avs_sdt_handler_stream_begin(void *data, const uuid_t uuid, xrsr_src_t src, rdkx_timestamp_t *timestamp) {
   
   XLOGD_DEBUG(" Begin avs sdt stream .....");
   
   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)data;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }

   if(obj->handlers.stream_begin != NULL) {
      (*obj->handlers.stream_begin)(uuid, src, timestamp, obj->user_data);
   }
   
   /***AVS VOICE START***/
   Voice_Start();
}

void avs_sdt_handler_stream_kwd(void *data, const uuid_t uuid, rdkx_timestamp_t *timestamp) {

   XLOGD_DEBUG(" KWD avs sdt stream .....");

   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)data;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }

   if(obj->handlers.stream_kwd != NULL) {
      (*obj->handlers.stream_kwd)(uuid, timestamp,obj->user_data);
   }
}

void avs_sdt_handler_stream_end(void *data, const uuid_t uuid, xrsr_stream_stats_t *stats, rdkx_timestamp_t *timestamp) {

   XLOGD_DEBUG(" End avs sdt stream .....");

   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)data;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }

   if(obj->handlers.stream_end != NULL) {
      (*obj->handlers.stream_end)(uuid, stats, timestamp, obj->user_data);
   }
   
   /***AVS VOICE STOP***/
    Voice_Stop();
}

void avs_sdt_handler_connected(void *data, const uuid_t uuid, xrsr_handler_send_t send, void *param, rdkx_timestamp_t *timestamp) {

   XLOGD_DEBUG(" avs sdt handler connected .....");

   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)data;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }
   if(obj->handlers.connected != NULL) {
      (*obj->handlers.connected)(uuid, timestamp,obj->user_data);
   }
}

void avs_sdt_handler_disconnected(void *data, const uuid_t uuid, xrsr_session_end_reason_t reason, bool retry, bool *detect_resume, rdkx_timestamp_t *timestamp) {
   
   XLOGD_DEBUG(" avs sdt handler disconnected .....");

	//avs_close();

   avs_sdt_obj_t *obj = (avs_sdt_obj_t *)data;
   if(!avs_sdt_object_is_valid(obj)) {
      XLOGD_ERROR("invalid object");
      return;
   }

   if(obj->handlers.disconnected != NULL) {
      (*obj->handlers.disconnected)(uuid, retry, timestamp, obj->user_data);
   }
}
