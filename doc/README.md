Btcnano Nano
=====================

Setup
---------------------
Btcnano Nano is a fork of Btcnano Core, which is the original Btcnano client and builds the backbone of the network. It downloads and stores the entire history of Btcnano transactions (which is currently several GBs); depending on the speed of your computer and network connection, the synchronization process can take anywhere from a few hours to a day or more.

To download Btcnano Nano, visit [btcnano.org](https://download.btcnano.org/).

Running
---------------------
The following are some helpful notes on how to run Btcnano on your native platform.

### Unix

Unpack the files into a directory and run:

- `bin/btcnano-qt` (GUI) or
- `bin/btcnanod` (headless)

### Windows

Unpack the files into a directory, and then run btcnano-qt.exe.

### OS X

Drag btcnano-nano to your applications folder, and then run btcnano-nano.

### Need Help?

* See the documentation at the [Btcnano Wiki](https://en.btcnano.it/wiki/Main_Page)
for help and more information.
* Ask for help on the [Btcnano Nano Subreddit](https://www.reddit.com/r/BitNANO/).

Building
---------------------
The following are developer notes on how to build Btcnano on your native platform. They are not complete guides, but include notes on the necessary libraries, compile flags, etc.

- [OS X Build Notes](build-osx.md)
- [Unix Build Notes](build-unix.md)
- [Windows Build Notes](build-windows.md)
- [OpenBSD Build Notes](build-openbsd.md)
- [Gitian Building Guide](gitian-building.md)

Development
---------------------
The Btcnano Nano repo's [root README](/README.md) contains relevant information on the development process and automated testing.

- [Developer Notes](developer-notes.md)
- [Release Notes](release-notes.md)
- [Release Process](release-process.md)
- [Source Code Documentation (External Link)](https://dev.visucore.com/btcnano/doxygen/)
- [Translation Process](translation_process.md)
- [Translation Strings Policy](translation_strings_policy.md)
- [Travis CI](travis-ci.md)
- [Unauthenticated REST Interface](REST-interface.md)
- [Shared Libraries](shared-libraries.md)
- [BIPS](bips.md)
- [Dnsseed Policy](dnsseed-policy.md)
- [Benchmarking](benchmarking.md)

### Miscellaneous
- [Assets Attribution](assets-attribution.md)
- [Files](files.md)
- [Fuzz-testing](fuzzing.md)
- [Reduce Traffic](reduce-traffic.md)
- [Tor Support](tor.md)
- [Init Scripts (systemd/upstart/openrc)](init.md)
- [ZMQ](zmq.md)

License
---------------------
Distributed under the [MIT software license](/COPYING).
This product includes software developed by the OpenSSL Project for use in the [OpenSSL Toolkit](https://www.openssl.org/). This product includes
cryptographic software written by Eric Young ([eay@cryptsoft.com](mailto:eay@cryptsoft.com)), and UPnP software written by Thomas Bernard.
