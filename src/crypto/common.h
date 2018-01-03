// Copyright (c) 2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BTCNANO_CRYPTO_COMMON_H
#define BTCNANO_CRYPTO_COMMON_H

#if defined(HAVE_CONFIG_H)
#include "btcnano-config.h"
#endif

#include <cstdint>
#include <cstring>
#include <assert.h>

#include "sodium.h"
#include "compat/endian.h"

static inline uint16_t ReadLE16(const uint8_t *ptr) {
    uint16_t x;
    memcpy((char *)&x, ptr, 2);
    return le16toh(x);
}

static inline uint32_t ReadLE32(const uint8_t *ptr) {
    uint32_t x;
    memcpy((char *)&x, ptr, 4);
    return le32toh(x);
}

static inline uint64_t ReadLE64(const uint8_t *ptr) {
    uint64_t x;
    memcpy((char *)&x, ptr, 8);
    return le64toh(x);
}

static inline void WriteLE16(uint8_t *ptr, uint16_t x) {
    uint16_t v = htole16(x);
    memcpy(ptr, (char *)&v, 2);
}

static inline void WriteLE32(uint8_t *ptr, uint32_t x) {
    uint32_t v = htole32(x);
    memcpy(ptr, (char *)&v, 4);
}

static inline void WriteLE64(uint8_t *ptr, uint64_t x) {
    uint64_t v = htole64(x);
    memcpy(ptr, (char *)&v, 8);
}

static inline uint32_t ReadBE32(const uint8_t *ptr) {
    uint32_t x;
    memcpy((char *)&x, ptr, 4);
    return be32toh(x);
}

static inline uint64_t ReadBE64(const uint8_t *ptr) {
    uint64_t x;
    memcpy((char *)&x, ptr, 8);
    return be64toh(x);
}

static inline void WriteBE32(uint8_t *ptr, uint32_t x) {
    uint32_t v = htobe32(x);
    memcpy(ptr, (char *)&v, 4);
}

static inline void WriteBE64(uint8_t *ptr, uint64_t x) {
    uint64_t v = htobe64(x);
    memcpy(ptr, (char *)&v, 8);
}

/**
 * Return the smallest number n such that (x >> n) == 0 (or 64 if the highest
 * bit in x is set.
 */
uint64_t static inline CountBits(uint64_t x) {
#ifdef HAVE_DECL___BUILTIN_CLZL
    if (sizeof(unsigned long) >= sizeof(uint64_t)) {
        return x ? 8 * sizeof(unsigned long) - __builtin_clzl(x) : 0;
    }
#endif
#ifdef HAVE_DECL___BUILTIN_CLZLL
    if (sizeof(unsigned long long) >= sizeof(uint64_t)) {
        return x ? 8 * sizeof(unsigned long long) - __builtin_clzll(x) : 0;
    }
#endif
    int ret = 0;
    while (x) {
        x >>= 1;
        ++ret;
    }
    return ret;
}

int inline init_and_check_sodium()
{
    if (sodium_init() == -1) {
        return -1;
    }

    // What follows is a runtime test that ensures the version of libsodium
    // we're linked against checks that signatures are canonical (s < L).
    const unsigned char message[1] = { 0 };

    unsigned char pk[crypto_sign_PUBLICKEYBYTES];
    unsigned char sk[crypto_sign_SECRETKEYBYTES];
    unsigned char sig[crypto_sign_BYTES];

    crypto_sign_keypair(pk, sk);
    crypto_sign_detached(sig, NULL, message, sizeof(message), sk);

    assert(crypto_sign_verify_detached(sig, message, sizeof(message), pk) == 0);

    // Copied from libsodium/crypto_sign/ed25519/ref10/open.c
    static const unsigned char L[32] =
      { 0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58,
        0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10 };

    // Add L to S, which starts at sig[32].
    unsigned int s = 0;
    for (size_t i = 0; i < 32; i++) {
        s = sig[32 + i] + L[i] + (s >> 8);
        sig[32 + i] = s & 0xff;
    }

    assert(crypto_sign_verify_detached(sig, message, sizeof(message), pk) != 0);

    return 0;
}

#endif // BTCNANO_CRYPTO_COMMON_H
