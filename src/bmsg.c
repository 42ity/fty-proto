/*  =========================================================================
    bmsg - Command line tool to work with bios proto messages

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

/*
@header
    bmsg - Command line tool to work with bios proto messages
@discuss
@end
*/

#define die(format, ...) \
    do { \
        zsys_error (format, ##__VA_ARGS__); \
        ret = EXIT_FAILURE; \
        goto exit; \
    } while (0);

#include "biosproto_classes.h"

static void
    s_do_monitor (mlm_client_t *client)
{

    while (!zsys_interrupted) {
        zmsg_t *msg = mlm_client_recv (client);

        if (!msg)
            break;

        if (!streq (mlm_client_command (client), "STREAM DELIVER")) {
            zmsg_destroy (&msg);
            continue;
        }

        const char *address = mlm_client_address (client);
        const char *sender = mlm_client_sender (client);
        const char *subject = mlm_client_subject (client);

        puts ("--------------------------------------------------------------------------------");
        printf ("stream=%s\nsender=%s\nsubject=%s\n", address, sender, subject);
        bios_proto_t *bmsg = bios_proto_decode (&msg);
        if (!bmsg)
            printf ("  (cannot decode bios_proto message)\n");
        else
            bios_proto_print (bmsg);
        bios_proto_destroy (&bmsg);

        puts ("--------------------------------------------------------------------------------");

        zmsg_destroy (&msg);
    }
}

int main (int argc, char *argv [])
{
    puts ("bmsg - Command line tool to work with bios proto messages");
    bool verbose = false;
    int argn;
    int ret = 0;
    char *endpoint = "ipc://@/malamute";

    char *bmsg_command = "monitor";

    mlm_client_t *client = NULL;

    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("bmsg [options] ...");
            puts ("  --endpoint / -e        malamute endpoint (default ipc://@/malamute)");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --help / -h            this information");
            puts ("  monitor [stream1 [pattern1 ...] monitor given stream/pattern. Pattern is .* by default");
            puts ("  publish (pub) type     publish given message type on respective stream (" BIOS_PROTO_STREAM_ALERTS ", " BIOS_PROTO_STREAM_ALERTS ", " BIOS_PROTO_STREAM_METRICS ")");
            puts ("  publish (pub) alert rule element_src state severity description time action");
            puts ("                         publish alert on stream " BIOS_PROTO_STREAM_ALERTS);
            puts ("  publish (pub) asset name operation");
            puts ("                         publish asset on stream " BIOS_PROTO_STREAM_ASSETS " (for now without ext attributes)");
            puts ("  publish (pub) metric quantity element_src value units time");
            puts ("                         publish metric on stream " BIOS_PROTO_STREAM_METRICS);
            return 0;
        }
        else
        if (streq (argv [argn], "--verbose")
        ||  streq (argv [argn], "-v"))
            verbose = true;
        else
        if (streq (argv [argn], "--endpoint")
        ||  streq (argv [argn], "-e")) {
            if (argn == argc -1)
                die ("value after --endpoint / -e expected", "");
            endpoint = argv [argn];
            argn ++;
        }
        else
        if (argv [argn][0] != '-')
            break;
        else {
            printf ("Unknown option: %s\n", argv [argn]);
            return 1;
        }
    }

    // identify the main command here
    if (argv [argn]) {
        if (streq (argv [argn], "monitor"))
            bmsg_command = "monitor";
        else
        if (streq (argv [argn], "publish")
        ||  streq (argv [argn], "pub"))
            bmsg_command = "publish";
        else
            die ("Unknown command %s", argv[argn]);
        argn ++;
    }

    if (verbose)
        zsys_info ("command=%s", bmsg_command);

    // connect to malamute
    client = mlm_client_new ();
    assert (client);

    char *address;
    int r = asprintf (&address, "bmsg.%"PRIi64, zclock_mono ());
    assert (r > -1);
    if (verbose)
        zsys_info ("address: %s", address);

    r = mlm_client_connect (client, endpoint, 5000, address);
    zstr_free (&address);
    if (r == -1)
        die ("mlm_client_connect failed", NULL);

    // set monitor
    if (streq (bmsg_command, "monitor")) {

        if (!argv [argn]) {
            // set all streams
            if (verbose)
                zsys_info ("set_consumer on " BIOS_PROTO_STREAM_ALERTS ", " BIOS_PROTO_STREAM_ASSETS ", " BIOS_PROTO_STREAM_METRICS);
            r = mlm_client_set_consumer (client, BIOS_PROTO_STREAM_METRICS, ".*");
            if (r == -1)
                die ("set consumer " BIOS_PROTO_STREAM_METRICS " failed", NULL);

            r = mlm_client_set_consumer (client, BIOS_PROTO_STREAM_ASSETS, ".*");
            if (r == -1)
                die ("set consumer " BIOS_PROTO_STREAM_ASSETS " failed", NULL);

            r = mlm_client_set_consumer (client, BIOS_PROTO_STREAM_ALERTS, ".*");
            if (r == -1)
                die ("set consumer " BIOS_PROTO_STREAM_ALERTS " failed", NULL);
        }

        while (argv [argn]) {
            char *stream = argv [argn];
            char *pattern = ".*";
            if (argv [argn+1])
                pattern = argv [++argn];
            argn ++;

            if (verbose)
                zsys_info ("set_consumer (%s, %s)", stream, pattern);
            r = mlm_client_set_consumer (client, stream, pattern);
            if (r == -1)
                die ("set consumer %s %s failed", stream, pattern);
        }
        s_do_monitor (client);
    }
    else
    if (streq (bmsg_command, "publish")) {
        if (streq (argv[argn], "alert")) {

            mlm_client_set_producer (client, BIOS_PROTO_STREAM_ALERTS);

            char *rule = argv[++argn];
            if (!rule)
                die ("missing rule", NULL);

            char *element_src = argv[++argn];
            if (!element_src)
                die ("missing element_src", NULL);

            char *state = argv[++argn];
            if (!state)
                die ("missing state", NULL);

            char *severity = argv[++argn];
            if (!severity)
                die ("missing severity", NULL);

            char *description = argv[++argn];
            if (!description)
                die ("missing description", NULL);

            char *s_time = argv[++argn];
            uint64_t time;
            if (!s_time)
                die ("missing time", NULL);
            int r = sscanf (s_time, "%"SCNu64, &time);
            if (r < 1)
                die ("time %s is not a number", s_time);

            char *action = argv[++argn];
            if (!action)
                die ("missing action", NULL);

            if (verbose)
                zsys_info ("publishing alert rule=%s, element_src=%s, state=%s, severity=%s, description=%s, time=%"PRIu64 ", action=%s",
                        rule,
                        element_src,
                        state,
                        severity,
                        description,
                        time,
                        action);

            char *subject;
            r = asprintf (&subject, "%s@%s", rule, element_src);
            assert (r > 0);

            zmsg_t *msg = bios_proto_encode_alert (
                        NULL,
                        rule,
                        element_src,
                        state,
                        severity,
                        description,
                        time,
                        action);
            mlm_client_send (client, subject, &msg);
            zstr_free (&subject);
            // to get all the threads behind enough time to send it
            zclock_sleep (500);
        }
        else
        if (streq (argv[argn], "metric")) {

            mlm_client_set_producer (client, BIOS_PROTO_STREAM_METRICS);

            char *quantity = argv[++argn];
            if (!quantity)
                die ("missing quantity", NULL);

            char *element_src = argv[++argn];
            if (!element_src)
                die ("missing element_src", NULL);

            char *value = argv[++argn];
            if (!value)
                die ("missing value", NULL);

            char *unit = argv[++argn];
            if (!unit)
                die ("missing unit", NULL);

            char *s_time = argv[++argn];
            if (!s_time)
                die ("missing time", NULL);
            uint64_t time;
            int r = sscanf (s_time, "%"SCNu64, &time);
            if (r < 1)
                die ("time %s is not a number", s_time);

            if (verbose) {
                zsys_info ("publishing metric type=%s, element_src=%s, value=%s, unit=%s, time=%"PRIu64 ,
                        quantity,
                        element_src,
                        value,
                        unit,
                        time);
            }

            char *subject;
            r = asprintf (&subject, "%s@%s", quantity, element_src);
            assert (r > 0);

            zmsg_t *msg = bios_proto_encode_metric (
                        NULL,
                        quantity,
                        element_src,
                        value,
                        unit,
                        time);
            mlm_client_send (client, subject, &msg);
            zstr_free (&subject);
            // to get all the threads behind enough time to send it
            zclock_sleep (500);
        }
        else
        if (streq (argv[argn], "asset")) {

            mlm_client_set_producer (client, BIOS_PROTO_STREAM_ASSETS);

            char *name = argv[++argn];
            if (!name)
                die ("missing name", NULL);

            char *operation = argv[++argn];
            if (!operation)
                die ("missing operation", NULL);

            if (verbose) {
                zsys_info ("publishing asset name=%s, operation=%s",
                        name,
                        operation);
            }

            char *subject;
            r = asprintf (&subject, "%s@%s", operation, name);
            assert (r > 0);

            zmsg_t *msg = bios_proto_encode_asset (
                        NULL,
                        name,
                        operation,
                        NULL);
            mlm_client_send (client, subject, &msg);
            zstr_free (&subject);
            // to get all the threads behind enough time to send it
            zclock_sleep (500);
        }
        else {
            die ("unknown type %s", argv[argn]);
        }
    }

exit:
    mlm_client_destroy (&client);
    return ret;
}
