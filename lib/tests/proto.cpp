#include <catch2/catch.hpp>
#include "fty_proto.h"

//  --------------------------------------------------------------------------
//  Selftest

TEST_CASE("proto test")
{
    //  Simple create/destroy test
    fty_proto_t* self = fty_proto_new(0);
    REQUIRE(self);
    fty_proto_destroy(&self);

    //  Create pair of sockets we can send through
    zsock_t* input = zsock_new(ZMQ_ROUTER);
    REQUIRE(input);
    zsock_connect(input, "inproc://selftest-fty_proto");

    zsock_t* output = zsock_new(ZMQ_DEALER);
    REQUIRE(output);
    zsock_bind(output, "inproc://selftest-fty_proto");

    //  Encode/send/decode and verify each message type
    int          instance;
    fty_proto_t* copy;
    zconfig_t*   config;


    self = fty_proto_new(FTY_PROTO_METRIC);

    //  Check that _dup works on empty message
    copy = fty_proto_dup(self);
    REQUIRE(copy);
    fty_proto_destroy(&copy);

    fty_proto_aux_insert(self, "Name", "Brutus");
    fty_proto_aux_insert(self, "Age", "%d", 43);
    fty_proto_set_time(self, 123);
    fty_proto_set_ttl(self, 123);
    fty_proto_set_type(self, "Life is short but Now lasts for ever");
    fty_proto_set_name(self, "Life is short but Now lasts for ever");
    fty_proto_set_value(self, "Life is short but Now lasts for ever");
    fty_proto_set_unit(self, "Life is short but Now lasts for ever");
    // convert to zpl
    config = fty_proto_zpl(self, nullptr);
    zconfig_print(config);
    //  Send twice from same object
    fty_proto_send_again(self, output);
    fty_proto_send(&self, output);

    for (instance = 0; instance < 3; instance++) {
        if (instance < 2)
            self = fty_proto_recv(input);
        else {
            self = fty_proto_new_zpl(config);
            zconfig_destroy(&config);
        }
        REQUIRE(self);
        if (instance < 2)
            CHECK(fty_proto_routing_id(self));

        CHECK(fty_proto_aux_size(self) == 2);
        CHECK(streq(fty_proto_aux_string(self, "Name", "?"), "Brutus"));
        CHECK(fty_proto_aux_number(self, "Age", 0) == 43);
        CHECK(fty_proto_time(self) == 123);
        CHECK(fty_proto_ttl(self) == 123);
        CHECK(streq(fty_proto_type(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_name(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_value(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_unit(self), "Life is short but Now lasts for ever"));
        CHECK(fty_proto_metadata(self) == nullptr); // out of scope
        fty_proto_destroy(&self);
    }


    self = fty_proto_new(FTY_PROTO_ALERT);

    //  Check that _dup works on empty message
    copy = fty_proto_dup(self);
    REQUIRE(copy);
    fty_proto_destroy(&copy);

    fty_proto_aux_insert(self, "Name", "Brutus");
    fty_proto_aux_insert(self, "Age", "%d", 43);
    fty_proto_set_time(self, 123);
    fty_proto_set_ttl(self, 123);
    fty_proto_set_rule(self, "Life is short but Now lasts for ever");
    fty_proto_set_name(self, "Life is short but Now lasts for ever");
    fty_proto_set_state(self, "Life is short but Now lasts for ever");
    fty_proto_set_severity(self, "Life is short but Now lasts for ever");
    fty_proto_set_description(self, "Life is short but Now lasts for ever");
    fty_proto_action_append(self, "Name: %s", "Brutus");
    fty_proto_action_append(self, "Age: %d", 43);

    const char* METADATA = "{ \"Life\": \"is short but Now lasts for ever\" }";
    fty_proto_set_metadata(self, METADATA);

    // convert to zpl
    config = fty_proto_zpl(self, nullptr);
    zconfig_print(config);
    //  Send twice from same object
    fty_proto_send_again(self, output);
    fty_proto_send(&self, output);

    for (instance = 0; instance < 3; instance++) {
        if (instance < 2)
            self = fty_proto_recv(input);
        else {
            self = fty_proto_new_zpl(config);
            zconfig_destroy(&config);
        }
        REQUIRE(self);
        if (instance < 2)
            CHECK(fty_proto_routing_id(self));

        CHECK(fty_proto_aux_size(self) == 2);
        CHECK(streq(fty_proto_aux_string(self, "Name", "?"), "Brutus"));
        CHECK(fty_proto_aux_number(self, "Age", 0) == 43);
        CHECK(fty_proto_time(self) == 123);
        CHECK(fty_proto_ttl(self) == 123);
        CHECK(streq(fty_proto_rule(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_name(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_state(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_severity(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_description(self), "Life is short but Now lasts for ever"));
        CHECK(fty_proto_action_size(self) == 2);
        CHECK(streq(fty_proto_action_first(self), "Name: Brutus"));
        CHECK(streq(fty_proto_action_next(self), "Age: 43"));
        CHECK(streq(fty_proto_metadata(self), METADATA));
        fty_proto_destroy(&self);
    }


    self = fty_proto_new(FTY_PROTO_ASSET);

    //  Check that _dup works on empty message
    copy = fty_proto_dup(self);
    REQUIRE(copy);
    fty_proto_destroy(&copy);

    fty_proto_aux_insert(self, "Name", "Brutus");
    fty_proto_aux_insert(self, "Age", "%d", 43);
    fty_proto_set_name(self, "Life is short but Now lasts for ever");
    fty_proto_set_operation(self, "Life is short but Now lasts for ever");
    fty_proto_ext_insert(self, "Name", "Brutus");
    fty_proto_ext_insert(self, "Age", "%d", 43);
    // convert to zpl
    config = fty_proto_zpl(self, nullptr);
    zconfig_print(config);
    //  Send twice from same object
    fty_proto_send_again(self, output);
    fty_proto_send(&self, output);

    for (instance = 0; instance < 3; instance++) {
        if (instance < 2)
            self = fty_proto_recv(input);
        else {
            self = fty_proto_new_zpl(config);
            zconfig_destroy(&config);
        }
        REQUIRE(self);
        if (instance < 2)
            CHECK(fty_proto_routing_id(self));

        CHECK(fty_proto_aux_size(self) == 2);
        CHECK(streq(fty_proto_aux_string(self, "Name", "?"), "Brutus"));
        CHECK(fty_proto_aux_number(self, "Age", 0) == 43);
        CHECK(streq(fty_proto_name(self), "Life is short but Now lasts for ever"));
        CHECK(streq(fty_proto_operation(self), "Life is short but Now lasts for ever"));
        CHECK(fty_proto_ext_size(self) == 2);
        CHECK(streq(fty_proto_ext_string(self, "Name", "?"), "Brutus"));
        CHECK(fty_proto_ext_number(self, "Age", 0) == 43);
        CHECK(fty_proto_metadata(self) == nullptr); // out of scope
        fty_proto_destroy(&self);
    }

    zconfig_destroy(&config);
    zsock_destroy(&input);
    zsock_destroy(&output);
}
