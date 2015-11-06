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

// implements new BIOS protocols

#include "bios_proto.h"

#include <stdio.h>
#include <string.h>

int metric_send (
        void *dest,
        const char *type,
        const char *element_src,
        const char *value,
        const char *unit,
        int64_t   timestamp,
        const char *element_dest
        ) {

    if (!cl || !type || !element_src || !value || !unit) {
        return -1;
    }
    // timestamp is positive, -1 means current timestamp
    if ( timestamp < -1 ) {
        return -2;
    }

    char *subject, *stimestamp;
    asprintf (&subject, "%s@%s", type, element_src);
    asprintf (&stimestamp, "%"PRIi64, timestamp);

    int r;
    if (zsock_is (dest))
        r = zsock_sendx ((zsock_t)dest, subject, type, element_src, value, unit, stimestamp, element_dest, NULL);
    else
        r = mlm_client_sendx ((mlm_client_t*)dest, subject, type, element_src, value, unit, stimestamp, element_dest, NULL);

    zstr_free (&subject);
    zstr_free (&stimestamp);
    return r;
}

int metric_decode (
        zmsg_t **msg_p,
        char **type,
        char **element_src,
        char **value,
        char **unit,
        int64_t   *timestamp,
        char **element_dest
        ) {

    if (!msg_p || !*msg_p || !type || !element_src || !value || !unit || !timestamp) {
        return -1;
    }

    zmsg_t *msg = *msg_p;
    if ( ( zmsg_size(msg) < 5 ) || ( zmsg_size(msg) > 6 ) ) {
        zmsg_destroy (&msg);
        return -2;
    }
    *type = zmsg_popstr (msg);
    *element_src = zmsg_popstr (msg);
    *value = zmsg_popstr (msg);
    *unit = zmsg_popstr (msg);

    char *stme = zmsg_popstr (msg);
    char *endptr;
    errno = 0;
    long int foo = strtol (stme, &endptr, 10);
    if (errno != 0) {
        *timestamp = -1;
    }
    else {
        *timestamp = foo;
    }
    errno = 0;
    zstr_free (&stme);

    if (*timestamp == -1) {
        *timestamp = time (NULL);
    }

    if (element_dest)
    {
        if ( zmsg_size(msg) != 1 ) {
            zmsg_destroy (&msg);
            return -2;
        }
        *element_dest = zmsg_popstr(msg);
    }

    zmsg_destroy (&msg);
    return 0;
}

int alert_send (
        mlm_client_t *cl,
        const char *rule_name,
        const char *element_name,
        int64_t timestamp,
        const char *state,
        const char *severity,
        const char *description
        )
{
    if (!cl || !rule_name || !element_name || !state || !severity || !description) {
        return -1;
    }
    // timestamp is positive, -1 means current timestamp
    if ( timestamp < -1 ) {
        return -2;
    }

    char *subject, *stimestamp;
    asprintf (&subject, "%s/%s@%s", rule_name, severity, element_name);
    asprintf (&stimestamp, "%"PRIi64, timestamp);

    int r = mlm_client_sendx (cl, subject, rule_name, element_name, stimestamp, state, severity, description, NULL);

    zstr_free (&subject);
    zstr_free (&stimestamp);
    return r;

}

int alert_decode (
        zmsg_t **msg_p,
        char **rule_name,
        char **element_name,
        int64_t *timestamp,
        char **state,
        char **severity,
        char **description
        )
{
    if (!msg_p || !*msg_p || !rule_name || !element_name || !state || !severity || !timestamp || !description) {
        return -1;
    }

    zmsg_t *msg = *msg_p;
    if ( zmsg_size(msg) != 6 ) {
        zmsg_destroy (&msg);
        return -2;
    }
    *rule_name = zmsg_popstr (msg);
    *element_name = zmsg_popstr (msg);

    char *stme = zmsg_popstr (msg);
    char *endptr;
    errno = 0;
    long int foo = strtol (stme, &endptr, 10);
    if (errno != 0) {
        *timestamp = -1;
    }
    else {
        *timestamp = foo;
    }
    errno = 0;
    zstr_free (&stme);

    if (*timestamp == -1) {
        *timestamp = time (NULL);
    }

    *state = zmsg_popstr (msg);
    *severity = zmsg_popstr (msg);
    *description = zmsg_popstr (msg);

    zmsg_destroy (&msg);
    return 0;

}
