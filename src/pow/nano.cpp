// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2017-2018 The Bitcoin Nano developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"
#include "arith_uint256.h"
#include "pow/tromp/equi_miner.h"
#include "streams.h"
#include "chainparams.h"
#include "crypto/equihash.h"
#include "util.h"
#include "pow/nano.h"

#include <string>
#include <iostream>
#include "arith_uint256.h"
#include <fstream>

// partly copied from https://github.com/zcash/zcash/blob/master/src/miner.cpp#L581
bool equihash_(std::string solver, CBlock *pblock, int n, int k)
{
    arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);

    // Hash state
    crypto_generichash_blake2b_state state;
    EhInitialiseState(n, k, state);

    // I = the block header minus nonce and solution.
    CEquihashInput I{*pblock};
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << I;

    // H(I||...
    crypto_generichash_blake2b_update(&state, (unsigned char*)&ss[0], ss.size());

    // H(I||V||...
    crypto_generichash_blake2b_state curr_state;
    curr_state = state;
    crypto_generichash_blake2b_update(&curr_state,
        pblock->nNonce.begin(),
        pblock->nNonce.size());

    // (x_1, x_2, ...) = A(I, V, n, k)
    LogPrint("pow", "Running Equihash solver \"%s\" with nNonce = %s\n",
             solver, pblock->nNonce.ToString());

    std::function<bool(std::vector<unsigned char>)> validBlock =
    [&pblock, &hashTarget](std::vector<unsigned char> soln) {
        // Write the solution to the hash and compute the result.
        pblock->nSolution = soln;

        if (UintToArith256(pblock->GetHash()) > hashTarget) {
           return false;
        }

        // Found a solution
        LogPrintf("NanoMiner:\n");
        LogPrintf("proof-of-work found  \n  hash: %s  \ntarget: %s\n", pblock->GetHash().GetHex(), hashTarget.GetHex());
        return true;
    };

    if (solver == "tromp") {
        // Create solver and initialize it.
        equi eq(1);
        eq.setstate(&curr_state);

        // Intialization done, start algo driver.
        eq.digit0(0);
        eq.xfull = eq.bfull = eq.hfull = 0;
        eq.showbsizes(0);
        for (u32 r = 1; r < WK; r++) {
            (r&1) ? eq.digitodd(r, 0) : eq.digiteven(r, 0);
            eq.xfull = eq.bfull = eq.hfull = 0;
            eq.showbsizes(r);
        }
        eq.digitK(0);

        // Convert solution indices to byte array (decompress) and pass it to validBlock method.
        for (size_t s = 0; s < eq.nsols; s++) {
            std::vector<eh_index> index_vector(PROOFSIZE);
            for (size_t i = 0; i < PROOFSIZE; i++) {
                index_vector[i] = eq.sols[s][i];
            }
            std::vector<unsigned char> sol_char = GetMinimalFromIndices(index_vector, DIGITBITS);
            if (validBlock(sol_char)) 
                return true;
        }
    } else {
        try {
            if (EhOptimisedSolve(n, k, curr_state, validBlock)) 
                return true;
        } catch (std::exception&) {
            LogPrintf("pow/nano.cpp: ", "execption catched...");
        }
    }
    return false;
}
