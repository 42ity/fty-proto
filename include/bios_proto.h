/*  =========================================================================
    bios_proto - Core BIOS protocols

    Codec header for bios_proto.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: bios_proto.xml, or
     * The code generation script that built this file: zproto_codec_c_v1
    ************************************************************************
    Copyright (C) 2014 - 2015 Eaton

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
    =========================================================================
*/

#ifndef BIOS_PROTO_H_INCLUDED
#define BIOS_PROTO_H_INCLUDED

/*  These are the bios_proto messages:

    METRIC - BIOS core protocols

The software maintains three main types of information divided to three streams

1. Stream: ASSETS - semi-static data about changes in assets, see ASSET message
2. Stream: METRICS - dynamic information about metric data coming from varous devices
3. Stream: ALERTS - information about alerts produced for given asset based on metric information
        aux                 hash
        type                string
        Type of metric send (temperature, humidity, power.load, ...)

        element_src         string
        Name of source element to which metrics are bound to

        value               string
        Value of metric as plain string

        unit                string
        Unit of metric (i.e. C, F or K for temperature)

        ttl                 number 4
        Metric time to live seconds (i.e. How long is the metric valid - At the latest how long from now should i get a new one)


    ALERT - BIOS core protocols

The software maintains three main types of information divided to three streams

1. Stream: ASSETS - semi-static data about changes in assets, see ASSET message
2. Stream: METRICS - dynamic information about metric data coming from varous devices
3. Stream: ALERTS - information about alerts produced for given asset based on metric information
        aux                 hash
        rule                string
        a rule name, that triggers this alert

        element_src         string
        name of the element, where alert was detected. Most of the time is would be asset element name

        state               string
        state of the alert. Possible values are ACTIVE/ACK-WIP/ACK-IGNORE/ACK-PAUSE/ACK-SILENCE/RESOLVED

        severity            string
        severity of the alert. Possible values are INFO/WARNING/CRITICAL

        description         string
        a description of the alert

        time                number 8
        ALERT date/time

        action              string
        list of strings separated by "/" ( EMAIL/SMS ) ( is optional and can be empty )


    ASSET - BIOS core protocols

The software maintains three main types of information divided to three streams

1. Stream: ASSETS - semi-static data about changes in assets, see ASSET message
2. Stream: METRICS - dynamic information about metric data coming from varous devices
3. Stream: ALERTS - information about alerts produced for given asset based on metric information
        aux                 hash
        name                string
        Unique name of asset.

        operation           string
        What have hapened with asset (create|update|delete|inventory).

        ext                 hash
        Additional extended information for assets.

*/

#define BIOS_PROTO_VERSION                  1
#define BIOS_PROTO_STREAM_METRICS           "METRICS"
#define BIOS_PROTO_STREAM_ALERTS            "ALERTS"
#define BIOS_PROTO_STREAM_ASSETS            "ASSETS"
#define BIOS_PROTO_STREAM_ALERTS_SYS        "_ALERTS_SYS"
#define BIOS_PROTO_STREAM_METRICS_SENSOR    "_METRICS_SENSOR"
#define BIOS_PROTO_STREAM_METRICS_UNAVAILABLE   "_METRICS_UNAVAILABLE"
#define BIOS_PROTO_METRIC_ELEMENT_DEST      "element-dest"
#define BIOS_PROTO_ASSET_TYPE               "type"
#define BIOS_PROTO_ASSET_SUBTYPE            "subtype"
#define BIOS_PROTO_ASSET_STATUS             "status"
#define BIOS_PROTO_ASSET_PRIORITY           "priority"
#define BIOS_PROTO_ASSET_PARENT             "parent"
#define BIOS_PROTO_ASSET_OP_CREATE          "create"
#define BIOS_PROTO_ASSET_OP_UPDATE          "update"
#define BIOS_PROTO_ASSET_OP_DELETE          "delete"
#define BIOS_PROTO_ASSET_OP_RETIRE          "retire"
#define BIOS_PROTO_ASSET_OP_INVENTORY       "inventory"
#define BIOS_PROTO_RULE_CLASS               "rule_class"

#define BIOS_PROTO_METRIC                   1
#define BIOS_PROTO_ALERT                    2
#define BIOS_PROTO_ASSET                    3

#include <czmq.h>


#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef BIOS_PROTO_T_DEFINED
typedef struct _bios_proto_t bios_proto_t;
#define BIOS_PROTO_T_DEFINED
#endif

//  @interface
//  Create a new bios_proto
bios_proto_t *
    bios_proto_new (int id);

//  Destroy the bios_proto
void
    bios_proto_destroy (bios_proto_t **self_p);

//  Parse a zmsg_t and decides whether it is bios_proto. Returns
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.
bool
    is_bios_proto (zmsg_t *msg_p);

//  Parse a bios_proto from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and
//  nullifies the msg reference.
bios_proto_t *
    bios_proto_decode (zmsg_t **msg_p);

//  Encode bios_proto into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    bios_proto_encode (bios_proto_t **self_p);

//  Receive and parse a bios_proto from the socket. Returns new object,
//  or NULL if error. Will block if there's no message waiting.
bios_proto_t *
    bios_proto_recv (void *input);

//  Receive and parse a bios_proto from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.
bios_proto_t *
    bios_proto_recv_nowait (void *input);

//  Send the bios_proto to the output, and destroy it
int
    bios_proto_send (bios_proto_t **self_p, void *output);

//  Send the bios_proto to the output, and do not destroy it
int
    bios_proto_send_again (bios_proto_t *self, void *output);

//  Encode the METRIC
zmsg_t *
    bios_proto_encode_metric (
        zhash_t *aux,
        const char *type,
        const char *element_src,
        const char *value,
        const char *unit,
        uint32_t ttl);

