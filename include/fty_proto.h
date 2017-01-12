/*  =========================================================================
    fty_proto - Core FTY protocols

    Codec header for fty_proto.

    ** WARNING *************************************************************
    THIS SOURCE FILE IS 100% GENERATED. If you edit this file, you will lose
    your changes at the next build cycle. This is great for temporary printf
    statements. DO NOT MAKE ANY CHANGES YOU WISH TO KEEP. The correct places
    for commits are:

     * The XML model used for this code generation: fty_proto.xml, or
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

#ifndef FTY_PROTO_H_INCLUDED
#define FTY_PROTO_H_INCLUDED

/*  These are the fty_proto messages:

    METRIC - FTY core protocols

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
    

    ALERT - FTY core protocols

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
    

    ASSET - FTY core protocols

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

#define FTY_PROTO_STREAM_METRICS            "METRICS"
#define FTY_PROTO_STREAM_ALERTS             "ALERTS"
#define FTY_PROTO_STREAM_ASSETS             "ASSETS"
#define FTY_PROTO_STREAM_ALERTS_SYS         "_ALERTS_SYS"
#define FTY_PROTO_STREAM_METRICS_SENSOR     "_METRICS_SENSOR"
#define FTY_PROTO_STREAM_METRICS_UNAVAILABLE  "_METRICS_UNAVAILABLE"
#define FTY_PROTO_METRIC_ELEMENT_DEST       "element-dest"
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
#define FTY_PROTO_RULE_CLASS                "rule_class"

#define FTY_PROTO_METRIC                    1
#define FTY_PROTO_ALERT                     2
#define FTY_PROTO_ASSET                     3

#include <czmq.h>

#include "ftyproto.h"

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
//  Destroy the fty_proto
FTY_PROTO_EXPORT void
    fty_proto_destroy (fty_proto_t **self_p);

//  Parse a zmsg_t and decides whether it is fty_proto. Returns  
//  true if it is, false otherwise. Doesn't destroy or modify the
//  original message.                                            
FTY_PROTO_EXPORT bool
    fty_proto_is (zmsg_t *msg);

//  Parse a fty_proto from zmsg_t. Returns a new object, or NULL if
//  the message could not be parsed, or was NULL. Destroys msg and 
//  nullifies the msg reference.                                   
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT fty_proto_t *
    fty_proto_decode (zmsg_t **msg_p);

//  Encode fty_proto into zmsg and destroy it. Returns a newly created      
//  object or NULL if error. Use when not in control of sending the message.
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zmsg_t *
    fty_proto_encode (fty_proto_t **self_p);

//  Receive and parse a fty_proto from the socket. Returns new object,
//  or NULL if error. Will block if there's no message waiting.       
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT fty_proto_t *
    fty_proto_recv (void *input);

//  Receive and parse a fty_proto from the socket. Returns new object,        
//  or NULL either if there was no input waiting, or the recv was interrupted.
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT fty_proto_t *
    fty_proto_recv_nowait (void *input);

//  Send the fty_proto to the output, and destroy it
FTY_PROTO_EXPORT int
    fty_proto_send (fty_proto_t **self_p, void *output);

//  Send the fty_proto to the output, and do not destroy it
FTY_PROTO_EXPORT int
    fty_proto_send_again (fty_proto_t *self, void *output);

//  Encode the METRIC
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zmsg_t *
    fty_proto_encode_metric (zhash_t *aux, const char *type, const char *element_src, const char *value, const char *unit, uint32_t ttl);

//  Encode the ALERT
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zmsg_t *
    fty_proto_encode_alert (zhash_t *aux, const char *rule, const char *element_src, const char *state, const char *severity, const char *description, uint64_t time, const char *action);

//  Encode the ASSET
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zmsg_t *
    fty_proto_encode_asset (zhash_t *aux, const char *name, const char *operation, zhash_t *ext);

//  Send the METRIC to the output in one step                    
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
FTY_PROTO_EXPORT int
    fty_proto_send_metric (void *output, zhash_t *aux, const char *type, const char *element_src, const char *value, const char *unit, uint32_t ttl);

//  Send the ALERT to the output in one step                     
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
FTY_PROTO_EXPORT int
    fty_proto_send_alert (void *output, zhash_t *aux, const char *rule, const char *element_src, const char *state, const char *severity, const char *description, uint64_t time, const char *action);

//  Send the ASSET to the output in one step                     
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
FTY_PROTO_EXPORT int
    fty_proto_send_asset (void *output, zhash_t *aux, const char *name, const char *operation, zhash_t *ext);

//  Duplicate the fty_proto message
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT fty_proto_t *
    fty_proto_dup (fty_proto_t *self);

//  Print contents of message to stdout
FTY_PROTO_EXPORT void
    fty_proto_print (fty_proto_t *self);

//  Export class as zconfig_t*. Caller is responsibe for destroying the instance
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zconfig_t *
    fty_proto_zpl (fty_proto_t *self, zconfig_t *parent);

//  Get the message routing id
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zframe_t *
    fty_proto_routing_id (fty_proto_t *self);

//  Set the message routing id
FTY_PROTO_EXPORT void
    fty_proto_set_routing_id (fty_proto_t *self, zframe_t *routing_id);

//  Get the fty_proto id
FTY_PROTO_EXPORT int
    fty_proto_id (fty_proto_t *self);

//  Set the fty_proto id
FTY_PROTO_EXPORT void
    fty_proto_set_id (fty_proto_t *self, int id);

//  Return the printable command
FTY_PROTO_EXPORT const char *
    fty_proto_command (fty_proto_t *self);

//  Get/set the aux field
FTY_PROTO_EXPORT zhash_t *
    fty_proto_aux (fty_proto_t *self);

//  Get/set the aux field and transfer ownership to caller
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zhash_t *
    fty_proto_get_aux (fty_proto_t *self);

//  Get/set the aux field, transferring ownership from caller
FTY_PROTO_EXPORT void
    fty_proto_set_aux (fty_proto_t *self, zhash_t **aux_p);

//  Get/set a value in the aux dictionary
FTY_PROTO_EXPORT const char *
    fty_proto_aux_string (fty_proto_t *self, const char *key, const char *default_value);

//  Get/set a value in the aux dictionary
FTY_PROTO_EXPORT uint64_t
    fty_proto_aux_number (fty_proto_t *self, const char *key, uint64_t default_value);

//  Get/set a value in the aux dictionary
FTY_PROTO_EXPORT void
    fty_proto_aux_insert (fty_proto_t *self, const char *key, const char *format, ...) CHECK_PRINTF (3);

//  Get/set a value in the aux dictionary
FTY_PROTO_EXPORT size_t
    fty_proto_aux_size (fty_proto_t *self);

//  Get/set the type field
FTY_PROTO_EXPORT const char *
    fty_proto_type (fty_proto_t *self);

//  Get/set the type field
FTY_PROTO_EXPORT void
    fty_proto_set_type (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the element_src field
FTY_PROTO_EXPORT const char *
    fty_proto_element_src (fty_proto_t *self);

//  Get/set the element_src field
FTY_PROTO_EXPORT void
    fty_proto_set_element_src (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the value field
FTY_PROTO_EXPORT const char *
    fty_proto_value (fty_proto_t *self);

//  Get/set the value field
FTY_PROTO_EXPORT void
    fty_proto_set_value (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the unit field
FTY_PROTO_EXPORT const char *
    fty_proto_unit (fty_proto_t *self);

//  Get/set the unit field
FTY_PROTO_EXPORT void
    fty_proto_set_unit (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the ttl field
FTY_PROTO_EXPORT uint32_t
    fty_proto_ttl (fty_proto_t *self);

//  Get/set the ttl field
FTY_PROTO_EXPORT void
    fty_proto_set_ttl (fty_proto_t *self, uint32_t ttl);

//  Get/set the rule field
FTY_PROTO_EXPORT const char *
    fty_proto_rule (fty_proto_t *self);

//  Get/set the rule field
FTY_PROTO_EXPORT void
    fty_proto_set_rule (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the state field
FTY_PROTO_EXPORT const char *
    fty_proto_state (fty_proto_t *self);

//  Get/set the state field
FTY_PROTO_EXPORT void
    fty_proto_set_state (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the severity field
FTY_PROTO_EXPORT const char *
    fty_proto_severity (fty_proto_t *self);

//  Get/set the severity field
FTY_PROTO_EXPORT void
    fty_proto_set_severity (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the description field
FTY_PROTO_EXPORT const char *
    fty_proto_description (fty_proto_t *self);

//  Get/set the description field
FTY_PROTO_EXPORT void
    fty_proto_set_description (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the time field
FTY_PROTO_EXPORT uint64_t
    fty_proto_time (fty_proto_t *self);

//  Get/set the time field
FTY_PROTO_EXPORT void
    fty_proto_set_time (fty_proto_t *self, uint64_t time);

//  Get/set the action field
FTY_PROTO_EXPORT const char *
    fty_proto_action (fty_proto_t *self);

//  Get/set the action field
FTY_PROTO_EXPORT void
    fty_proto_set_action (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the name field
FTY_PROTO_EXPORT const char *
    fty_proto_name (fty_proto_t *self);

//  Get/set the name field
FTY_PROTO_EXPORT void
    fty_proto_set_name (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the operation field
FTY_PROTO_EXPORT const char *
    fty_proto_operation (fty_proto_t *self);

//  Get/set the operation field
FTY_PROTO_EXPORT void
    fty_proto_set_operation (fty_proto_t *self, const char *format, ...) CHECK_PRINTF (2);

//  Get/set the ext field
FTY_PROTO_EXPORT zhash_t *
    fty_proto_ext (fty_proto_t *self);

//  Get/set the ext field and transfer ownership to caller
//  Caller owns return value and must destroy it when done.
FTY_PROTO_EXPORT zhash_t *
    fty_proto_get_ext (fty_proto_t *self);

//  Get/set the ext field, transferring ownership from caller
FTY_PROTO_EXPORT void
    fty_proto_set_ext (fty_proto_t *self, zhash_t **ext_p);

//  Get/set a value in the ext dictionary
FTY_PROTO_EXPORT const char *
    fty_proto_ext_string (fty_proto_t *self, const char *key, const char *default_value);

//  Get/set a value in the ext dictionary
FTY_PROTO_EXPORT uint64_t
    fty_proto_ext_number (fty_proto_t *self, const char *key, uint64_t default_value);

//  Get/set a value in the ext dictionary
FTY_PROTO_EXPORT void
    fty_proto_ext_insert (fty_proto_t *self, const char *key, const char *format, ...) CHECK_PRINTF (3);

//  Get/set a value in the ext dictionary
FTY_PROTO_EXPORT size_t
    fty_proto_ext_size (fty_proto_t *self);

//  Self test of this class
FTY_PROTO_EXPORT void
    fty_proto_test (bool verbose);

//  @end

//  For backwards compatibility with old codecs
#define fty_proto_dump      fty_proto_print
#define is_fty_proto fty_proto_is

#ifdef __cplusplus
}
#endif

#endif

