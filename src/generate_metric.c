#include <bios_proto.h>
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
    if(argc != 5 && argc != 6) {
        printf("Usage: %s type source value units [endpoint]\n", argv[0]);
        exit(-1);
    }
    const char* endpoint = (argc == 6) ? argv[5] : "ipc://@/malamute";
    mlm_client_t *producer = mlm_client_new();
    mlm_client_connect(producer, endpoint, 5000, "metric_generator");
    mlm_client_set_producer(producer, "METRICS");
    zmsg_t *msg = bios_proto_encode_metric(NULL, argv[1], argv[2], argv[3], argv[4], -1);
    char *buff;
    if(asprintf(&buff, "%s@%s", argv[1], argv[2]) < 3) {
        printf("Can't allocate subject\n");
        exit(-1);
    }
    mlm_client_send(producer, buff, &msg);
    mlm_client_destroy(&producer);
}
