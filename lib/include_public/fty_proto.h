/*  =========================================================================
    fty_proto - Core FTY protocols

    Codec header for fty_proto.
    Copyright (C) 2014 - 2020 Eaton

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

#ifndef FTY_PROTO_H_INCLUDED
#define FTY_PROTO_H_INCLUDED

/*
    The software maintains three main types of information divided to three streams.
    These are the fty_proto messages for:


    1. Stream: METRICS - dynamic information about metric data coming from varous devices

    METRIC - FTY core protocols

        aux                 hash
        time                number 8     Metric timestamp -
        unixtime.

        ttl                 number 4
        Time to live in seconds.

        Value defines duration of validity (i.e. how long is metric valid,
        when it expires) as well as the latest time we should receive metric
        updates.

        type                string
        Metric name, e.g.: "temperature", "humidity", "power.load", ...

        name                string
        Name of asset where metric is produced.

        value               string
        Metric value, e.g.: "25.323" or "900".

        unit                string
        Metric unit, e.g.: "C" or "F" for temperature.


    2. Stream: ALERTS - information about alerts produced for a given asset based on metric information

    ALERT - FTY core protocols

        aux                 hash
        time                number 8
        Alert timestamp - unixtime.

        ttl                 number 4
        Time to live in seconds.

        Value defines duration of validity (i.e. how long is alert valid, when
        it expires).
        When current unixtime is greater than time + ttl this alert expired
        and is no longer valid.

        rule                string
        Name of the rule which triggers this alert.

        name                string
        Name of asset where alert is trigged.

        state               string
        Alert state.

        Permissible values:
        ACTIVE / ACK-WIP / ACK-IGNORE / ACK-PAUSE / ACK-SILENCE / RESOLVED

        severity            string
        Alert severity.

        Permissible values:
        INFO / WARNING / CRITICAL

        description         string
        Alert description.

        metadata            string
        Alert metadata.
        Opaque string payload (unspecified) passing through codec and zpl persistency.

        action              strings
        List of actions, e.g.: "EMAIL", "SMS".
        Can be empty.


    3. Stream: ASSETS - semi-static data about changes in assets, see ASSET message

    ASSET - FTY core protocols

        aux                 hash
        name                string
        Asset name.

        operation           string
        Asset operation.

        Permissible values:
        create / update / delete / inventory / retire

        A little explanation. Due to historical reasons:
        * 'update' is actually 'delete' + 'create', so only it is essentially
           a 'replace' operation: only data passed in this message become the
           new content of asset information.
        * 'delete' is the same as 'retire'
        * 'inventory' is update of extended and/or auxilliary information
          (ext/aux field)

        ext                 hash
        Additional extended information.

*/

//  FTY_PROTO version macros for compile-time API detection
#define FTY_PROTO_VERSION_MAJOR 1
#define FTY_PROTO_VERSION_MINOR 0
#define FTY_PROTO_VERSION_PATCH 0

#define FTY_PROTO_MAKE_VERSION(major, minor, patch) \
    ((major) * 10000 + (minor) * 100 + (patch))
#define FTY_PROTO_VERSION \
    FTY_PROTO_MAKE_VERSION(FTY_PROTO_VERSION_MAJOR, FTY_PROTO_VERSION_MINOR, FTY_PROTO_VERSION_PATCH)


#define FTY_PROTO_STREAM_METRICS            "METRICS"
#define FTY_PROTO_STREAM_ALERTS             "ALERTS"
#define FTY_PROTO_STREAM_ASSETS             "ASSETS"
#define FTY_PROTO_STREAM_ALERTS_SYS         "_ALERTS_SYS"
#define FTY_PROTO_STREAM_METRICS_SENSOR     "_METRICS_SENSOR"
#define FTY_PROTO_STREAM_METRICS_UNAVAILABLE  "_METRICS_UNAVAILABLE"
#define FTY_PROTO_STREAM_EULA               "_EULA"
#define FTY_PROTO_STREAM_LICENSING_ANNOUNCEMENTS  "LICENSING-ANNOUNCEMENTS"
#define FTY_PROTO_ASSET_TYPE                "type"
#define FTY_PROTO_ASSET_SUBTYPE             "subtype"
#define FTY_PROTO_ASSET_STATUS              "status"
#define FTY_PROTO_ASSET_PRIORITY            "priority"
#define FTY_PROTO_ASSET_PARENT              "parent"
#define FTY_PROTO_ASSET_OP_CREATE           "create"
#define FTY_PROTO_ASSET_OP_UPDATE           "update"
#define FTY_PROTO_ASSET_OP_DELETE           "delete"
#define FTY_PROTO_ASSET_OP_RETIRE           "retire"
#define FTY_PROTO_ASSET_OP_INVENTORY        "inventory"
#define FTY_PROTO_ASSET_AUX_TYPE            "type"
#define FTY_PROTO_ASSET_AUX_SUBTYPE         "subtype"
#define FTY_PROTO_ASSET_AUX_PARENT_NAME_1   "parent_name.1"
#define FTY_PROTO_ASSET_EXT_PORT            "port"
#define FTY_PROTO_ASSET_EXT_MODEL           "model"
#define FTY_PROTO_ASSET_EXT_DEVICE_PART     "device.part"
#define FTY_PROTO_RULE_CLASS                "rule_class"
#define FTY_PROTO_METRIC_ELEMENT_DEST       "element-dest"
#define FTY_PROTO_METRICS_AUX_PORT          "port"
#define FTY_PROTO_METRICS_SENSOR_AUX_SNAME  "sname"
#define FTY_PROTO_METRICS_SENSOR_AUX_PORT   "port"

