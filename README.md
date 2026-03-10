# Core protocol implementation for FTY

## How to build

To build fty-info project run:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=usr -DBUILD_TESTING=On ..
make
sudo make install
```

## Scope and goals

Implementation of core protocols (assets, metrics, alerts messages) for 42ity project.

It defines three types of main messages/objects on the system
* metrics
* assets
* alerts

## Ownership and license

The contributors are listed in AUTHORS. This project uses the GPL2+ license, see LICENSE.

fty-proto uses the [C4.1 (Collective Code Construction Contract)](http://rfc.zeromq.org/spec:22) process for contributions.

fty-proto uses the [CLASS (C Language Style for Scalabilty)](http://rfc.zeromq.org/spec:21) guide for code style.

To report an issue, use the [42Ity issue tracker](https://github.com/42ity/fty-proto/issues) at github.com.
