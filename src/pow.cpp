// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Btcnano Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "crypto/equihash.h"
#include "primitives/block.h"
#include "streams.h"
#include "uint256.h"
#include "util.h"

#include <algorithm>

unsigned int GetNextWorkRequired(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params)
{
	unsigned int pow = UintToArith256(params.powLimit).GetCompact();
	// genesis
	if (pindexLast == NULL)
		return pow;

	const CBlockIndex *pindexFirst = pindexLast;
	arith_uint256 bnTot {0};
	for (int i = 0; pindexFirst && i < params.nPowAveragingWindow; ++i)
	{
		arith_uint256 bnTmp;	
		bnTmp.SetCompact(pindexFirst->nBits);
		bnTot += bnTmp;
		pindexFirst = pindexFirst->pprev;
	}

	if (pindexFirst == NULL)
	{
		return pow;
	}
	
	arith_uint256 bnAvg{bnTot / params.nPowAveragingWindow};

	return CalculateNextWorkRequired(bnAvg, pindexLast->GetMedianTimePast(), pindexFirst->GetMedianTimePast(), params);
}

unsigned int CalculateNextWorkRequired(arith_uint256 bnAvg, int64_t lastBlockTime, int64_t firstBlockTime, const Consensus::Params &params)
{
	// limit adjustment
	int64_t actualTimeSpan = lastBlockTime - firstBlockTime;

	actualTimeSpan = std::max(actualTimeSpan, params.MinActualTimespan());
	actualTimeSpan = std::min(actualTimeSpan, params.MaxActualTimespan());

	// retarget
	const arith_uint256 pow = UintToArith256(params.powLimit);
	arith_uint256 bnNew {bnAvg};
	bnNew /= params.AveragingWindowTimespan();
	bnNew *= actualTimeSpan;
	
	bnNew = std::min(pow, bnNew);

	return bnNew.GetCompact();
}

bool CheckEquihashSolution(const CBlockHeader *pblock, const CChainParams& params)
{
    size_t n = params.n();
    size_t k = params.k();

    // Hash state
    crypto_generichash_blake2b_state state;
    EhInitialiseState(n, k, state);

    // I = the block header minus nonce and solution.
    CEquihashInput I{*pblock};
    // I||V
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << I;
    ss << pblock->nNonce;

    // H(I||V||...
    crypto_generichash_blake2b_update(&state, (unsigned char*)&ss[0], ss.size());

    #ifdef ENABLE_RUST
    // Ensure that our Rust interactions are working in production builds. This is
    // temporary and should be removed.
    {
        assert(librustzcash_xor(0x0f0f0f0f0f0f0f0f, 0x1111111111111111) == 0x1e1e1e1e1e1e1e1e);
    }
    #endif // ENABLE_RUST

    bool isValid;
    EhIsValidSolution(n, k, state, pblock->nSolution, isValid);
    if (!isValid)
        return error("CheckEquihashSolution(): invalid solution");

    return true;
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return error("CheckProofOfWork(): nBits below minimum work");

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("CheckProofOfWork(): hash doesn't match nBits");

    return true;
}

