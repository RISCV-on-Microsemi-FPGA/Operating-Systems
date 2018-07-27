/*
// Copyright (c) 2016 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#ifndef OC_RI_H
#define OC_RI_H

#include "oic/port/mynewt/config.h"
#include "oic/oc_rep.h"
#include "oic/oc_uuid.h"
#include "oic/port/oc_connectivity.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { OC_GET = 1, OC_POST, OC_PUT, OC_DELETE } oc_method_t;

typedef enum {
  OC_DISCOVERABLE = (1 << 0),
  OC_OBSERVABLE = (1 << 1),
  OC_ACTIVE = (1 << 2),
  OC_SECURE = (1 << 4),
  OC_PERIODIC = (1 << 6),
} oc_resource_properties_t;

typedef enum {
  OC_STATUS_OK = 0,
  OC_STATUS_CREATED,
  OC_STATUS_CHANGED,
  OC_STATUS_DELETED,
  OC_STATUS_NOT_MODIFIED,
  OC_STATUS_BAD_REQUEST,
  OC_STATUS_UNAUTHORIZED,
  OC_STATUS_BAD_OPTION,
  OC_STATUS_FORBIDDEN,
  OC_STATUS_NOT_FOUND,
  OC_STATUS_METHOD_NOT_ALLOWED,
  OC_STATUS_NOT_ACCEPTABLE,
  OC_STATUS_REQUEST_ENTITY_TOO_LARGE,
  OC_STATUS_UNSUPPORTED_MEDIA_TYPE,
  OC_STATUS_INTERNAL_SERVER_ERROR,
  OC_STATUS_NOT_IMPLEMENTED,
  OC_STATUS_BAD_GATEWAY,
  OC_STATUS_SERVICE_UNAVAILABLE,
  OC_STATUS_GATEWAY_TIMEOUT,
  OC_STATUS_PROXYING_NOT_SUPPORTED,
  __NUM_OC_STATUS_CODES__,
  OC_IGNORE
} oc_status_t;

struct oc_separate_response;
struct oc_response_buffer;

typedef struct oc_response {
    struct oc_separate_response *separate_response;
    struct oc_response_buffer *response_buffer;
} oc_response_t;

typedef enum {
  OC_IF_BASELINE = 1 << 1,
  OC_IF_LL = 1 << 2,
  OC_IF_B = 1 << 3,
  OC_IF_R = 1 << 4,
  OC_IF_RW = 1 << 5,
  OC_IF_A = 1 << 6,
  OC_IF_S = 1 << 7,
} oc_interface_mask_t;

typedef enum {
  OCF_RES = 0,
  OCF_P,
#ifdef OC_SECURITY
  OCF_SEC_DOXM,
  OCF_SEC_PSTAT,
  OCF_SEC_ACL,
  OCF_SEC_CRED,
#endif
  __NUM_OC_CORE_RESOURCES__
} oc_core_resource_t;

#define NUM_OC_CORE_RESOURCES (__NUM_OC_CORE_RESOURCES__ + MAX_NUM_DEVICES)

typedef struct oc_request {
    oc_endpoint_t *origin;
    struct oc_resource *resource;
    const char *query;
    int query_len;
    oc_response_t *response;
    struct coap_packet_rx *packet;
} oc_request_t;

typedef void (*oc_request_handler_t)(oc_request_t *, oc_interface_mask_t);

typedef struct oc_resource {
  SLIST_ENTRY(oc_resource) next;
  int device;
  oc_string_t uri;
  oc_string_array_t types;
  oc_interface_mask_t interfaces;
  oc_interface_mask_t default_interface;
  oc_resource_properties_t properties;
  oc_request_handler_t get_handler;
  oc_request_handler_t put_handler;
  oc_request_handler_t post_handler;
  oc_request_handler_t delete_handler;
  struct os_callout callout;
  uint32_t observe_period_mseconds;
  uint8_t num_observers;
} oc_resource_t;

void oc_ri_init(void);

void oc_ri_shutdown(void);

int oc_status_code(oc_status_t key);

oc_resource_t *oc_ri_get_app_resource_by_uri(const char *uri);

oc_resource_t *oc_ri_get_app_resources(void);

#ifdef OC_SERVER
oc_resource_t *oc_ri_alloc_resource(void);
bool oc_ri_add_resource(oc_resource_t *resource);
void oc_ri_delete_resource(oc_resource_t *resource);
#endif

int oc_ri_get_query_nth_key_value(const char *query, int query_len, char **key,
                                  int *key_len, char **value, int *value_len,
                                  int n);
int oc_ri_get_query_value(const char *query, int query_len, const char *key,
                          char **value);

oc_interface_mask_t oc_ri_get_interface_mask(char *iface, int if_len);

struct coap_packet_rx;
struct coap_packet;
bool oc_ri_invoke_coap_entity_handler(struct coap_packet_rx *request,
                                      struct coap_packet *response,
                                      int32_t *offset,
                                      oc_endpoint_t *endpoint);

#ifdef __cplusplus
}
#endif

#endif /* OC_RI_H */