#define FTY_PROTO_METRIC                    1
#define FTY_PROTO_ALERT                     2
#define FTY_PROTO_ASSET                     3

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

//  Opaque class structure
#ifndef FTY_PROTO_T_DEFINED
typedef struct _fty_proto_t fty_proto_t;
#define FTY_PROTO_T_DEFINED
#endif

//  @warning THE FOLLOWING @INTERFACE BLOCK IS AUTO-GENERATED BY ZPROJECT
//  @warning Please edit the model at "api/fty_proto.api" to make changes.
//  @interface
//  This is a stable class, and may not change except for emergencies. It
//  is provided in stable builds.
//  Create a new fty_proto
fty_proto_t *
    fty_proto_new (int id);

//  Create a new fty_proto from zpl/zconfig_t *
fty_proto_t *
    fty_proto_new_zpl (zconfig_t *config);

//  Destroy the fty_proto
void
    fty_proto_destroy (fty_proto_t **self_p);

//  Parses a zmsg_t and decides whether it carries a fty_proto.
//  Returns true if it does, false otherwise.
//  Doesn't destroy or modify the original message.
bool
    fty_proto_is (zmsg_t *msg_p);

//  Parse a fty_proto from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and
//  nullifies the msg reference.
fty_proto_t *
    fty_proto_decode (zmsg_t **msg_p);

//  Encode fty_proto into zmsg and destroy it. Returns a newly created
//  object or NULL if error. Use when not in control of sending the message.
zmsg_t *
    fty_proto_encode (fty_proto_t **self_p);

//  Receive and parse a fty_proto from the socket. Returns new object,
//  or NULL if error. Will block if there's no message waiting.
fty_proto_t *
    fty_proto_recv (void *input);

//  Receive and parse a fty_proto from the socket. Returns new object,
//  or NULL either if there was no input waiting, or the recv was interrupted.
fty_proto_t *
    fty_proto_recv_nowait (void *input);

//  Send the fty_proto to the output, and destroy it
int
    fty_proto_send (fty_proto_t **self_p, void *output);

//  Send the fty_proto to the output, and do not destroy it
int
    fty_proto_send_again (fty_proto_t *self, void *output);

//  Encode the METRIC
zmsg_t *
    fty_proto_encode_metric (
        zhash_t *aux,
        uint64_t time,
        uint32_t ttl,
        const char *type,
        const char *name,
        const char *value,
        const char *unit);

//  Encode the ALERT
zmsg_t *
    fty_proto_encode_alert (
        zhash_t *aux,
        uint64_t time,
        uint32_t ttl,
        const char *rule,
        const char *name,
        const char *state,
        const char *severity,
        const char *description,
        zlist_t *action);

//  Encode the ASSET
zmsg_t *
    fty_proto_encode_asset (
        zhash_t *aux,
        const char *name,
        const char *operation,
        zhash_t *ext);


//  Send the METRIC to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    fty_proto_send_metric (void *output,
        zhash_t *aux,
        uint64_t time,
        uint32_t ttl,
        const char *type,
        const char *name,
        const char *value,
        const char *unit);

//  Send the ALERT to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    fty_proto_send_alert (void *output,
        zhash_t *aux,
        uint64_t time,
        uint32_t ttl,
        const char *rule,
        const char *name,
        const char *state,
        const char *severity,
        const char *description,
        zlist_t *action);

//  Send the ASSET to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    fty_proto_send_asset (void *output,
        zhash_t *aux,
        const char *name,
        const char *operation,
        zhash_t *ext);

//  Duplicate the fty_proto message
fty_proto_t *
    fty_proto_dup (fty_proto_t *self);

//  Print contents of message to stdout
void
    fty_proto_print (fty_proto_t *self);

//  Export class as zconfig_t*. Caller is responsibe for destroying the instance
zconfig_t *
    fty_proto_zpl (fty_proto_t *self, zconfig_t* parent);

//  Get/set the message routing id
zframe_t *
    fty_proto_routing_id (fty_proto_t *self);
