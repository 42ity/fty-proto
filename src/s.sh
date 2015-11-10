#!/bin/sh

# build the selftest

die () {
    echo "${@}" >&2
    exit 1
}

[ -f .libs/libbiosproto.so ] || \
    die "Can't locate .libs/libbiosproto.so, perhaps run make?"

gcc selftest.c -I ../include -L .libs/ -lmlm -lczmq -lbiosproto || \
    die

LD_LIBRARY_PATH=.libs/ ./a.out
