# Copyright (c) 2013-2016 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

DIST_SUBDIRS = secp256k1 univalue

AM_LDFLAGS = $(PTHREAD_CFLAGS) $(LIBTOOL_LDFLAGS) $(HARDENED_LDFLAGS)
AM_CXXFLAGS = $(HARDENED_CXXFLAGS) $(ERROR_CXXFLAGS) -std=c++11
AM_CPPFLAGS = $(HARDENED_CPPFLAGS)
EXTRA_LIBRARIES =

if EMBEDDED_UNIVALUE
LIBUNIVALUE = univalue/libunivalue.la

$(LIBUNIVALUE): $(wildcard univalue/lib/*) $(wildcard univalue/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)
else
LIBUNIVALUE = $(UNIVALUE_LIBS)
endif

BTCNANO_CONFIG_INCLUDES=-I$(builddir)/config
BTCNANO_INCLUDES=-I$(builddir) -I$(builddir)/obj $(BDB_CPPFLAGS) $(BOOST_CPPFLAGS) $(LEVELDB_CPPFLAGS) $(CRYPTO_CFLAGS) $(SSL_CFLAGS)

BTCNANO_INCLUDES += -I$(srcdir)/secp256k1/include
BTCNANO_INCLUDES += $(UNIVALUE_CFLAGS)

BTCNANO_SEEDER_INCLUDES = -I$(srcdir)/seeder
BTCNANO_SEEDER_INCLUDES += $(BTCNANO_INCLUDES)

LIBBTCNANO_SODIUM=/usr/local/lib/libsodium.a
LIBBTCNANO_SERVER=libbtcnano_server.a
LIBBTCNANO_COMMON=libbtcnano_common.a
LIBBTCNANO_CONSENSUS=libbtcnano_consensus.a
LIBBTCNANO_CLI=libbtcnano_cli.a
LIBBTCNANO_UTIL=libbtcnano_util.a
LIBBTCNANO_CRYPTO=crypto/libbtcnano_crypto.a
LIBBTCNANOQT=qt/libbtcnanoqt.a
LIBSECP256K1=secp256k1/libsecp256k1.la

if ENABLE_ZMQ
LIBBTCNANO_ZMQ=libbtcnano_zmq.a
endif
if BUILD_BTCNANO_LIBS
LIBBTCNANOCONSENSUS=libbtcnanoconsensus.la
endif
if BUILD_BTCNANO_SEEDER
LIBBTCNANO_SEEDER=libbtcnano_seeder.a
endif
if ENABLE_WALLET
LIBBTCNANO_WALLET=libbtcnano_wallet.a
endif

$(LIBSECP256K1): $(wildcard secp256k1/src/*) $(wildcard secp256k1/include/*)
	$(AM_V_at)$(MAKE) $(AM_MAKEFLAGS) -C $(@D) $(@F)

# Make is not made aware of per-object dependencies to avoid limiting building parallelization
# But to build the less dependent modules first, we manually select their order here:
EXTRA_LIBRARIES += \
  $(LIBBTCNANO_CRYPTO) \
  $(LIBBTCNANO_UTIL) \
  $(LIBBTCNANO_COMMON) \
  $(LIBBTCNANO_CONSENSUS) \
  $(LIBBTCNANO_SERVER) \
  $(LIBBTCNANO_CLI) \
  $(LIBBTCNANO_SEEDER) \
  $(LIBBTCNANO_WALLET) \
  $(LIBBTCNANO_ZMQ)

lib_LTLIBRARIES = $(LIBBTCNANOCONSENSUS)

bin_PROGRAMS =
noinst_PROGRAMS =
TESTS =
BENCHMARKS =

if BUILD_BTCNANOD
  bin_PROGRAMS += btcnanod
endif

if BUILD_BTCNANO_SEEDER
  bin_PROGRAMS += btcnano-seeder
endif

if BUILD_BTCNANO_UTILS
  bin_PROGRAMS += btcnano-cli btcnano-tx
endif

.PHONY: FORCE check-symbols check-security
# btcnano core #
BTCNANO_CORE_H = \
  addrdb.h \
  addrman.h \
  base58.h \
  bloom.h \
  blockencodings.h \
  cashaddr.h \
  chain.h \
  chainparams.h \
  chainparamsbase.h \
  chainparamsseeds.h \
  checkpoints.h \
  checkqueue.h \
  clientversion.h \
  coins.h \
  compat.h \
  compat/byteswap.h \
  compat/endian.h \
  compat/sanity.h \
  compressor.h \
  config.h \
  consensus/consensus.h \
  core_io.h \
  core_memusage.h \
  cuckoocache.h \
  globals.h \
  httprpc.h \
  httpserver.h \
  indirectmap.h \
  init.h \
  key.h \
  keystore.h \
  dbwrapper.h \
  limitedmap.h \
  memusage.h \
  merkleblock.h \
  miner.h \
  net.h \
  net_processing.h \
  netaddress.h \
  netbase.h \
  netmessagemaker.h \
  noui.h \
  policy/fees.h \
  policy/policy.h \
  pow.h \
  protocol.h \
  random.h \
  reverselock.h \
  rpc/blockchain.h \
  rpc/client.h \
  rpc/misc.h \
  rpc/protocol.h \
  rpc/server.h \
  rpc/register.h \
  scheduler.h \
  script/scriptcache.h \
  script/sigcache.h \
  script/sign.h \
  script/standard.h \
  script/ismine.h \
  streams.h \
  support/allocators/secure.h \
  support/allocators/zeroafterfree.h \
  support/cleanse.h \
  support/events.h \
  support/lockedpool.h \
  sync.h \
  threadsafety.h \
  threadinterrupt.h \
  timedata.h \
  torcontrol.h \
  txdb.h \
  txmempool.h \
  ui_interface.h \
  undo.h \
  util.h \
  utilmoneystr.h \
  utiltime.h \
  validation.h \
  validationinterface.h \
  versionbits.h \
  wallet/coincontrol.h \
  wallet/crypter.h \
  wallet/db.h \
  wallet/finaltx.h \
  wallet/rpcdump.h \
  wallet/rpcwallet.h \
  wallet/wallet.h \
  wallet/walletdb.h \
  warnings.h \
  zmq/zmqabstractnotifier.h \
  zmq/zmqconfig.h\
  zmq/zmqnotificationinterface.h \
  zmq/zmqpublishnotifier.h


obj/build.h: FORCE
	@$(MKDIR_P) $(builddir)/obj
	@$(top_srcdir)/share/genbuild.sh $(abs_top_builddir)/src/obj/build.h \
	  $(abs_top_srcdir)
libbtcnano_util_a-clientversion.$(OBJEXT): obj/build.h

# server: shared between btcnanod and btcnano-qt
libbtcnano_server_a_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES) $(MINIUPNPC_CPPFLAGS) $(EVENT_CFLAGS) $(EVENT_PTHREADS_CFLAGS)
libbtcnano_server_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_server_a_SOURCES = \
  addrman.cpp \
  addrdb.cpp \
  bloom.cpp \
  blockencodings.cpp \
  chain.cpp \
  checkpoints.cpp \
  config.cpp \
  globals.cpp \
  httprpc.cpp \
  httpserver.cpp \
  init.cpp \
  dbwrapper.cpp \
  merkleblock.cpp \
  miner.cpp \
  net.cpp \
  net_processing.cpp \
  noui.cpp \
  policy/fees.cpp \
  policy/policy.cpp \
  pow.cpp \
  rest.cpp \
  rpc/abc.cpp \
  rpc/blockchain.cpp \
  rpc/mining.cpp \
  rpc/misc.cpp \
  rpc/net.cpp \
  rpc/rawtransaction.cpp \
  rpc/server.cpp \
  script/scriptcache.cpp \
  script/sigcache.cpp \
  script/ismine.cpp \
  timedata.cpp \
  torcontrol.cpp \
  txdb.cpp \
  txmempool.cpp \
  ui_interface.cpp \
  validation.cpp \
  validationinterface.cpp \
  versionbits.cpp \
  $(BTCNANO_CORE_H)

if ENABLE_ZMQ
libbtcnano_zmq_a_CPPFLAGS = $(BTCNANO_INCLUDES) $(ZMQ_CFLAGS)
libbtcnano_zmq_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_zmq_a_SOURCES = \
  zmq/zmqabstractnotifier.cpp \
  zmq/zmqnotificationinterface.cpp \
  zmq/zmqpublishnotifier.cpp
endif


# wallet: shared between btcnanod and btcnano-qt, but only linked
# when wallet enabled
libbtcnano_wallet_a_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES)
libbtcnano_wallet_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_wallet_a_SOURCES = \
  wallet/crypter.cpp \
  wallet/db.cpp \
  wallet/finaltx.cpp \
  wallet/rpcdump.cpp \
  wallet/rpcwallet.cpp \
  wallet/wallet.cpp \
  wallet/walletdb.cpp \
  $(BTCNANO_CORE_H)

# crypto primitives library
crypto_libbtcnano_crypto_a_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_CONFIG_INCLUDES)
crypto_libbtcnano_crypto_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
crypto_libbtcnano_crypto_a_SOURCES = \
  crypto/aes.cpp \
  crypto/aes.h \
  crypto/chacha20.h \
  crypto/chacha20.cpp \
  crypto/common.h \
  crypto/hmac_sha256.cpp \
  crypto/hmac_sha256.h \
  crypto/hmac_sha512.cpp \
  crypto/hmac_sha512.h \
  crypto/ripemd160.cpp \
  crypto/ripemd160.h \
  crypto/sha1.cpp \
  crypto/sha1.h \
  crypto/sha256.cpp \
  crypto/sha256.h \
  crypto/sha512.cpp \
  crypto/sha512.h \
  crypto/equihash.cpp \
  crypto/equihash.h \
  crypto/equihash.tcc

if ENABLE_MINING
EQUIHASH_TROMP_SOURCES = \
  pow/tromp/equi_miner.h \
  pow/tromp/equi.h \
  pow/tromp/osx_barrier.h \
  pow/tromp/equi_miner.cpp \
  pow/tromp/equi.cpp \
  pow/tromp/osx_barrier.cpp \
  pow/nano.h \
  pow/nano.cpp

crypto_libbtcnano_crypto_a_CPPFLAGS += \
  -DEQUIHASH_TROMP_ATOMIC
crypto_libbtcnano_crypto_a_SOURCES += \
  ${EQUIHASH_TROMP_SOURCES}
endif

# consensus: shared between all executables that validate any consensus rules.
libbtcnano_consensus_a_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES)
libbtcnano_consensus_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_consensus_a_SOURCES = \
  amount.h \
  arith_uint256.cpp \
  arith_uint256.h \
  consensus/merkle.cpp \
  consensus/merkle.h \
  consensus/params.h \
  consensus/validation.h \
  hash.cpp \
  hash.h \
  prevector.h \
  primitives/block.cpp \
  primitives/block.h \
  primitives/transaction.cpp \
  primitives/transaction.h \
  pubkey.cpp \
  pubkey.h \
  script/btcnanoconsensus.cpp \
  script/interpreter.cpp \
  script/interpreter.h \
  script/script.cpp \
  script/script.h \
  script/script_error.cpp \
  script/script_error.h \
  serialize.h \
  tinyformat.h \
  uint256.cpp \
  uint256.h \
  utilstrencodings.cpp \
  utilstrencodings.h \
  version.h

# common: shared between btcnanod, and btcnano-qt and non-server tools
libbtcnano_common_a_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES)
libbtcnano_common_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_common_a_SOURCES = \
  amount.cpp \
  base58.cpp \
  cashaddr.cpp \
  chainparams.cpp \
  coins.cpp \
  compressor.cpp \
  core_read.cpp \
  core_write.cpp \
  key.cpp \
  keystore.cpp \
  netaddress.cpp \
  netbase.cpp \
  protocol.cpp \
  scheduler.cpp \
  script/sign.cpp \
  script/standard.cpp \
  warnings.cpp \
  $(BTCNANO_CORE_H)

# util: shared between all executables.
# This library *must* be included to make sure that the glibc
# backward-compatibility objects and their sanity checks are linked.
libbtcnano_util_a_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES)
libbtcnano_util_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_util_a_SOURCES = \
  support/lockedpool.cpp \
  chainparamsbase.cpp \
  clientversion.cpp \
  compat/glibc_sanity.cpp \
  compat/glibcxx_sanity.cpp \
  compat/strnlen.cpp \
  random.cpp \
  rpc/protocol.cpp \
  support/cleanse.cpp \
  sync.cpp \
  threadinterrupt.cpp \
  util.cpp \
  utilmoneystr.cpp \
  utilstrencodings.cpp \
  utiltime.cpp \
  $(BTCNANO_CORE_H)

if GLIBC_BACK_COMPAT
libbtcnano_util_a_SOURCES += compat/glibc_compat.cpp
endif

# cli: shared between btcnano-cli and btcnano-qt
libbtcnano_cli_a_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES)
libbtcnano_cli_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_cli_a_SOURCES = \
  rpc/client.cpp \
  $(BTCNANO_CORE_H)

# seeder library
libbtcnano_seeder_a_CPPFLAGS = $(AM_CPPFLAGS) $(PIE_FLAGS) $(BTCNANO_SEEDER_INCLUDES)
libbtcnano_seeder_a_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
libbtcnano_seeder_a_SOURCES = \
  seeder/btcnano.cpp \
  seeder/btcnano.h \
  seeder/compat.h \
  seeder/db.cpp \
  seeder/db.h \
  seeder/dns.c \
  seeder/dns.h \
  seeder/netbase.cpp \
  seeder/netbase.h \
  seeder/protocol.cpp \
  seeder/protocol.h \
  seeder/strlcpy.h \
  seeder/util.cpp \
  seeder/util.h

nodist_libbtcnano_util_a_SOURCES = $(srcdir)/obj/build.h
#

# btcnanod binary #
btcnanod_SOURCES = btcnanod.cpp
btcnanod_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES)
btcnanod_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
btcnanod_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
btcnanod_SOURCES += btcnanod-res.rc
endif

btcnanod_LDADD = \
  $(LIBBTCNANO_SERVER) \
  $(LIBBTCNANO_COMMON) \
  $(LIBUNIVALUE) \
  $(LIBBTCNANO_UTIL) \
  $(LIBBTCNANO_WALLET) \
  $(LIBBTCNANO_ZMQ) \
  $(LIBBTCNANO_CONSENSUS) \
  $(LIBBTCNANO_CRYPTO) \
  $(LIBLEVELDB) \
  $(LIBLEVELDB_SSE42) \
  $(LIBMEMENV) \
  $(LIBSECP256K1)

btcnanod_LDADD += $(LIBBTCNANO_SODIUM)

btcnanod_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)

# btcnano-cli binary #
btcnano_cli_SOURCES = btcnano-cli.cpp
btcnano_cli_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES) $(EVENT_CFLAGS)
btcnano_cli_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
btcnano_cli_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
btcnano_cli_SOURCES += btcnano-cli-res.rc
endif

btcnano_cli_LDADD = \
  $(LIBBTCNANO_CLI) \
  $(LIBUNIVALUE) \
  $(LIBBTCNANO_UTIL) \
  $(LIBBTCNANO_CRYPTO)

btcnano_cli_LDADD += $(LIBBTCNANO_SODIUM)

btcnano_cli_LDADD += $(BOOST_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(EVENT_LIBS)
#

# btcnano-seeder binary #
btcnano_seeder_SOURCES = seeder/main.cpp
btcnano_seeder_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_SEEDER_INCLUDES)
btcnano_seeder_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
btcnano_seeder_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

btcnano_seeder_LDADD = \
  $(LIBBTCNANO_SEEDER) \
  $(LIBBTCNANO_UTIL) \
  $(LIBBTCNANO_CRYPTO)

btcnano_seeder_LDADD += $(BOOST_LIBS) $(BDB_LIBS) $(SSL_LIBS) $(CRYPTO_LIBS) $(MINIUPNPC_LIBS) $(EVENT_PTHREADS_LIBS) $(EVENT_LIBS) $(ZMQ_LIBS)
#

# btcnano-tx binary #
btcnano_tx_SOURCES = btcnano-tx.cpp
btcnano_tx_CPPFLAGS = $(AM_CPPFLAGS) $(BTCNANO_INCLUDES)
btcnano_tx_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)
btcnano_tx_LDFLAGS = $(RELDFLAGS) $(AM_LDFLAGS) $(LIBTOOL_APP_LDFLAGS)

if TARGET_WINDOWS
btcnano_tx_SOURCES += btcnano-tx-res.rc
endif

btcnano_tx_LDADD = \
  $(LIBUNIVALUE) \
  $(LIBBTCNANO_COMMON) \
  $(LIBBTCNANO_UTIL) \
  $(LIBBTCNANO_CONSENSUS) \
  $(LIBBTCNANO_CRYPTO) \
  $(LIBSECP256K1)

btcnano_tx_LDADD += $(LIBBTCNANO_SODIUM)

btcnano_tx_LDADD += $(BOOST_LIBS) $(CRYPTO_LIBS)
#

# btcnanoconsensus library #
if BUILD_BTCNANO_LIBS
include_HEADERS = script/btcnanoconsensus.h
libbtcnanoconsensus_la_SOURCES = $(crypto_libbtcnano_crypto_a_SOURCES) $(libbtcnano_consensus_a_SOURCES)

if GLIBC_BACK_COMPAT
  libbtcnanoconsensus_la_SOURCES += compat/glibc_compat.cpp
endif

libbtcnanoconsensus_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined $(RELDFLAGS)
libbtcnanoconsensus_la_LIBADD = $(LIBSECP256K1)
libbtcnanoconsensus_la_CPPFLAGS = $(AM_CPPFLAGS) -I$(builddir)/obj -I$(srcdir)/secp256k1/include -DBUILD_BTCNANO_INTERNAL
libbtcnanoconsensus_la_CXXFLAGS = $(AM_CXXFLAGS) $(PIE_FLAGS)

endif
#

CTAES_DIST =  crypto/ctaes/bench.c
CTAES_DIST += crypto/ctaes/ctaes.c
CTAES_DIST += crypto/ctaes/ctaes.h
CTAES_DIST += crypto/ctaes/README.md
CTAES_DIST += crypto/ctaes/test.c

CLEANFILES = $(EXTRA_LIBRARIES)

CLEANFILES += *.gcda *.gcno
CLEANFILES += compat/*.gcda compat/*.gcno
CLEANFILES += consensus/*.gcda consensus/*.gcno
CLEANFILES += crypto/*.gcda crypto/*.gcno
CLEANFILES += policy/*.gcda policy/*.gcno
CLEANFILES += primitives/*.gcda primitives/*.gcno
CLEANFILES += script/*.gcda script/*.gcno
CLEANFILES += support/*.gcda support/*.gcno
CLEANFILES += univalue/*.gcda univalue/*.gcno
CLEANFILES += wallet/*.gcda wallet/*.gcno
CLEANFILES += wallet/test/*.gcda wallet/test/*.gcno
CLEANFILES += zmq/*.gcda zmq/*.gcno

DISTCLEANFILES = obj/build.h

EXTRA_DIST = $(CTAES_DIST)

clean-local:
	-$(MAKE) -C secp256k1 clean
	-$(MAKE) -C univalue clean
	-rm -f leveldb/*/*.gcda leveldb/*/*.gcno leveldb/helpers/memenv/*.gcda leveldb/helpers/memenv/*.gcno
	-rm -rf test/__pycache__

.rc.o:
	@test -f $(WINDRES)
	## FIXME: How to get the appropriate modulename_CPPFLAGS in here?
	$(AM_V_GEN) $(WINDRES) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(CPPFLAGS) -DWINDRES_PREPROC -i $< -o $@

.mm.o:
	$(AM_V_CXX) $(OBJCXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	  $(CPPFLAGS) $(AM_CXXFLAGS) $(QT_INCLUDES) $(AM_CXXFLAGS) $(PIE_FLAGS) $(CXXFLAGS) -c -o $@ $<

check-symbols: $(bin_PROGRAMS)
if GLIBC_BACK_COMPAT
	@echo "Checking glibc back compat..."
	$(AM_V_at) READELF=$(READELF) CPPFILT=$(CPPFILT) $(top_srcdir)/contrib/devtools/symbol-check.py < $(bin_PROGRAMS)
endif

check-security: $(bin_PROGRAMS)
if HARDEN
	@echo "Checking binary security..."
	$(AM_V_at) READELF=$(READELF) OBJDUMP=$(OBJDUMP) $(top_srcdir)/contrib/devtools/security-check.py < $(bin_PROGRAMS)
endif

%.pb.cc %.pb.h: %.proto
	@test -f $(PROTOC)
	$(AM_V_GEN) $(PROTOC) --cpp_out=$(@D) --proto_path=$(<D) $<

if EMBEDDED_LEVELDB
include Makefile.leveldb.include
endif

if ENABLE_TESTS
include Makefile.test.include
endif

if ENABLE_BENCH
include Makefile.bench.include
endif

if ENABLE_QT
include Makefile.qt.include
endif

if ENABLE_QT_TESTS
include Makefile.qttest.include
endif
