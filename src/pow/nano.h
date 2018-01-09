// Copyright (c) 2017-2018 The Bitcoin Nano developers                                                                                                                                                                                                                        
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BTCNANO_NANO_H
#define BTCNANO_NANO_H

#include "primitives/block.h"
#include "chainparams.h"
#include <string>

bool equihash_(std::string solver, CBlock *pblock, int n, int k);

#endif
