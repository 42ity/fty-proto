/*  =========================================================================
    bmsg - Command line tool to work with fty proto messages

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

/*
@header
    bmsg - Command line tool to work with fty proto messages
@discuss
@end
*/
#define LOG_CONFIG "/etc/fty/ftylog.cfg"

#define die(format, ...) \
    do { \
        log_error (format, ##__VA_ARGS__); \
        ret = EXIT_FAILURE; \
        goto exit; \
    } while (0);

#include "fty_proto_classes.h"

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
    log_info ("got [%zu] messages in 10 seconds", cnt);
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

    log_info ("interval/count/min/avg/max = %"PRIi64 " ms/%zu/%zu/%.3f/%zu",
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
            if ( is_fty_proto (msg) ) {
                fty_proto_t *bmsg = fty_proto_decode (&msg);
                if (!bmsg)
                    printf ("  (cannot decode fty_proto message)\n");
                else
                    fty_proto_print (bmsg);
                fty_proto_destroy (&bmsg);
            }
            else {
                zmsg_print (msg);
                zmsg_destroy (&msg);
            }

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

    for (int i = argn; i < argc; i++)
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
    log_info ("%s, subject=%s", prefix, subject);
    zmsg_t *msg2 = zmsg_dup (msg);
    fty_proto_t *bmsg = fty_proto_decode (&msg2);
    fty_proto_print (bmsg);
    fty_proto_destroy (&bmsg);
}


#define STRFTIME_DATETIME_FORMAT "%FT%TZ"
static int
s_calendar_to_datetime(time_t timestamp, char* buffer, size_t n) {
    struct tm* tmp = gmtime (&timestamp);
    if (!tmp || strftime (buffer, n, STRFTIME_DATETIME_FORMAT, tmp) == 0) {
        return 0;
    }
    return -1;
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
#define FTY_ALERT_LIST "fty-alert-list"
#define RFC_ALERTS_LIST "rfc-alerts-list"

int main (int argc, char *argv [])
{
    puts ("bmsg - Command line tool to work with fty proto messages");
    bool verbose = false;
    int timeout = -1;
    int argn;
    int ret = 0;
    int r = 0;
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
            puts ("  --timeout / -t         timeout (seconds) when waiting for reply");
            puts ("  monitor [stream1 [pattern1 ...] monitor given stream/pattern. Pattern is .* by default");
            puts ("  publish type     publish given message type on respective stream (" FTY_PROTO_STREAM_ALERTS ", " FTY_PROTO_STREAM_ALERTS_SYS ", " FTY_PROTO_STREAM_ASSETS ", " FTY_PROTO_STREAM_METRICS ", " FTY_PROTO_STREAM_METRICS_SENSOR ", " FTY_PROTO_STREAM_METRICS_UNAVAILABLE ", " FTY_PROTO_STREAM_EULA ", " FTY_PROTO_STREAM_LICENSING_ANNOUNCEMENTS ")");
            puts ("  publish (alert|alertsys) <rule_name> <element_src> <state> <severity> <description> <time> <action>");
            puts ("                         publish alert on stream " FTY_PROTO_STREAM_ALERTS " or " FTY_PROTO_STREAM_ALERTS_SYS);
            puts ("                         <state> has possible values " ACTIVE "," ACK_WIP "," ACK_PAUSE "," ACK_IGNORE "," ACK_SILENCE "," RESOLVED);
            puts ("                         <severity> has possible values " s_CRITICAL "," s_WARNING "," s_INFO);
            puts ("                         <time> is an UNIX timestamp");
            puts ("                         <action> has possible values SMS, EMAIL, SMS/EMAIL, EMAIL/SMS");
            puts ("                         Note: Publishing on " FTY_PROTO_STREAM_ALERTS_SYS " has some restrictions - only ACTIVE/RESOLVED alerts.");
            puts ("  publish asset <name> <operation> [auxiliary_data see section below]");
            puts ("                         publish asset on stream " FTY_PROTO_STREAM_ASSETS);
            puts ("                         <operation> has possible values create, update, delete, inventory");
            puts ("                         Auxilary data:");
            puts ("                             priority=X where X in[1,5]");
            puts ("  publish metric_unavailable <metric topic>");
            puts ("                         publish information on stream " FTY_PROTO_STREAM_METRICS_UNAVAILABLE " that this metric is no longer  monitored by system");
            puts ("  publish (metric|metricsensor) <quantity> <element_src> <value> <units> <ttl> <time>");
            puts ("                         publish metric on stream " FTY_PROTO_STREAM_METRICS " or " FTY_PROTO_STREAM_METRICS_SENSOR);
            puts ("                         <quantity> a string name for the metric type");
            puts ("                         <element_src> a string name for asset where metric was detected");
            puts ("                         <value> a string value of the metric (for now only values convertable to double should be used");
            puts ("                         <units> a string like %, W, days");
            puts ("                         <ttl>   a number time to leave [s]");
            puts ("                         <time>  an UNIX timestamp when metric was detected");
            puts ("                         Auxilary data:");
            puts ("                             quantity=Y, where Y is value");
            puts ("  publish eula <state>");
            puts ("                         publish <state> of EULA on stream " FTY_PROTO_STREAM_EULA );
            puts ("                         Currently the only state used by the system is ACCEPTED.");
            puts ("  publish licensing-limitation <quantity> <value> [unit] ");
            puts ("                         publish fty-proto metric on stream " FTY_PROTO_STREAM_LICENSING_ANNOUNCEMENTS);
            puts ("                         See description details in licensing agent for valid values of quantity parameter.");
            puts ("  request <agent_name> <subject> [additional parameters]");
            puts ("  alertslist <state>");
            puts ("                        <state> among ALL | ACTIVE | ACK-WIP | ACK-IGNORE | ACK-PAUSE | ACK-SILENCE");
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
        if (streq (argv [argn], "--timeout")
        ||  streq (argv [argn], "-t")) {
            if (argn + 1 == argc)
                die ("value after --timeout / -t expected", "");
            timeout = atoi(argv [argn+1]) * 1000;
            log_debug ("cli specified timeout: %i", timeout);
            argn ++;
        }
        else
        if (streq (argv [argn], "--endpoint")
        ||  streq (argv [argn], "-e")) {
            if (argn + 1 == argc)
                die ("value after --endpoint / -e expected", "");
            endpoint = argv [argn+1];
            log_debug ("cli specified endpoint: %s", endpoint);
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
    ftylog_setInstance("bmsg", LOG_CONFIG);
    if (verbose)
        ftylog_setVeboseMode(ftylog_getInstance());

    // identify the main command here
    if (argv [argn]) {
        if (streq (argv [argn], "monitor"))
            bmsg_command = "monitor";
        else
        if (streq (argv [argn], "publish")
        ||  streq (argv [argn], "pub"))
            bmsg_command = "publish";
        else
        if (streq (argv [argn], "request")
        ||  streq (argv [argn], "req"))
            bmsg_command = "request";
        else
        if (streq (argv [argn], "alertslist"))
            bmsg_command = "alertslist";
        else
            die ("Unknown command %s", argv[argn]);
        if ( argn +1 == argc )
            die ("too few arguments", "");
        ++argn;
    }

    log_trace ("command=%s", bmsg_command);

    // connect to malamute
    client = mlm_client_new ();
    assert (client);

    char *address;
    r = asprintf (&address, "bmsg.%"PRIi64, zclock_mono ());
    assert (r > -1);
    log_trace ("address: %s", address);

    r = mlm_client_connect (client, endpoint, 5000, address);
    zstr_free (&address);
    if (r == -1)
        die ("mlm_client_connect failed", NULL);

    // set monitor
    if (streq (bmsg_command, "monitor")) {

        if (!argv [argn]) {
            // set all streams
            log_trace ("setting consumer on " FTY_PROTO_STREAM_ALERTS ", " FTY_PROTO_STREAM_ASSETS ", " FTY_PROTO_STREAM_METRICS);
            r = mlm_client_set_consumer (client, FTY_PROTO_STREAM_METRICS, ".*");
            if (r == -1)
                die ("set consumer on" FTY_PROTO_STREAM_METRICS " failed", NULL);

            r = mlm_client_set_consumer (client, FTY_PROTO_STREAM_ASSETS, ".*");
            if (r == -1)
                die ("set consumer on" FTY_PROTO_STREAM_ASSETS " failed", NULL);

            r = mlm_client_set_consumer (client, FTY_PROTO_STREAM_ALERTS, ".*");
            if (r == -1)
                die ("set consumer on" FTY_PROTO_STREAM_ALERTS " failed", NULL);
        }

        while (argv [argn]) {
            char *stream = argv [argn];
            char *pattern = ".*";
            if (argv [argn+1])
                pattern = argv [++argn];
            argn ++;

            log_trace ("set consumer (%s, %s)", stream, pattern);
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
                mlm_client_set_producer (client, FTY_PROTO_STREAM_ALERTS);
            } else {
                mlm_client_set_producer (client, FTY_PROTO_STREAM_ALERTS_SYS);
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
            if (streq (argv[argn], "alertsys") &&
                (!streq (state, "ACTIVE") && !streq (state, "RESOLVED")))
                die ("restrictions not met for alertsys: only ACTIVE/RESOLVED", "NULL");

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

            zlist_t *action = zlist_new ();
            if (!action)
                    die ("zlist_new: %s", strerror(errno))
            for (++argn; argv[argn]; ++argn) {
                if (strchr(argv[argn], '='))
                    break;
                if (zlist_append (action, argv[argn]) < 0)
                    die ("zlist_append: %s", strerror(errno))
            }
            if (!zlist_size(action))
                die ("missing action", NULL);

            zhash_t *aux = s_parse_aux (argc, argn, argv);

            char *subject;
            r = asprintf (&subject, "%s@%s/%s@%s", rule, element_src, severity, element_src);
            assert (r > 0);

            zmsg_t *msg = fty_proto_encode_alert (
                        aux,
                        time,
                        300,
                        rule,
                        element_src,
                        state,
                        severity,
                        description,
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
        if (streq (argv[argn], "metric") || streq (argv[argn], "metricsensor") ) {
            if (streq (argv[argn], "metric")) {
                mlm_client_set_producer (client, FTY_PROTO_STREAM_METRICS);
            } else {
                mlm_client_set_producer (client, FTY_PROTO_STREAM_METRICS_SENSOR);
            }
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

            char *s_time = argv[++argn];
            if (!s_time)
                die ("missing time", NULL);

            uint32_t time_m;
            r = sscanf (s_time, "%"SCNu32, &time_m);
            if (r < 1)
                die ("time %s is not a number", s_time);

            zhash_t *aux = s_parse_aux (argc, argn+1, argv);

            char *subject;
            r = asprintf (&subject, "%s@%s", quantity, element_src);
            assert (r > 0);

            zmsg_t *msg = fty_proto_encode_metric (
                        aux,
                        time_m,
                        ttl,
                        quantity,
                        element_src,
                        value,
                        unit);

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

            mlm_client_set_producer (client, FTY_PROTO_STREAM_ASSETS);

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

            zmsg_t *msg = fty_proto_encode_asset (
                        aux,
                        name,
                        operation,
                        ext);

            if (verbose)
                s_print_bmsg ("asset", subject, msg);

            mlm_client_send (client, subject, &msg);
            zhash_destroy (&aux);
            zhash_destroy (&ext);
            zstr_free (&subject);
            // to get all the threads behind enough time to send it
            zclock_sleep (500);
        }
        else
        if (streq (argv[argn], "metric_unavailable")) {

            mlm_client_set_producer (client, FTY_PROTO_STREAM_METRICS_UNAVAILABLE);

            char *metric_topic = argv[++argn];
            if (!metric_topic)
                die ("missing metric_topic", NULL);

            zmsg_t *msg = zmsg_new();
            zmsg_addstr (msg, "METRIC_UNAVAILABLE");
            zmsg_addstr (msg, metric_topic);

            if (verbose)
                zmsg_print (msg);

            mlm_client_send (client, metric_topic, &msg);
            // to get all the threads behind enough time to send it
            zclock_sleep (500);
        }
        else
            if (streq (argv[argn], "eula")) {

                mlm_client_set_producer (client, FTY_PROTO_STREAM_EULA);

                char *state = argv[++argn];
                if (!state)
                    die ("missing EULA state", NULL);

                zmsg_t *msg = zmsg_new();
                zmsg_addstr (msg, "EULA");
                zmsg_addstr (msg, state);

                if (verbose)
                    zmsg_print (msg);

                mlm_client_send (client, "eula", &msg);
                // to get all the threads behind enough time to send it
                zclock_sleep (500);
            }
        else
            if (streq (argv[argn], "licensing-limitation")) {

                mlm_client_set_producer (client, FTY_PROTO_STREAM_LICENSING_ANNOUNCEMENTS);

                char *quantity = argv[++argn];
                if (!quantity)
                    die ("missing quantity", NULL);

                char *value = argv[++argn];
                if (!value)
                    die ("missing value", NULL);

                char *unit = argv[++argn];
                if (!unit)
                    unit = strdup ("");

                zmsg_t *msg = fty_proto_encode_metric (
                            NULL,
                            time (NULL),
                            24*60*60,
                            quantity,
                            "rackcontroller-0",
                            value,
                            unit);

                if (verbose)
                    zmsg_print (msg);

                mlm_client_send (client, "LIMITATIONS", &msg);
                // to get all the threads behind enough time to send it
                zclock_sleep (500);
            }
        else {
            die ("unknown type %s", argv[argn]);
        }
    }
    else
    if (streq (bmsg_command, "request")) {
        char *agent_name = argv[argn];
        if (!agent_name)
             die ("missing agent name", NULL);

        argn ++;
        char *subject = argv[argn];
        if (!subject)
             die ("missing subject", NULL);

        argn ++;
        zmsg_t *request = zmsg_new ();
        // Process additional arguments
        while (argv [argn]) {
             char *add_arg = argv [argn];
             zmsg_addstr (request, add_arg);
             log_trace ("param: '%s'", add_arg);
             argn ++;
        }
        int rv = mlm_client_sendto (client, agent_name, subject, NULL, 5000, &request);
        if (rv != 0)
             die ("Request failed", NULL);

        zmsg_destroy (&request);

        // Listen for the result
        zpoller_t *poller = zpoller_new (mlm_client_msgpipe (client), NULL);
        zsock_t *which = (zsock_t *) zpoller_wait (poller, timeout);
        if (which == mlm_client_msgpipe (client)) {
            zmsg_t *recv = mlm_client_recv (client);
            // Check if positive
            char *okfail = zmsg_popstr (recv);
            if ( okfail && streq (okfail, "OK"))
                ret = 0;
            else {
                // Display the error
                printf ("%s\n", okfail);
                ret = 1;
            }

            zstr_free (&okfail);
            char *rc = zmsg_popstr(recv);
            while (rc) {
                printf ("%s\n", rc);
                zstr_free (&rc);
                rc = zmsg_popstr(recv);
            }

            zmsg_destroy (&recv);
        }
    }
    else
    if (streq (bmsg_command, "alertslist")) {
        //implement RFC-Alerts-List request protocol
        char *state = argv[argn];
        if (!state)
            state="ALL";
        zmsg_t *request = zmsg_new ();
        // Process additional arguments
        zmsg_addstr (request, "LIST");
        zmsg_addstr (request, state);
        //printf ("do %s %s %s %s\n",FTY_ALERT_LIST, RFC_ALERTS_LIST, "LIST", state);
        int rv = mlm_client_sendto (client, FTY_ALERT_LIST, RFC_ALERTS_LIST, NULL, 5000, &request);
        if (rv != 0)
             die ("Request failed", NULL);
        zmsg_destroy (&request);
        // Listen for the result
        zpoller_t *poller = zpoller_new (mlm_client_msgpipe (client), NULL);
        zsock_t *which = (zsock_t *) zpoller_wait (poller, timeout);
        if (which == mlm_client_msgpipe (client)) {
            zmsg_t *recv = mlm_client_recv (client);
            // Check if positive
            char *okfail = zmsg_popstr (recv);
            char *part = zmsg_popstr (recv);
            if ( okfail && streq (okfail, "LIST") && part && streq (part, "ALL") )
                ret = 0;
            else {
                // Display the error
                printf ("%s %s\n", okfail,part);
                ret = 1;
            }
            zstr_free (&okfail);
            zstr_free (&part);
            if(ret==0){
                zframe_t *frame = zmsg_pop (recv);
                while (frame) {

                    #if CZMQ_VERSION_MAJOR == 3
                        zmsg_t *decoded_zmsg = zmsg_decode (zframe_data (frame), zframe_size (frame));
                    #else
                        zmsg_t *decoded_zmsg = zmsg_decode (frame);
                    #endif
                    zframe_destroy (&frame);
                    fty_proto_t *decoded = fty_proto_decode (&decoded_zmsg);

                    char buff[64];
                    int rv = s_calendar_to_datetime (fty_proto_time (decoded), buff, 64);
                    assert(rv);

                    printf("%s %s %s %s [",
                        fty_proto_rule (decoded),
                        fty_proto_name (decoded),
                        fty_proto_state (decoded),
                        buff);
                    const char *action = fty_proto_action_first(decoded);
                    bool first = true;
                    while (action) {
                        if (!first)
                            fputs(",", stdout);
                        fputs(action, stdout);
                        first = false;
                        action = fty_proto_action_next(decoded);
                    }
                    printf("] %s %s\n",
                        fty_proto_severity (decoded),
                        fty_proto_description (decoded));
                    fty_proto_destroy (&decoded);
                    frame = zmsg_pop (recv);
            }
            zmsg_destroy (&recv);
        }
        }

    }

exit:
    mlm_client_destroy (&client);
    return ret;
}
