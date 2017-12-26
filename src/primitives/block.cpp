// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Btcnano Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "primitives/block.h"

#include "crypto/common.h"
#include "hash.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

uint256 CBlockHeader::GetHash() const {
    return SerializeHash(*this);
}

std::string CBlock::ToString() const {
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, "
                   "hashMerkleRoot=%s, hashReserved=%s, nTime=%u, nBits=%08x, nNonce=%s, "
                   "vtx=%u)\n",
                   GetHash().ToString(), nVersion, hashPrevBlock.ToString(),
                   hashMerkleRoot.ToString(), hashReserved.ToString(), nTime, nBits, nNonce.ToString(), vtx.size());
/*
    s << strprintf("{");
    for (size_t i = 0; i < nSolution.size(); ++i)
    {   
    	s << strprintf("%d,", (int)nSolution[i]);
    }
    s << strprintf("}\n");
*/
	
    for (unsigned int i = 0; i < vtx.size(); i++) {
        s << "  " << vtx[i]->ToString() << "\n";
    }
    return s.str();
}
