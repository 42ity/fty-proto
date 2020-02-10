# Core protocol implementation for FTY

## Scope and goals

Implementation of core protocols (assets, metrics, alerts messages) for 42ity
project. fty-proto defines [grammar](src/fty_proto.bnf) of messages as well as [C API](doc/fty_proto.txt) to serialize/deserialize messages.

It defines three types of main messages on the system
* metrics
* assets
* alerts

## Ownership and license

The contributors are listed in AUTHORS. This project uses the GPL2+ license, see LICENSE.

fty-proto uses the [C4.1 (Collective Code Construction Contract)](http://rfc.zeromq.org/spec:22) process for contributions.

fty-proto uses the [CLASS (C Language Style for Scalabilty)](http://rfc.zeromq.org/spec:21) guide for code style.

To report an issue, use the [42Ity issue tracker](https://github.com/42ity/fty-proto/issues) at github.com.

## Building and Installing

You need libzmq and czmq prior to building fty-proto.

### Using Open Build Service

Zeromq community maintain packages [network:zeromq](https://download.opensuse.org/...) for a lot of Linux distributions. Check if your system is supported and install devel packagage of czmq (libczmq-dev, czmq-devel).

### From source code

Here's how to build CZMQ from GitHub (building from packages is very similar, you don't clone a repo but unpack a tarball), including the libzmq (ZeroMQ core) library (NOTE: skip ldconfig on OSX):

    git clone git://github.com/zeromq/libzmq.git
    cd libzmq
    ./autogen.sh
    # do not specify "--with-libsodium" if you prefer to use internal tweetnacl security implementation (recommended for development)
    ./configure --with-libsodium
    make check
    sudo make install
    sudo ldconfig
    cd ..

    git clone git://github.com/zeromq/czmq.git
    cd czmq
    ./autogen.sh && ./configure && make check
    sudo make install
    sudo ldconfig
    cd ..

Then build fty-proto itself

    git clone git://github.com/42ity/fty-proto.git
    cd fty-proto
    ./autogen.sh && ./configure && make check
    sudo make install
    sudo ldconfig
    cd ..

## API summary

See [fty_proto.txt](doc/fty_proto.txt) for API summary and example code.

## Development

fty-proto is developed through a test-driven process that guarantees no memory violations or leaks in the code:

* Modify a class or method.
* Update the test method for that class.
* Run the 'selftest' script, which uses the Valgrind memcheck tool.
* Repeat until perfect.

Always ask - can I make it simpler?

## How to compile and test projects using fty-proto by 42ITy standards

### project.xml
Add this block in the `project.xml` file :
````
    <use project = "fty-proto" libname = "libfty_proto" header = "ftyproto.h" prefix = "fty_proto"
        min_major = "1" min_minor = "0" min_patch = "0"
        repository = "https://github.com/42ity/fty-proto.git"
        release = "master"
        test = "fty_proto_test" />
````
