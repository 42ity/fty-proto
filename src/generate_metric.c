/*
Copyright (C) 2014 - 2018 Eaton

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
*/

#include <fty_proto.h>
#include <malamute.h>

#define LOG_CONFIG "/etc/fty/ftylog.cfg"

int main(int argc, char** argv) {
    if (argc < 6) {
        fprintf (stderr, "Usage: %s <type> <source> <value> <units> <TTL> [endpoint]\n", argv[0]);
        return EXIT_FAILURE;
    }

    ftylog_setInstance("generate_metric", LOG_CONFIG);

    const char* endpoint = NULL;
    if (argc == 7) {
       endpoint = argv[6];
    }
    else {
       endpoint = "ipc://@/malamute";
    }

    // TTL
    uint32_t ttl = 0;
    if (atoi (argv[5]) <= 0 || atoi (argv[5]) > INT32_MAX) {
        log_error ("<TTL> has bad value. Number in range 1 .. INT32_MAX expected.");
        return EXIT_FAILURE;
    }
    ttl = atoi (argv[5]);

    // topic
    char *buff = NULL;
    if (asprintf (&buff, "%s@%s", argv[1], argv[2]) < 3) {
        log_error ("asprintf () failed. Can't allocate subject.");
        return EXIT_FAILURE;
    }

    mlm_client_t *producer = mlm_client_new ();
    assert (producer);

    int rv = mlm_client_connect (producer, endpoint, 5000, "metric_generator");
    if (rv == -1) {
        log_error ("mlm_client_connect (endpoint = '%s', timeout = '5000', address = 'metric_generator') failed", endpoint);
        mlm_client_destroy (&producer);
        zstr_free (&buff);
        return EXIT_FAILURE;
    }

    rv = mlm_client_set_producer (producer, "METRICS");
    if (rv == -1) {
        log_error ("mlm_client_set_producer (stream = 'METRICS') failed.");
        mlm_client_destroy (&producer);
        zstr_free (&buff);
        return EXIT_FAILURE;
    }

    zmsg_t *msg = fty_proto_encode_metric (
            NULL,       // aux
            time (NULL),// time
            ttl,        // TTL
            argv[1],    // type
            argv[2],    // name
            argv[3],    // value
            argv[4]     // unit
            );
    assert (msg);
    zstr_free (&buff);

    rv = mlm_client_send (producer, buff, &msg);
    if (rv == -1) {
        log_error ("mlm_client_send (subject = '%s') failed.", buff);
        mlm_client_destroy(&producer);
        return EXIT_FAILURE;
    }

    mlm_client_destroy(&producer);
    return EXIT_SUCCESS;
}
