/*
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
*/

#include "fty_proto.h"
#include <catch2/catch.hpp>
#include <malamute.h>

// Selftest for FTY core protocols using malamute client API

/*
 *
 *  Howto use zproto_codec_c_v1 generated protocol with Malamute API
 *
 *  Problems found:
 *   1.) Q: Can you use zproto_codec_c?
 *       A: It supports only plain sockets, there are no ways to convert
 *       things to zmsg_t.
 *       So with malamute API one would need to construct mlm_proto
 *       messages on his own to have SUBJECTS et all working ...
 *       Or to extend the codec ...
 *
 *   2.) Q: How to deal with ymsg wrapper?
 *       A: Tested the <headers> which is a set of fields in each message
 *
 *   3.) Q: How this will work for other messages (alerts, assets, tresholds?)
 *       A: Who knows ..., send a PR
 *
 *   4.) Q: I don't like it, it's ugly
 *       A: Waiting on the pull requests
 *
 *
 */

#include "fty_proto.h"
#include <malamute.h>

static const char* endpoint = "inproc://@/malamute";

static void s_test_metrics()
{
    int r = 0;

    mlm_client_t* producer = mlm_client_new();
    mlm_client_connect(producer, endpoint, 5000, "producer");
    mlm_client_set_producer(producer, "METRICS");

    mlm_client_t* consumer = mlm_client_new();
    mlm_client_connect(consumer, endpoint, 5000, "consumer");
    mlm_client_set_consumer(consumer, "METRICS", ".*");

    // METRICS stream: Test case: send all values
    // send
    zhash_t* aux = zhash_new();
    REQUIRE(aux);
    r = zhash_insert(aux, FTY_PROTO_METRIC_ELEMENT_DEST, const_cast<char*>("ELEMENT_DEST"));
    REQUIRE(r == 0);

    zmsg_t* msg = fty_proto_encode_metric(aux, 30, 10, "TYPE", "ELEMENT_SRC", "VALUE", "UNITS");
    REQUIRE(msg);
    zhash_destroy(&aux);

    r = mlm_client_send(producer, "TYPE@ELEMENT_SRC", &msg);
    REQUIRE(r == 0);

    // recv
    msg = mlm_client_recv(consumer);
    REQUIRE(msg);

    const char* subject = mlm_client_subject(consumer);
    CHECK(streq(subject, "TYPE@ELEMENT_SRC"));

    fty_proto_t* recv = fty_proto_decode(&msg);
    REQUIRE(recv);

    CHECK(streq(fty_proto_value(recv), "VALUE"));
    CHECK(streq(fty_proto_aux_string(recv, FTY_PROTO_METRIC_ELEMENT_DEST, "N/A"), "ELEMENT_DEST"));

    fty_proto_destroy(&recv);
    mlm_client_destroy(&producer);
    mlm_client_destroy(&consumer);
}

TEST_CASE("mlm test")
{
    zactor_t* server = zactor_new(mlm_server, const_cast<char*>("Malamute"));
    REQUIRE(server);
    zstr_sendx(server, "BIND", endpoint, NULL);
    s_test_metrics();
    zactor_destroy(&server);
}
