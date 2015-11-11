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

    METRIC - TODO THERE WILL BE SOME DESCRPTION



MVY: To see if we can handle the need for ymsg wrapper by header
     Field aux is going to be added in each message
        aux                 hash        
        type                string      
        Type of metric send (temperature, humidity, power.load, ...)
    
        element_src         string      
        Name of source element to which metrics are bind to.
    
        value               string      
        Value of metric as plain string
    
        unit                string      
        Unit of metric (C, F or K for temperature)
    
        time                number 8    
        Metric date/time, -1 will be replaced by actual time on receiving side.
    

    ALERT - TODO THERE WILL BE SOME DESCRPTION



MVY: To see if we can handle the need for ymsg wrapper by header
     Field aux is going to be added in each message
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
        ALERT date/time, -1 will be replaced by actual time on receiving side by current time.
    
        action              string      
        list of strings separated by "/" ( EMAIL/SMS ) ( is optional and can be empty )
    
*/

#define BIOS_PROTO_VERSION                  1
#define BIOS_PROTO_METRIC_ELEMENT_DEST      "element-dest"

#define BIOS_PROTO_METRIC                   1
#define BIOS_PROTO_ALERT                    2

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
        uint64_t time);

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


//  Send the METRIC to the output in one step
//  WARNING, this call will fail if output is of type ZMQ_ROUTER.
int
    bios_proto_send_metric (void *output,
        zhash_t *aux,
        const char *type,
        const char *element_src,
        const char *value,
        const char *unit,
        uint64_t time);
    
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

//  Get/set the time field
uint64_t
    bios_proto_time (bios_proto_t *self);
void
    bios_proto_set_time (bios_proto_t *self, uint64_t time);

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

//  Get/set the action field
const char *
    bios_proto_action (bios_proto_t *self);
void
    bios_proto_set_action (bios_proto_t *self, const char *format, ...);

//  Self test of this class
int
    bios_proto_test (bool verbose);
//  @end

//  For backwards compatibility with old codecs
#define bios_proto_dump     bios_proto_print

#ifdef __cplusplus
}
#endif

#endif
