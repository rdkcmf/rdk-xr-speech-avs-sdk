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
#ifndef __AVS_SDT_SUPPORT__
#define __AVS_SDT_SUPPORT__

#include <stdint.h>
#include <stdbool.h>
#include <xrsr.h>
#include <xr_timestamp.h>
#include <jansson.h>


#define AVS_SDT_SESSION_ID_LEN_MAX      (64)  ///< Session identifier maximum length including NULL termination
#define AVS_SDT_SESSION_STR_LEN_MAX     (512) ///< Session strings maximum length including NULL termination

typedef struct {
   bool        test_flag;        ///< True if the device is used for testing only, otherwise false
   void       *user_data;        ///< User data that is passed in to all of the callbacks
} avs_sdt_params_t;

/// AVS stream parameter structure
/// The stream parameter data structure is returned in the session begin callback function.
typedef struct {
   uint32_t keyword_sample_begin;               ///< The offset in samples from the beginning of the buffered audio to the beginning of the keyword
   uint32_t keyword_sample_end;                 ///< The offset in samples from the beginning of the buffered audio to the end of the keyword
   uint16_t keyword_doa;                        ///< The direction of arrival in degrees (0-359)
   uint16_t keyword_sensitivity;                ///<
   uint16_t keyword_sensitivity_triggered;      ///<
   bool     keyword_sensitivity_high_support;   ///<
   bool     keyword_sensitivity_high_triggered; ///<
   uint16_t keyword_sensitivity_high;           ///<
   double   dynamic_gain;                       ///<
   double   signal_noise_ratio;                 ///<
   double   linear_confidence;                  ///<
   int32_t  nonlinear_confidence;               ///<
   bool     push_to_talk;                       ///< True if the session was started by the user pressing a button
} avs_sdt_stream_params_t;

//sdt object
typedef void * avs_sdt_object_t;

//sdt session begin handler
typedef void (*avs_sdt_handler_session_begin_t)(const uuid_t uuid, xrsr_src_t src, uint32_t dst_index, xrsr_session_configuration_t *configuration, avs_sdt_stream_params_t *stream_params, rdkx_timestamp_t *timestamp, void *user_data);

//sdt session end handler
typedef void (*avs_sdt_handler_session_end_t)(const uuid_t uuid, xrsr_session_stats_t *stats, rdkx_timestamp_t *timestamp, void *user_data);

//sdt stream begin handler
typedef void (*avs_sdt_handler_stream_begin_t)(const uuid_t uuid, xrsr_src_t src, rdkx_timestamp_t *timestamp, void *user_data);

//sdt kwd handler
typedef void (*avs_sdt_handler_stream_kwd_t)(const uuid_t uuid, rdkx_timestamp_t *timestamp, void *user_data); 

//sdt stream end handler
typedef void (*avs_sdt_handler_stream_end_t)(const uuid_t uuid, xrsr_stream_stats_t *stats, rdkx_timestamp_t *timestamp, void *user_data);

//sdt stream connect handler
typedef void (*avs_sdt_handler_connected_t)(const uuid_t uuid, rdkx_timestamp_t *timestamp, void *user_data);

//sdt disconnect handler
typedef void (*avs_sdt_handler_disconnected_t)(const uuid_t uuid, bool retry, rdkx_timestamp_t *timestamp, void *user_data);

//sdt handler structure
typedef struct {
   avs_sdt_handler_session_begin_t     session_begin;     ///< Indicates that a voice session has started
   avs_sdt_handler_session_end_t       session_end;       ///< Indicates that a voice session has ended
   avs_sdt_handler_stream_begin_t      stream_begin;      ///< An audio stream has started
   avs_sdt_handler_stream_kwd_t        stream_kwd;        ///< The keyword has been passed in the stream
   avs_sdt_handler_stream_end_t        stream_end;        ///< An audio stream has ended
   avs_sdt_handler_connected_t         connected;         ///< The session has connected
   avs_sdt_handler_disconnected_t      disconnected;      ///< The session has disconnected
} avs_sdt_handlers_t;

#ifdef __cplusplus
extern "C" {
#endif

avs_sdt_object_t avs_sdt_create(const avs_sdt_params_t *params);

bool avs_sdt_handlers(avs_sdt_object_t object, const avs_sdt_handlers_t *handlers_in, xrsr_handlers_t *handlers_out);

void avs_sdt_destroy(avs_sdt_object_t object);

#ifdef __cplusplus
}
#endif


#endif

