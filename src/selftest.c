#include "bios_proto.h"
#include "malamute.h"

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

static const char *endpoint = "inproc://@/malamute";

static void
s_test_metrics (zactor_t *server)
{
    int r = 0;
    mlm_client_t *producer = mlm_client_new();
    mlm_client_connect (producer, endpoint, 5000, "producer");
    mlm_client_set_producer (producer, "METRICS");

    mlm_client_t *consumer = mlm_client_new();
    mlm_client_connect (consumer, endpoint, 5000, "consumer");
    mlm_client_set_consumer (consumer, "METRICS", ".*");

    // METRICS stream: Test case: send all values
    // send
    zmsg_t *msg = bios_proto_encode_metric (NULL, "TYPE", "ELEMENT_SRC", "VALUE", "UNITS", -1, "ELEMENT_DEST");
    assert (msg);
    r = mlm_client_send (producer, "TYPE@ELEMENT_SRC", &msg);
    assert (r == 0);

    // recv
    msg = mlm_client_recv (consumer);
    assert (msg);

    const char* subject = mlm_client_subject (consumer);
    assert (streq (subject, "TYPE@ELEMENT_SRC"));

    bios_proto_t *recv = bios_proto_decode (&msg);
    assert (recv);

    assert (streq (bios_proto_value (recv), "VALUE"));

    bios_proto_destroy (&recv);
    mlm_client_destroy (&producer);
    mlm_client_destroy (&consumer);
}

int main() {

    zactor_t *server = zactor_new (mlm_server, "Malamute");
    zstr_sendx (server, "BIND", endpoint, NULL);

    s_test_metrics(server);

    zactor_destroy (&server);
}
