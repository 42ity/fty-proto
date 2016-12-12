#!/bin/sh
#
# Copyright (C) 2015 Eaton
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
#! \file    s.sh
#  \brief   build the selftest for libftyproto
#  \author  Michal Vyskocil <michalvyskocil@Eaton.com>
#  \author  Tomas Halman <TomasHalman@Eaton.com>
#  \author  Michal Hrusecky <MichalHrusecky@Eaton.com>
#  \author  Chernikava, Alena <AlenaChernikava@Eaton.com>
#  \details Not yet documented file
#

die () {
    echo "${@}" >&2
    exit 1
}

[ -f .libs/libftyproto.so ] || \
    die "Can't locate .libs/libftyproto.so, perhaps run make?"

gcc selftest.c -I ../include -L .libs/ -lmlm -lczmq -lftyproto || \
    die

LD_LIBRARY_PATH=.libs/ ./a.out
