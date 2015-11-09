/*
 *
 * Copyright (C) 2015 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

// implements new BIOS core protocols

#pragma once

#include <malamute.h>

#ifdef __cplusplus
extern "C" {
#endif

//  @interface
//  Send a metric message to configured malamute client or zsock_t. Caller is
//  responsible for correct initialization of mlm_client.
//  type   - type of metric (temperature, humidity, ...)
//  source - source device issuing it
//  value  - actual value as a string
//  timestamp - when it happened, -1 is converted to actual time on receiver's side
//  dest   - destinating device, or NULL
int metric_send (
        void *dest,
        const char *type,
        const char *element_src,
        const char *value,
        const char *unit,
        int64_t  timestamp,
        const char *element_dest
        );

int metric_decode (
        zmsg_t **msg_p,         // message to decode, message is destroyed
        char **type,            // type of metric
        char **element_src,     // source element
        char **value,           // value of metric
        char **unit,            // unit ('%', 'kWh', '', ...)
        int64_t  *timestamp,    // (optional) unix time of measurement, -1 means current system time
        char **element_dest     // (optional) destionation element or NULL
        );

int alert_send (
        mlm_client_t *cl,         // malamute client publish the metric, caller is responsible for correct initialization
        const char *rule_name,    // rule name that case alert evaluation
        const char *element_name, // element where alert was evaluated
        const int64_t  timestamp, // unix time when state was changed, -1 means current system time
        const char *state,        // state of the alert
        const char *severity,     // severity of the alert
        const char *description   // description of the alert
        );

int alert_decode (
        zmsg_t **msg_p,          // message to decode, message is destroyed
        char **rule_name,        // rule name that case alert evaluation
        char **element_name,     // element where alert was evaluated
        int64_t  *timestamp,     // unix time when state was changed, -1 means current system time
        char **state,            // state of the alert
        char **severity,         // severity of the alert
        char **description       // description of the alert
        );

#ifdef __cplusplus
}
#endif

