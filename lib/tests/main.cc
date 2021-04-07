/*  ========================================================================
    Copyright (C) 2020 Eaton
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
    ========================================================================
*/

#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include "fty_proto.h"
#include "mlm_test.h"
#include <filesystem>

typedef struct {
    const char *testname;           // test name, can be called from command line this way
    void (*test) (bool);            // function to run the test (or NULL for private tests)
} test_item_t;

static test_item_t
all_tests [] = {
    { "fty_proto_test", fty_proto_test },
    { "mlm_test", mlm_test },
    {NULL, NULL}          //  Sentinel
};

//  -------------------------------------------------------------------------
//  Run all tests.
//

static void
test_runall (bool verbose)
{
    printf ("Running fty-proto selftests...\n");
    test_item_t *item;
    for (item = all_tests; item->testname; item++) {
        if (item->test)
            item->test (verbose);
    }
    printf ("Tests passed OK\n");
}

TEST_CASE("All the stuff of before")
{
    std::cout << "Current path is " << std::filesystem::current_path() << std::endl;
    test_runall(true);
}
