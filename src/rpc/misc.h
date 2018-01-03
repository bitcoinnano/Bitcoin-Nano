// Copyright (c) 2017 The Bitcoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BTCNANO_RPCMISC_H
#define BTCNANO_RPCMISC_H

#include "script/script.h"

class UniValue;

CScript createmultisig_redeemScript(const UniValue &params);

#endif // BTCNANO_RPCBLOCKCHAIN_H
