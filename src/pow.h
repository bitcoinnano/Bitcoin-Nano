// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BTCNANO_POW_H
#define BTCNANO_POW_H

#include "consensus/params.h"
#include "uint256.h"

#include <stdint.h>

class CBlockHeader;
class CBlockIndex;
class CChainParams;
class uint256;
class arith_uint256;

unsigned int GetNextWorkRequired(const CBlockIndex* pindexPrev, 
								 const CBlockHeader *pblock, 
								 const Consensus::Params &);
unsigned int CalculateNextWorkRequired(arith_uint256 bnAvg,
                                       int64_t firstBlockTime,
									   int64_t lastBlockTime,
                                       const Consensus::Params&);

/** Check whether the Equihash solution in a block header is valid */
bool CheckEquihashSolution(const CBlockHeader *pblock, const CChainParams&);

/** Check whether a block hash satisfies the proof-of-work requirement specified by nBits */
bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params&);

#endif // BTCNANO_POW_H
