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

static const int64_t STAT_INTERVAL = 10000;     // we'll count messages in 10 seconds intervals

static void
s_number_destructor (void ** item_p)
{
    free (*item_p);
    *item_p = NULL;
}

static void
s_add_cnt (zlistx_t *stat_list, size_t cnt)
{
    zsys_info ("got [%zu] messages in 10 seconds", cnt);
    size_t *item_p = (size_t*) zmalloc (sizeof (size_t));
    *item_p = cnt;
    zlistx_add_end (stat_list, (void*) item_p);
}

static void
s_print_stats (zlistx_t *stat_list)
{

    size_t max = 0, sum = 0;
    size_t min = SIZE_MAX;

    for (void *it = zlistx_first (stat_list);
               it != NULL;
               it = zlistx_next (stat_list))
    {
        size_t v = *(size_t*)it;

        if (v < min)
            min = v;
        if (v > max)
            max = v;
        sum += v;
    }

    double avg = 0.0;
    if (zlistx_size (stat_list) > 0)
        avg = (double) sum / zlistx_size (stat_list);
    else
        min = 0;

    zsys_info ("interval/count/min/avg/max = %"PRIi64 " ms/%zu/%zu/%.3f/%zu",
            STAT_INTERVAL,
            zlistx_size (stat_list),
            min,
            avg,
            max);
}

static void
    s_do_monitor (mlm_client_t *client, bool stats)
{

    zlistx_t *stat_list = zlistx_new ();
    zlistx_set_destructor (stat_list, s_number_destructor);
    size_t cnt = 0;
    int64_t interval = STAT_INTERVAL;
    int64_t start = zclock_mono ();
    zpoller_t *poller = zpoller_new (mlm_client_msgpipe (client), NULL);

    while (!zsys_interrupted) {

        if (stats)
        {
            interval = STAT_INTERVAL - (zclock_mono () - start);

            if (interval > 100)
                zpoller_wait (poller, interval);

            if (zsys_interrupted || zpoller_terminated (poller))
                break;

            if (interval < 100 || zpoller_expired (poller)) {
                s_add_cnt (stat_list, cnt);
                cnt = 0;
                start = zclock_mono ();
                continue;
            }
        }
        else
            zpoller_wait (poller, 0);

        // it's unlikelly someone is going to send messages to bmsg mailbox, so counter is not increased here
        // ... but it's fine to cleanup the broker too
        zmsg_t *msg = mlm_client_recv (client);

        if (!msg)
            break;

        if (!streq (mlm_client_command (client), "STREAM DELIVER")) {
            zmsg_destroy (&msg);
            continue;
        }

        if (stats)
            cnt += 1;
        else
        {
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
        }

        zmsg_destroy (&msg);
    }
    if (stats)
        s_print_stats (stat_list);

    zlistx_destroy (&stat_list);
}

static zhash_t *
    s_parse_aux (int argc, int argn, char *argv [])
{
    int ret=EXIT_SUCCESS;    // to make die working here
    zhash_t *hash = zhash_new ();

    for (int i = argn; i != argc; i++)
    {
        char *key = argv [i];

        // skip ext. prefix there!
        if (strncmp (key, "ext.", 4) == 0)
            continue;

        char *eq = strchr (key, '=');
        if (eq == NULL)
            die ("Failed to parse '%s', missing =", key);

        *eq = '\0';
        char *value = eq+1;
        zhash_update (hash, key, (void*) value);
        *eq = '=';
    }
exit:
    if (ret == EXIT_FAILURE)
        exit (1);
    return hash;
}

static zhash_t *
    s_parse_ext (int argc, int argn, char *argv [])
{
    int ret=EXIT_SUCCESS;    // to make die working here
    zhash_t *hash = zhash_new ();

    for (int i = argn; i != argc; i++)
    {
        char *key = argv [i];

        // use ext. prefix there!
        if (strncmp (key, "ext.", 4) != 0)
            continue;

        char *eq = strchr (key, '=');
        if (eq == NULL)
            die ("Failed to parse '%s', missing =", key);

        *eq = '\0';
        char *value = eq+1;
        zhash_update (hash, key+4, (void*) value);
        *eq = '=';
    }
exit:
    if (ret == EXIT_FAILURE)
        exit (1);
    return hash;
}

