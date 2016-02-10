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
            die ("Unknown command %s", argv[argn]);
        argn ++;
    }

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
                zsys_info ("set_consumer on METRICS .*, ALERTS .* and ASSETS .*");
            r = mlm_client_set_consumer (client, "METRICS", ".*");
            if (r == -1)
                die ("set consumer METRICS failed", NULL);

            r = mlm_client_set_consumer (client, "ASSETS", ".*");
            if (r == -1)
                die ("set consumer ASSETS failed", NULL);

            r = mlm_client_set_consumer (client, "ALERTS", ".*");
            if (r == -1)
                die ("set consumer ALERTS failed", NULL);
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

exit:
    mlm_client_destroy (&client);
    return ret;
}
