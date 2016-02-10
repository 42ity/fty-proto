Format:         1.0
Source:         biosproto
Version:        0.1.0-1
Binary:         libbiosproto0, biosproto-dev
Architecture:   any all
Maintainer:     John Doe <John.Doe@example.com>
Standards-Version: 3.9.5
Build-Depends: bison, debhelper (>= 8),
    pkg-config,
    automake,
    autoconf,
    libtool,
    libsodium-dev,
    libzmq4-dev,
    libuuid-dev,
    libczmq-dev,
    libmlm-dev,
    dh-autoreconf

Package-List:
 libbiosproto0 deb net optional arch=any
 biosproto-dev deb libdevel optional arch=any