//  Encode the ALERT
zmsg_t *
    bios_proto_encode_alert (
        zhash_t *aux,
        const char *rule,
        const char *element_src,
        const char *state,
        const char *severity,
        const char *description,
        uint64_t time,
        const char *action);

//  Encode the ASSET
zmsg_t *
    bios_proto_encode_asset (
        zhash_t *aux,
        const char *name,
        const char *operation,
        zhash_t *ext);


//  Send the METRIC to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    bios_proto_send_metric (void *output,
        zhash_t *aux,
        const char *type,
        const char *element_src,
        const char *value,
        const char *unit,
        uint32_t ttl);

//  Send the ALERT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    bios_proto_send_alert (void *output,
        zhash_t *aux,
        const char *rule,
        const char *element_src,
        const char *state,
        const char *severity,
        const char *description,
        uint64_t time,
        const char *action);

//  Send the ASSET to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    bios_proto_send_asset (void *output,
        zhash_t *aux,
        const char *name,
        const char *operation,
        zhash_t *ext);

//  Duplicate the bios_proto message
bios_proto_t *
    bios_proto_dup (bios_proto_t *self);

//  Print contents of message to stdout
void
    bios_proto_print (bios_proto_t *self);

//  Get/set the message routing id
zframe_t *
    bios_proto_routing_id (bios_proto_t *self);
void
    bios_proto_set_routing_id (bios_proto_t *self, zframe_t *routing_id);

//  Get the bios_proto id and printable command
int
    bios_proto_id (bios_proto_t *self);
void
    bios_proto_set_id (bios_proto_t *self, int id);
const char *
    bios_proto_command (bios_proto_t *self);

//  Get/set the aux field
zhash_t *
    bios_proto_aux (bios_proto_t *self);
//  Get the aux field and transfer ownership to caller
zhash_t *
    bios_proto_get_aux (bios_proto_t *self);
//  Set the aux field, transferring ownership from caller
void
    bios_proto_set_aux (bios_proto_t *self, zhash_t **aux_p);

//  Get/set a value in the aux dictionary
const char *
    bios_proto_aux_string (bios_proto_t *self,
        const char *key, const char *default_value);
uint64_t
    bios_proto_aux_number (bios_proto_t *self,
        const char *key, uint64_t default_value);
void
    bios_proto_aux_insert (bios_proto_t *self,
        const char *key, const char *format, ...);
size_t
    bios_proto_aux_size (bios_proto_t *self);

//  Get/set the type field
const char *
    bios_proto_type (bios_proto_t *self);
void
    bios_proto_set_type (bios_proto_t *self, const char *format, ...);

//  Get/set the element_src field
const char *
    bios_proto_element_src (bios_proto_t *self);
void
    bios_proto_set_element_src (bios_proto_t *self, const char *format, ...);

//  Get/set the value field
const char *
    bios_proto_value (bios_proto_t *self);
void
    bios_proto_set_value (bios_proto_t *self, const char *format, ...);

//  Get/set the unit field
const char *
    bios_proto_unit (bios_proto_t *self);
void
    bios_proto_set_unit (bios_proto_t *self, const char *format, ...);

//  Get/set the ttl field
uint32_t
    bios_proto_ttl (bios_proto_t *self);
void
    bios_proto_set_ttl (bios_proto_t *self, uint32_t ttl);

//  Get/set the rule field
const char *
    bios_proto_rule (bios_proto_t *self);
void
    bios_proto_set_rule (bios_proto_t *self, const char *format, ...);

//  Get/set the state field
const char *
    bios_proto_state (bios_proto_t *self);
void
    bios_proto_set_state (bios_proto_t *self, const char *format, ...);

//  Get/set the severity field
const char *
    bios_proto_severity (bios_proto_t *self);
void
    bios_proto_set_severity (bios_proto_t *self, const char *format, ...);

//  Get/set the description field
const char *
    bios_proto_description (bios_proto_t *self);
void
    bios_proto_set_description (bios_proto_t *self, const char *format, ...);

//  Get/set the time field
uint64_t
    bios_proto_time (bios_proto_t *self);
void
    bios_proto_set_time (bios_proto_t *self, uint64_t time);

//  Get/set the action field
const char *
    bios_proto_action (bios_proto_t *self);
void
    bios_proto_set_action (bios_proto_t *self, const char *format, ...);

//  Get/set the name field
const char *
    bios_proto_name (bios_proto_t *self);
void
    bios_proto_set_name (bios_proto_t *self, const char *format, ...);

//  Get/set the operation field
const char *
    bios_proto_operation (bios_proto_t *self);
void
    bios_proto_set_operation (bios_proto_t *self, const char *format, ...);

//  Get/set the ext field
zhash_t *
    bios_proto_ext (bios_proto_t *self);
//  Get the ext field and transfer ownership to caller
zhash_t *
    bios_proto_get_ext (bios_proto_t *self);
//  Set the ext field, transferring ownership from caller
void
    bios_proto_set_ext (bios_proto_t *self, zhash_t **ext_p);

//  Get/set a value in the ext dictionary
const char *
    bios_proto_ext_string (bios_proto_t *self,
        const char *key, const char *default_value);
uint64_t
    bios_proto_ext_number (bios_proto_t *self,
        const char *key, uint64_t default_value);
void
    bios_proto_ext_insert (bios_proto_t *self,
        const char *key, const char *format, ...);
size_t
    bios_proto_ext_size (bios_proto_t *self);

//  Self test of this class
void
    bios_proto_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define bios_proto_dump     bios_proto_print

#ifdef __cplusplus
}
#endif

#endif