void
    fty_proto_set_routing_id (fty_proto_t *self, zframe_t *routing_id);

//  Get the fty_proto id and printable command
int
    fty_proto_id (fty_proto_t *self);
void
    fty_proto_set_id (fty_proto_t *self, int id);
const char *
    fty_proto_command (fty_proto_t *self);

//  Get/set the aux field
zhash_t *
    fty_proto_aux (fty_proto_t *self);
//  Get the aux field and transfer ownership to caller
zhash_t *
    fty_proto_get_aux (fty_proto_t *self);
//  Set the aux field, transferring ownership from caller
void
    fty_proto_set_aux (fty_proto_t *self, zhash_t **aux_p);

//  Get/set a value in the aux dictionary
const char *
    fty_proto_aux_string (fty_proto_t *self,
        const char *key, const char *default_value);
uint64_t
    fty_proto_aux_number (fty_proto_t *self,
        const char *key, uint64_t default_value);
void
    fty_proto_aux_insert (fty_proto_t *self,
        const char *key, const char *format, ...);
size_t
    fty_proto_aux_size (fty_proto_t *self);

//  Get/set the time field
uint64_t
    fty_proto_time (fty_proto_t *self);
void
    fty_proto_set_time (fty_proto_t *self, uint64_t time);

//  Get/set the ttl field
uint32_t
    fty_proto_ttl (fty_proto_t *self);
void
    fty_proto_set_ttl (fty_proto_t *self, uint32_t ttl);

//  Get/set the type field
const char *
    fty_proto_type (fty_proto_t *self);
void
    fty_proto_set_type (fty_proto_t *self, const char *format, ...);

//  Get/set the name field
const char *
    fty_proto_name (fty_proto_t *self);
void
    fty_proto_set_name (fty_proto_t *self, const char *format, ...);

//  Get/set the value field
const char *
    fty_proto_value (fty_proto_t *self);
void
    fty_proto_set_value (fty_proto_t *self, const char *format, ...);

//  Get/set the unit field
const char *
    fty_proto_unit (fty_proto_t *self);
void
    fty_proto_set_unit (fty_proto_t *self, const char *format, ...);

//  Get/set the rule field
const char *
    fty_proto_rule (fty_proto_t *self);
void
    fty_proto_set_rule (fty_proto_t *self, const char *format, ...);

//  Get/set the state field
const char *
    fty_proto_state (fty_proto_t *self);
void
    fty_proto_set_state (fty_proto_t *self, const char *format, ...);

//  Get/set the severity field
const char *
    fty_proto_severity (fty_proto_t *self);
void
    fty_proto_set_severity (fty_proto_t *self, const char *format, ...);

//  Get/set the description field
const char *
    fty_proto_description (fty_proto_t *self);
void
    fty_proto_set_description (fty_proto_t *self, const char *format, ...);

//  Get/set the metadata field
const char *
    fty_proto_metadata (fty_proto_t *self);
void
    fty_proto_set_metadata (fty_proto_t *self, const char *format, ...);

//  Get/set the action field
zlist_t *
    fty_proto_action (fty_proto_t *self);
//  Get the action field and transfer ownership to caller
zlist_t *
    fty_proto_get_action (fty_proto_t *self);
//  Set the action field, transferring ownership from caller
void
    fty_proto_set_action (fty_proto_t *self, zlist_t **action_p);

//  Iterate through the action field, and append a action value
const char *
    fty_proto_action_first (fty_proto_t *self);
const char *
    fty_proto_action_next (fty_proto_t *self);
void
    fty_proto_action_append (fty_proto_t *self, const char *format, ...);
size_t
    fty_proto_action_size (fty_proto_t *self);

//  Get/set the operation field
const char *
    fty_proto_operation (fty_proto_t *self);
void
    fty_proto_set_operation (fty_proto_t *self, const char *format, ...);

//  Get/set the ext field
zhash_t *
    fty_proto_ext (fty_proto_t *self);
//  Get the ext field and transfer ownership to caller
zhash_t *
    fty_proto_get_ext (fty_proto_t *self);
//  Set the ext field, transferring ownership from caller
void
    fty_proto_set_ext (fty_proto_t *self, zhash_t **ext_p);

//  Get/set a value in the ext dictionary
const char *
    fty_proto_ext_string (fty_proto_t *self,
        const char *key, const char *default_value);
uint64_t
    fty_proto_ext_number (fty_proto_t *self,
        const char *key, uint64_t default_value);
void
    fty_proto_ext_insert (fty_proto_t *self,
        const char *key, const char *format, ...);
size_t
    fty_proto_ext_size (fty_proto_t *self);

//  Self test of this class
void
    fty_proto_test (bool verbose);

//  @end

//  For backwards compatibility with old codecs
#define fty_proto_dump fty_proto_print
#define is_fty_proto fty_proto_is

#ifdef __cplusplus
}
#endif

#endif