static void
    s_print_bmsg (const char *prefix, const char *subject, zmsg_t *msg)
{
    zsys_info ("%s, subject=%s", prefix, subject);
    zmsg_t *msg2 = zmsg_dup (msg);
    bios_proto_t *bmsg = bios_proto_decode (&msg2);
    bios_proto_print (bmsg);
    bios_proto_destroy (&bmsg);
}

#define ACTIVE "ACTIVE"
#define ACK_WIP "ACK-WIP"
#define ACK_IGNORE "ACK-IGNORE"
#define ACK_PAUSE "ACK-PAUSE"
#define ACK_SILENCE "ACK-SILENCE"
#define RESOLVED "RESOLVED"
#define s_CRITICAL "CRITICAL"
#define s_WARNING "WARNING"
#define s_INFO "INFO"

int main (int argc, char *argv [])
{
    puts ("bmsg - Command line tool to work with bios proto messages");
    bool verbose = false;
    int argn;
    int ret = 0;
    char *endpoint = "ipc://@/malamute";
    bool stats = false;

    char *bmsg_command = "monitor";
    mlm_client_t *client = NULL;
    for (argn = 1; argn < argc; argn++) {
        if (streq (argv [argn], "--help")
        ||  streq (argv [argn], "-h")) {
            puts ("bmsg [options] ...");
            puts ("  --endpoint / -e        malamute endpoint (default ipc://@/malamute)");
            puts ("  --verbose / -v         verbose test output");
            puts ("  --stats / -s           prints statistics");
            puts ("  --help / -h            this information");
            puts ("  monitor [stream1 [pattern1 ...] monitor given stream/pattern. Pattern is .* by default");
            puts ("  publish type     publish given message type on respective stream (" BIOS_PROTO_STREAM_ALERTS ", " BIOS_PROTO_STREAM_ALERTS_SYS ", " BIOS_PROTO_STREAM_METRICS ", " BIOS_PROTO_STREAM_ASSETS ")");
            puts ("  publish (alert|alertsys) <rule_name> <element_src> <state> <severity> <description> <time> <action>");
            puts ("                         publish alert on stream " BIOS_PROTO_STREAM_ALERTS " or " BIOS_PROTO_STREAM_ALERTS_SYS);
            puts ("                         <state> has possible values " ACTIVE "," ACK_WIP "," ACK_PAUSE "," ACK_IGNORE "," ACK_SILENCE "," RESOLVED);
            puts ("                         <severity> has possible values " s_CRITICAL "," s_WARNING "," s_INFO);
            puts ("                         <time> is an UNIX timestamp");
            puts ("                         <action> has possible values SMS, EMAIL, SMS/EMAIL, EMAIL/SMS");
            puts ("  publish asset <name> <operation> [auxilary_data see section bellow]");
            puts ("                         publish asset on stream " BIOS_PROTO_STREAM_ASSETS);
            puts ("                         <operation> has possible values INSERT, UPDATE, DELETE");
            puts ("                         Auxilary data:");
            puts ("                             priority=X where X in[1,5]");
            puts ("  publish metric <quantity> <element_src> <value> <units> <ttl>");
            puts ("                         publish metric on stream " BIOS_PROTO_STREAM_METRICS);
            puts ("                         <quantity> a string name for the metric type");
            puts ("                         <element_src> a string name for asset where metric was detected");
            puts ("                         <value> a string value of the metric (for now only values convertable to double should be used");
            puts ("                         <units> a string like %, W, days");
            puts ("                         <ttl>   a number time to leave [s]");
            puts ("                         Auxilary data:");
            puts ("                             time=Y, where Y is an UNIX timestamp when metric was detected");
            puts ("");
            puts ("Auxiliary data for all streams and extended attributes for stream ASSETS:");
            puts ("  those are recognized on the end of the command line");
            puts ("  key=value              add additional key and value to aux hash, the last value is used");
            puts ("  ext.key=value          add additional key and value to ext hash, the last value is used");
            puts ("                         ignored for non asset messages");
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
        if (streq (argv [argn], "--stats")
        ||  streq (argv [argn], "-s"))
            stats = true;
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
        if ( argn +1 == argc )
            die ("too few arguments", "");
        ++argn;
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
                zsys_info ("setting consumer on " BIOS_PROTO_STREAM_ALERTS ", " BIOS_PROTO_STREAM_ASSETS ", " BIOS_PROTO_STREAM_METRICS);
            r = mlm_client_set_consumer (client, BIOS_PROTO_STREAM_METRICS, ".*");
            if (r == -1)
                die ("set consumer on" BIOS_PROTO_STREAM_METRICS " failed", NULL);

            r = mlm_client_set_consumer (client, BIOS_PROTO_STREAM_ASSETS, ".*");
            if (r == -1)
                die ("set consumer on" BIOS_PROTO_STREAM_ASSETS " failed", NULL);

            r = mlm_client_set_consumer (client, BIOS_PROTO_STREAM_ALERTS, ".*");
            if (r == -1)
                die ("set consumer on" BIOS_PROTO_STREAM_ALERTS " failed", NULL);
        }

        while (argv [argn]) {
            char *stream = argv [argn];
            char *pattern = ".*";
            if (argv [argn+1])
                pattern = argv [++argn];
            argn ++;

            if (verbose)
                zsys_info ("set consumer (%s, %s)", stream, pattern);
            r = mlm_client_set_consumer (client, stream, pattern);
            if (r == -1)
                die ("set consumer (%s, %s) failed", stream, pattern);
        }
        s_do_monitor (client, stats);
    }
    else
    if (streq (bmsg_command, "publish")) {
        if (streq (argv[argn], "alert")  || streq (argv[argn], "alertsys")) {
            if (streq (argv[argn], "alert")) {
                mlm_client_set_producer (client, BIOS_PROTO_STREAM_ALERTS);
            } else {
                mlm_client_set_producer (client, BIOS_PROTO_STREAM_ALERTS_SYS);
            }
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

            zhash_t *aux = s_parse_aux (argc, argn+1, argv);

            char *subject;
            r = asprintf (&subject, "%s@%s", rule, element_src);
            assert (r > 0);

            zmsg_t *msg = bios_proto_encode_alert (
                        aux,
                        rule,
                        element_src,
                        state,
                        severity,
                        description,
                        time,
                        action);

            if (verbose)
                s_print_bmsg ("alert", subject, msg);

            mlm_client_send (client, subject, &msg);
            zhash_destroy (&aux);
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

            char *s_ttl = argv[++argn];
            if (!s_ttl)
                die ("missing TTL", NULL);
            uint32_t ttl;
            int r = sscanf (s_ttl, "%"SCNu32, &ttl);
            if (r < 1)
                die ("TTL %s is not a number", s_ttl);

            zhash_t *aux = s_parse_aux (argc, argn+1, argv);

            char *subject;
            r = asprintf (&subject, "%s@%s", quantity, element_src);
            assert (r > 0);

            zmsg_t *msg = bios_proto_encode_metric (
                        aux,
                        quantity,
                        element_src,
                        value,
                        unit,
                        ttl);

            if (verbose)
                s_print_bmsg ("metric", subject, msg);

            mlm_client_send (client, subject, &msg);
            zhash_destroy (&aux);
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

            zhash_t *aux = s_parse_aux (argc, argn+1, argv);
            zhash_t *ext = s_parse_ext (argc, argn+1, argv);

            char *subject;
            r = asprintf (&subject, "%s@%s", operation, name);
            assert (r > 0);

            zmsg_t *msg = bios_proto_encode_asset (
                        aux,
                        name,
                        operation,
                        ext);

            if (verbose)
                s_print_bmsg ("alert", subject, msg);

            mlm_client_send (client, subject, &msg);
            zhash_destroy (&aux);
            zhash_destroy (&ext);
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
