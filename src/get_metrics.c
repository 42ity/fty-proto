#include <fty_proto.h>
#include <malamute.h>

/*
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
*/

int main(int argc, char** argv) {
    const char* endpoint = (argc == 3) ? argv[2] : "ipc://@/malamute";
    mlm_client_t *consumer = mlm_client_new();
    mlm_client_connect(consumer, endpoint, 5000, "metrics_getter");
    mlm_client_set_consumer(consumer, "METRICS", (argc == 2) ? argv[1] : ".*");
    while(!zsys_interrupted) {
        // Get message
        zmsg_t *msg = mlm_client_recv(consumer);
        if(msg == NULL)
            continue;
        fty_proto_t *yn = fty_proto_decode(&msg);
        if(yn == NULL)
            continue;
        printf("%s = %s %s\n",
                   mlm_client_subject(consumer),
                   fty_proto_value(yn),
                   fty_proto_unit(yn)
              );
    }
    mlm_client_destroy(&consumer);
}
