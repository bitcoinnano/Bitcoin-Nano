// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BTCNANO_BTCNANOCONSENSUS_H
#define BTCNANO_BTCNANOCONSENSUS_H

#include <cstdint>

#if defined(BUILD_BTCNANO_INTERNAL) && defined(HAVE_CONFIG_H)
#include "config/btcnano-config.h"
#if defined(_WIN32)
#if defined(DLL_EXPORT)
#if defined(HAVE_FUNC_ATTRIBUTE_DLLEXPORT)
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL
#endif
#endif
#elif defined(HAVE_FUNC_ATTRIBUTE_VISIBILITY)
#define EXPORT_SYMBOL __attribute__((visibility("default")))
#endif
#elif defined(MSC_VER) && !defined(STATIC_LIBBTCNANOCONSENSUS)
#define EXPORT_SYMBOL __declspec(dllimport)
#endif

#ifndef EXPORT_SYMBOL
#define EXPORT_SYMBOL
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define BTCNANOCONSENSUS_API_VER 1

typedef enum btcnanoconsensus_error_t {
    btcnanoconsensus_ERR_OK = 0,
    btcnanoconsensus_ERR_TX_INDEX,
    btcnanoconsensus_ERR_TX_SIZE_MISMATCH,
    btcnanoconsensus_ERR_TX_DESERIALIZE,
    btcnanoconsensus_ERR_AMOUNT_REQUIRED,
    btcnanoconsensus_ERR_INVALID_FLAGS,
} btcnanoconsensus_error;

/** Script verification flags */
enum {
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_NONE = 0,
    // evaluate P2SH (BIP16) subscripts
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_P2SH = (1U << 0),
    // enforce strict DER (BIP66) compliance
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_DERSIG = (1U << 2),
    // enforce NULLDUMMY (BIP147)
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY = (1U << 4),
    // enable CHECKLOCKTIMEVERIFY (BIP65)
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY = (1U << 9),
    // enable CHECKSEQUENCEVERIFY (BIP112)
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY = (1U << 10),
    // enable WITNESS (BIP141)
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_WITNESS_DEPRECATED = (1U << 11),
    // enable SIGHASH_FORKID replay protection
    btcnanoconsensus_SCRIPT_ENABLE_SIGHASH_FORKID = (1U << 16),
    btcnanoconsensus_SCRIPT_FLAGS_VERIFY_ALL =
        btcnanoconsensus_SCRIPT_FLAGS_VERIFY_P2SH |
        btcnanoconsensus_SCRIPT_FLAGS_VERIFY_DERSIG |
        btcnanoconsensus_SCRIPT_FLAGS_VERIFY_NULLDUMMY |
        btcnanoconsensus_SCRIPT_FLAGS_VERIFY_CHECKLOCKTIMEVERIFY |
        btcnanoconsensus_SCRIPT_FLAGS_VERIFY_CHECKSEQUENCEVERIFY,
};

/// Returns 1 if the input nIn of the serialized transaction pointed to by txTo
/// correctly spends the scriptPubKey pointed to by scriptPubKey under the
/// additional constraints specified by flags.
/// If not nullptr, err will contain an error/success code for the operation
EXPORT_SYMBOL int btcnanoconsensus_verify_script(
    const uint8_t *scriptPubKey, unsigned int scriptPubKeyLen,
    const uint8_t *txTo, unsigned int txToLen, unsigned int nIn,
    unsigned int flags, btcnanoconsensus_error *err);

EXPORT_SYMBOL int btcnanoconsensus_verify_script_with_amount(
    const uint8_t *scriptPubKey, unsigned int scriptPubKeyLen, int64_t amount,
    const uint8_t *txTo, unsigned int txToLen, unsigned int nIn,
    unsigned int flags, btcnanoconsensus_error *err);

EXPORT_SYMBOL unsigned int btcnanoconsensus_version();

#ifdef __cplusplus
} // extern "C"
#endif

#undef EXPORT_SYMBOL

#endif // BTCNANO_BTCNANOCONSENSUS_H
