// Copyright (c) 2011-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "chainparams.h"
#include "coins.h"
#include "config.h"
#include "consensus/consensus.h"
#include "consensus/merkle.h"
#include "consensus/validation.h"
#include "policy/policy.h"
#include "pubkey.h"
#include "script/standard.h"
#include "txmempool.h"
#include "uint256.h"
#include "util.h"
#include "utilstrencodings.h"
#include "validation.h"

#include "test/test_btcnano.h"

#include <memory>

#include <boost/test/unit_test.hpp>

// Equihash
#include "crypto/equihash.h"

BOOST_FIXTURE_TEST_SUITE(miner_tests, TestingSetup)

static CFeeRate blockMinFeeRate = CFeeRate(DEFAULT_BLOCK_MIN_TX_FEE);

static struct {
    const char *nonce_hex;
    const char *solution_hex;
} blockinfo[] = {
    {"00000000000000000000000000000000000000000000000000000000000022de", "007f388bed6b91756ea3e0866716ef6e9485fae6160195c7cda5c1e43f96ee359e105bcf4e8c293690420939124f04a0196363910421187811575929db40500b0bfdd1e8964aa334b801e3339a336d585a30852f1dc294a2d3d36f9ecc747458f3d41b4572415496df2a9fb1f882156cdabf9f65e681f38019865d6d47482277e24c9b8973eb34a41254faae4c5e2caa9dde5925ec118f3d8fa767ae00f434645957154367afe72000c59c79182c8faddd24424b9ebbb09ccd651b00540c96b9c7eec648a28a1d72c2e575d0f2250078511a011598db8e0788edf0ddc15ae24b62f63d6f93f71a2743a3c43ece55471a9802a76f31561a6f365c3647029bfa736395883afc0632bc25d4a8661b25d5aa0310f3c3fd3a183e75d359d6de3e5910b5dfbb74b7660af906917dc42b12e3e484aae1dbd20eaee037ef301572b7fc24d85b4aff9c82b27dcd421cee1639230d0188fec59f0dd4c0ced69c1ad07abd23692b1bd30735af942df597dcf6f403a36371bc416cf3e29a58570f586b05c357dc49515689788ad9581b8887dd913a41dc35e1ac9c9f9f4ea534eb6b36cc8af0299b6d3905750425da0366bdc59a7824477d7946b6f35c4ec90b8e61790fa74a4fa92396ea856661027828d40abb11dbe36bba516fe8ec8913106677285a4790d8034d1d1bf9fd87990889ddffc369b954a3d1c172be7e1812226c2b100cbe82c42bf4423456b6cb2bac3b4828135cb54f7a933a01f7f4a2057ad92136ba8e19fec313b412d43c089a71f06fd1625329b78d49ac92c59e4080932ddb1645910fd874dfb1f358e214231f62041acc41fd2c4e7b7127b3042459e1457f6b307fce9825aa4d2b942277f52665f2a77dd107b4f16cb3280f20c7551ff6cd855f97a6144131f69bab5648fb4b81261eefbf629094e8bcc4e36077f46d51a647da51fc01dca9a9ac12e2f7e2e2b1c9229dae099e95370177143d3b38ab661f19758494a01b32f0c27155b45a872a867dc50f9d76473695e9e2c4f9357f5ba6bb6c455d985f4e2c21486fde6576c6a8ceda6e010a7dc2b504130f429ac33376781ee4af5bbe8d768005bc4cb5092b15c4f296a8bd8c54a298eecd790a5161755a8605cc46bf890b8ff93d508501842b78c7261e5deeb1096891c528a300e57bf2f0aa9e8af2623cdf16bba20427704120484b6af8be26e4983d2685c783ce85d0174f84598719c6beefcc3603a94d4aa62750725df50671d7f9903ec255f779643ebd2fd8122fae3319e61928dcdaa44880d6a483140de63d2d7d7dc9dd449e0ee00d908e0f2164fc054198641e8fb0d74279c9b4117884b9335028a9f50c7223d3c03675ecf73329e52603f77f20cffba99356e51a365b75825f7db56d77542784f3c2663c493a2e564d73f753e9d6ebb0c2f2027a2330a7117c67a20507474fc47282a02cb572de17bbc7a335959316f74a05e3687cfb5227bc5b1b7f084f50902760e77740d420df9a495521c09b911e5f199a8343918b8386fc74f22552a76524a22c8c70ff06084e7fefe9b3ab98e004fadf35eb5f60483f287851712d90ebdd6b512877170d3b7fb34f16813917ae3b5ed54ede6081bdd7cc646fb336658121fd8fbafc52959b48d13375dfa4ce8616c157533a05ee1dc1120f215c348b54357d68adb4da7f5f48d55c005b3e7a23d05746e44d968f7601d4dbdff702861030d6e3a4140e6e1a29978be541f713f8e2cc9aa1ac32fecc941ee4aa4c41bc7f91ea5328ff87cbf35a8de17d1d3d1ad6d4384b0df52d10b3984d62e1678e86dd150ee425490bc727ee7107fda0f5d2433ab1c5d407be9d123fad5c201355601d926d3923787be86a4aa5be0b8d5750171ad658f8e97798b5dcaed46345a9af70c441"},
};

CBlockIndex CreateBlockIndex(int nHeight) {
    CBlockIndex index;
    index.nHeight = nHeight;
    index.pprev = chainActive.Tip();
    return index;
}

bool TestSequenceLocks(const CTransaction &tx, int flags) {
    LOCK(mempool.cs);
    return CheckSequenceLocks(tx, flags);
}

// Test suite for ancestor feerate transaction selection.
// Implemented as an additional function, rather than a separate test case, to
// allow reusing the blockchain created in CreateNewBlock_validity.
// Note that this test assumes blockprioritypercentage is 0.
void TestPackageSelection(const CChainParams &chainparams, CScript scriptPubKey,
                          std::vector<CTransactionRef> &txFirst) {
    // Test the ancestor feerate transaction selection.
    TestMemPoolEntryHelper entry;

    GlobalConfig config;

    // these 3 tests assume blockprioritypercentage is 0.
    config.SetBlockPriorityPercentage(0);

    // Test that a medium fee transaction will be selected after a higher fee
    // rate package with a low fee rate parent.
    CMutableTransaction tx;
    tx.vin.resize(1);
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetId();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    tx.vout[0].nValue = 5000000000LL - 1000;
    // This tx has a low fee: 1000 satoshis.
    // Save this txid for later use.
    uint256 hashParentTx = tx.GetId();
    mempool.addUnchecked(
        hashParentTx,
        entry.Fee(1000).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));

    // This tx has a medium fee: 10000 satoshis.
    tx.vin[0].prevout.hash = txFirst[1]->GetId();
    tx.vout[0].nValue = 5000000000LL - 10000;
    uint256 hashMediumFeeTx = tx.GetId();
    mempool.addUnchecked(
        hashMediumFeeTx,
        entry.Fee(10000).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));

    // This tx has a high fee, but depends on the first transaction.
    tx.vin[0].prevout.hash = hashParentTx;
    // 50k satoshi fee.
    tx.vout[0].nValue = 5000000000LL - 1000 - 50000;
    uint256 hashHighFeeTx = tx.GetId();
    mempool.addUnchecked(
        hashHighFeeTx,
        entry.Fee(50000).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));

    std::unique_ptr<CBlockTemplate> pblocktemplate =
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[1]->GetId() == hashParentTx);
    BOOST_CHECK(pblocktemplate->block.vtx[2]->GetId() == hashHighFeeTx);
    BOOST_CHECK(pblocktemplate->block.vtx[3]->GetId() == hashMediumFeeTx);

    // Test that a package below the block min tx fee doesn't get included
    tx.vin[0].prevout.hash = hashHighFeeTx;
    // 0 fee.
    tx.vout[0].nValue = 5000000000LL - 1000 - 50000;
    uint256 hashFreeTx = tx.GetId();
    mempool.addUnchecked(hashFreeTx, entry.Fee(0).FromTx(tx));
    size_t freeTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);

    // Calculate a fee on child transaction that will put the package just
    // below the block min tx fee (assuming 1 child tx of the same size).
    CAmount feeToUse = blockMinFeeRate.GetFee(2 * freeTxSize).GetSatoshis() - 1;

    tx.vin[0].prevout.hash = hashFreeTx;
    tx.vout[0].nValue = 5000000000LL - 1000 - 50000 - feeToUse;
    uint256 hashLowFeeTx = tx.GetId();
    mempool.addUnchecked(hashLowFeeTx, entry.Fee(feeToUse).FromTx(tx));
    pblocktemplate =
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey);
    // Verify that the free tx and the low fee tx didn't get selected.
    for (size_t i = 0; i < pblocktemplate->block.vtx.size(); ++i) {
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetId() != hashFreeTx);
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetId() != hashLowFeeTx);
    }

    // Test that packages above the min relay fee do get included, even if one
    // of the transactions is below the min relay fee. Remove the low fee
    // transaction and replace with a higher fee transaction
    mempool.removeRecursive(tx);
    // Now we should be just over the min relay fee.
    tx.vout[0].nValue -= 2;
    hashLowFeeTx = tx.GetId();
    mempool.addUnchecked(hashLowFeeTx, entry.Fee(feeToUse + 2).FromTx(tx));
    pblocktemplate =
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[4]->GetId() == hashFreeTx);
    BOOST_CHECK(pblocktemplate->block.vtx[5]->GetId() == hashLowFeeTx);

    // Test that transaction selection properly updates ancestor fee
    // calculations as ancestor transactions get included in a block. Add a
    // 0-fee transaction that has 2 outputs.
    tx.vin[0].prevout.hash = txFirst[2]->GetId();
    tx.vout.resize(2);
    tx.vout[0].nValue = 5000000000LL - 100000000;
    // 1BTN output.
    tx.vout[1].nValue = 100000000;
    uint256 hashFreeTx2 = tx.GetId();
    mempool.addUnchecked(hashFreeTx2,
                         entry.Fee(0).SpendsCoinbase(true).FromTx(tx));

    // This tx can't be mined by itself.
    tx.vin[0].prevout.hash = hashFreeTx2;
    tx.vout.resize(1);
    feeToUse = blockMinFeeRate.GetFee(freeTxSize).GetSatoshis();
    tx.vout[0].nValue = 5000000000LL - 100000000 - feeToUse;
    uint256 hashLowFeeTx2 = tx.GetId();
    mempool.addUnchecked(hashLowFeeTx2,
                         entry.Fee(feeToUse).SpendsCoinbase(false).FromTx(tx));
    pblocktemplate =
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey);

    // Verify that this tx isn't selected.
    for (size_t i = 0; i < pblocktemplate->block.vtx.size(); ++i) {
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetId() != hashFreeTx2);
        BOOST_CHECK(pblocktemplate->block.vtx[i]->GetId() != hashLowFeeTx2);
    }

    // This tx will be mineable, and should cause hashLowFeeTx2 to be selected
    // as well.
    tx.vin[0].prevout.n = 1;
    // 10k satoshi fee.
    tx.vout[0].nValue = 100000000 - 10000;
    mempool.addUnchecked(tx.GetId(), entry.Fee(10000).FromTx(tx));
    pblocktemplate =
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey);
    BOOST_CHECK(pblocktemplate->block.vtx[8]->GetId() == hashLowFeeTx2);
}

void TestCoinbaseMessageEB(uint64_t eb, std::string cbmsg) {

    GlobalConfig config;
    config.SetMaxBlockSize(eb);
    const CChainParams &chainparams = config.GetChainParams();

    CScript scriptPubKey =
        CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909"
                              "a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112"
                              "de5c384df7ba0b8d578a4c702b6bf11d5f")
                  << OP_CHECKSIG;

    std::unique_ptr<CBlockTemplate> pblocktemplate =
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey);

    CBlock *pblock = &pblocktemplate->block;

    // IncrementExtraNonce creates a valid coinbase and merkleRoot
    unsigned int extraNonce = 0;
    IncrementExtraNonce(config, pblock, chainActive.Tip(), extraNonce);
    unsigned int nHeight = chainActive.Tip()->nHeight + 1;
    std::vector<uint8_t> vec(cbmsg.begin(), cbmsg.end());
    BOOST_CHECK(pblock->vtx[0]->vin[0].scriptSig ==
                ((CScript() << nHeight << CScriptNum(extraNonce) << vec) +
                 COINBASE_FLAGS));
}

// Coinbase scriptSig has to contains the correct EB value
// converted to MB, rounded down to the first decimal
BOOST_AUTO_TEST_CASE(CheckCoinbase_EB) {
    TestCoinbaseMessageEB(1000001, "/EB1.0/");
    TestCoinbaseMessageEB(2000000, "/EB2.0/");
    TestCoinbaseMessageEB(8000000, "/EB8.0/");
    TestCoinbaseMessageEB(8320000, "/EB8.3/");
}

// NOTE: These tests rely on CreateNewBlock doing its own self-validation!
BOOST_AUTO_TEST_CASE(CreateNewBlock_validity) {
    // Note that by default, these tests run with size accounting enabled.
    const CChainParams &chainparams = Params(CBaseChainParams::MAIN);
    CScript scriptPubKey =
        CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909"
                              "a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112"
                              "de5c384df7ba0b8d578a4c702b6bf11d5f")
                  << OP_CHECKSIG;
    std::unique_ptr<CBlockTemplate> pblocktemplate;
    CMutableTransaction tx, tx2;
    CScript script;
    uint256 hash;
    TestMemPoolEntryHelper entry;
    entry.nFee = 11;
    entry.dPriority = 111.0;
    entry.nHeight = 11;

    GlobalConfig config;

    LOCK(cs_main);
    fCheckpointsEnabled = false;

    // Simple block creation, nothing special yet:
    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));

    // We can't make transactions until we have inputs. Therefore, load 100
    // blocks :)
    int baseheight = 0;
    std::vector<CTransactionRef> txFirst;
    for (unsigned int i = 0; i < sizeof(blockinfo) / sizeof(*blockinfo); ++i) {
        // pointer for convenience.
        CBlock *pblock = &pblocktemplate->block;
        pblock->nVersion = 1;
        pblock->nTime = chainActive.Tip()->GetMedianTimePast()+6*Params().GetConsensus().nPowTargetSpacing;
        CMutableTransaction txCoinbase(*pblock->vtx[0]);
        txCoinbase.nVersion = 1;
        txCoinbase.vin[0].scriptSig = CScript();
//        txCoinbase.vin[0].scriptSig.push_back(blockinfo[i].extranonce);
        txCoinbase.vin[0].scriptSig.push_back(chainActive.Height());
        // Ignore the (optional) segwit commitment added by CreateNewBlock (as
        // the hardcoded nonces don't account for this)
        txCoinbase.vout.resize(1);
        txCoinbase.vout[0].scriptPubKey = CScript();
        pblock->vtx[0] = MakeTransactionRef(std::move(txCoinbase));
        if (txFirst.size() == 0) baseheight = chainActive.Height();
        if (txFirst.size() < 4) txFirst.push_back(pblock->vtx[0]);
        pblock->hashMerkleRoot = BlockMerkleRoot(*pblock);
        pblock->nNonce = uint256S(blockinfo[i].nonce_hex);
        pblock->nSolution = ParseHex(blockinfo[i].solution_hex);

        std::shared_ptr<const CBlock> shared_pblock =
            std::make_shared<const CBlock>(*pblock);
        BOOST_CHECK(ProcessNewBlock(GetConfig(), shared_pblock, true, nullptr));
        pblock->hashPrevBlock = pblock->GetHash();
    }

    // Just to make sure we can still make simple blocks.
    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));

    const CAmount BLOCKSUBSIDY = 50 * COIN.GetSatoshis();
    const CAmount LOWFEE = CENT.GetSatoshis();
    const CAmount HIGHFEE = COIN.GetSatoshis();
    const CAmount HIGHERFEE = 4 * COIN.GetSatoshis();

    // block sigops > limit: 1000 CHECKMULTISIG + 1
    tx.vin.resize(1);
    // NOTE: OP_NOP is used to force 20 SigOps for the CHECKMULTISIG
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_0 << OP_0 << OP_NOP
                                    << OP_CHECKMULTISIG << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetId();
    tx.vin[0].prevout.n = 0;
    tx.vout.resize(1);
    tx.vout[0].nValue = BLOCKSUBSIDY;
    for (unsigned int i = 0; i < 1001; ++i) {
        tx.vout[0].nValue -= LOWFEE;
        hash = tx.GetId();
        // Only first tx spends coinbase.
        bool spendsCoinbase = (i == 0) ? true : false;
        // If we don't set the # of sig ops in the CTxMemPoolEntry, template
        // creation fails.
        mempool.addUnchecked(hash, entry.Fee(LOWFEE)
                                       .Time(GetTime())
                                       .SpendsCoinbase(spendsCoinbase)
                                       .FromTx(tx));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_CHECK_THROW(
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey),
        std::runtime_error);
    mempool.clear();

    tx.vin[0].prevout.hash = txFirst[0]->GetId();
    tx.vout[0].nValue = BLOCKSUBSIDY;
    for (unsigned int i = 0; i < 1001; ++i) {
        tx.vout[0].nValue -= LOWFEE;
        hash = tx.GetId();
        // Only first tx spends coinbase.
        bool spendsCoinbase = (i == 0) ? true : false;
        // If we do set the # of sig ops in the CTxMemPoolEntry, template
        // creation passes.
        mempool.addUnchecked(hash, entry.Fee(LOWFEE)
                                       .Time(GetTime())
                                       .SpendsCoinbase(spendsCoinbase)
                                       .SigOpsCost(80)
                                       .FromTx(tx));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));
    mempool.clear();

    // block size > limit
    tx.vin[0].scriptSig = CScript();
    // 18 * (520char + DROP) + OP_1 = 9433 bytes
    std::vector<uint8_t> vchData(520);
    for (unsigned int i = 0; i < 18; ++i)
        tx.vin[0].scriptSig << vchData << OP_DROP;
    tx.vin[0].scriptSig << OP_1;
    tx.vin[0].prevout.hash = txFirst[0]->GetId();
    tx.vout[0].nValue = BLOCKSUBSIDY;
    for (unsigned int i = 0; i < 128; ++i) {
        tx.vout[0].nValue -= LOWFEE;
        hash = tx.GetId();
        // Only first tx spends coinbase.
        bool spendsCoinbase = (i == 0) ? true : false;
        mempool.addUnchecked(hash, entry.Fee(LOWFEE)
                                       .Time(GetTime())
                                       .SpendsCoinbase(spendsCoinbase)
                                       .FromTx(tx));
        tx.vin[0].prevout.hash = hash;
    }
    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));
    mempool.clear();

    // Orphan in mempool, template creation fails.
    hash = tx.GetId();
    mempool.addUnchecked(hash, entry.Fee(LOWFEE).Time(GetTime()).FromTx(tx));
    BOOST_CHECK_THROW(
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey),
        std::runtime_error);
    mempool.clear();

    // Child with higher priority than parent.
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vin[0].prevout.hash = txFirst[1]->GetId();
    tx.vout[0].nValue = BLOCKSUBSIDY - HIGHFEE;
    hash = tx.GetId();
    mempool.addUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    tx.vin[0].prevout.hash = hash;
    tx.vin.resize(2);
    tx.vin[1].scriptSig = CScript() << OP_1;
    tx.vin[1].prevout.hash = txFirst[0]->GetId();
    tx.vin[1].prevout.n = 0;
    // First txn output + fresh coinbase - new txn fee.
    tx.vout[0].nValue = tx.vout[0].nValue + BLOCKSUBSIDY - HIGHERFEE;
    hash = tx.GetId();
    mempool.addUnchecked(
        hash,
        entry.Fee(HIGHERFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));
    mempool.clear();

    // Coinbase in mempool, template creation fails.
    tx.vin.resize(1);
    tx.vin[0].prevout.SetNull();
    tx.vin[0].scriptSig = CScript() << OP_0 << OP_1;
    tx.vout[0].nValue = 0;
    hash = tx.GetId();
    // Give it a fee so it'll get mined.
    mempool.addUnchecked(
        hash,
        entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));
    BOOST_CHECK_THROW(
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey),
        std::runtime_error);
    mempool.clear();

    // Invalid (pre-p2sh) txn in mempool, template creation fails.
    tx.vin[0].prevout.hash = txFirst[0]->GetId();
    tx.vin[0].prevout.n = 0;
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = BLOCKSUBSIDY - LOWFEE;
    script = CScript() << OP_0;
    tx.vout[0].scriptPubKey = GetScriptForDestination(CScriptID(script));
    hash = tx.GetId();
    mempool.addUnchecked(
        hash,
        entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    tx.vin[0].prevout.hash = hash;
    tx.vin[0].scriptSig = CScript()
                          << std::vector<uint8_t>(script.begin(), script.end());
    tx.vout[0].nValue -= LOWFEE;
    hash = tx.GetId();
    mempool.addUnchecked(
        hash,
        entry.Fee(LOWFEE).Time(GetTime()).SpendsCoinbase(false).FromTx(tx));
    BOOST_CHECK_THROW(
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey),
        std::runtime_error);
    mempool.clear();

    // Double spend txn pair in mempool, template creation fails.
    tx.vin[0].prevout.hash = txFirst[0]->GetId();
    tx.vin[0].scriptSig = CScript() << OP_1;
    tx.vout[0].nValue = BLOCKSUBSIDY - HIGHFEE;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    hash = tx.GetId();
    mempool.addUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    tx.vout[0].scriptPubKey = CScript() << OP_2;
    hash = tx.GetId();
    mempool.addUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));
    BOOST_CHECK_THROW(
        BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey),
        std::runtime_error);
    mempool.clear();

    // Subsidy changing.
    int nHeight = chainActive.Height();
    // Create an actual 209999-long block chain (without valid blocks).
    while (chainActive.Tip()->nHeight < 209999) {
        CBlockIndex *prev = chainActive.Tip();
        CBlockIndex *next = new CBlockIndex();
        next->phashBlock = new uint256(GetRandHash());
        pcoinsTip->SetBestBlock(next->GetBlockHash());
        next->pprev = prev;
        next->nHeight = prev->nHeight + 1;
        next->BuildSkip();
        chainActive.SetTip(next);
    }
    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));
    // Extend to a 210000-long block chain.
    while (chainActive.Tip()->nHeight < 210000) {
        CBlockIndex *prev = chainActive.Tip();
        CBlockIndex *next = new CBlockIndex();
        next->phashBlock = new uint256(GetRandHash());
        pcoinsTip->SetBestBlock(next->GetBlockHash());
        next->pprev = prev;
        next->nHeight = prev->nHeight + 1;
        next->BuildSkip();
        chainActive.SetTip(next);
    }
    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));
    // Delete the dummy blocks again.
    while (chainActive.Tip()->nHeight > nHeight) {
        CBlockIndex *del = chainActive.Tip();
        chainActive.SetTip(del->pprev);
        pcoinsTip->SetBestBlock(del->pprev->GetBlockHash());
        delete del->phashBlock;
        delete del;
    }

    // non-final txs in mempool
    SetMockTime(chainActive.Tip()->GetMedianTimePast() + 1);
    int flags = LOCKTIME_VERIFY_SEQUENCE | LOCKTIME_MEDIAN_TIME_PAST;
    // height map
    std::vector<int> prevheights;

    // Relative height locked.
    tx.nVersion = 2;
    tx.vin.resize(1);
    prevheights.resize(1);
    // Only 1 transaction.
    tx.vin[0].prevout.hash = txFirst[0]->GetId();
    tx.vin[0].prevout.n = 0;
    tx.vin[0].scriptSig = CScript() << OP_1;
    // txFirst[0] is the 2nd block
    tx.vin[0].nSequence = chainActive.Tip()->nHeight + 1;
    prevheights[0] = baseheight + 1;
    tx.vout.resize(1);
    tx.vout[0].nValue = BLOCKSUBSIDY - HIGHFEE;
    tx.vout[0].scriptPubKey = CScript() << OP_1;
    tx.nLockTime = 0;
    hash = tx.GetId();
    mempool.addUnchecked(
        hash,
        entry.Fee(HIGHFEE).Time(GetTime()).SpendsCoinbase(true).FromTx(tx));

    {
        // Locktime passes.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransactionForCurrentBlock(
            config, tx, state, chainparams.GetConsensus(), flags));
    }

    // Sequence locks fail.
    BOOST_CHECK(!TestSequenceLocks(tx, flags));
    // Sequence locks pass on 2nd block.
    BOOST_CHECK(
        SequenceLocks(tx, flags, &prevheights,
                      CreateBlockIndex(chainActive.Tip()->nHeight + 2)));

    // Relative time locked.
    tx.vin[0].prevout.hash = txFirst[1]->GetId();
    // txFirst[1] is the 3rd block.
    tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG |
                          (((chainActive.Tip()->GetMedianTimePast() + 1 -
                             chainActive[1]->GetMedianTimePast()) >>
                            CTxIn::SEQUENCE_LOCKTIME_GRANULARITY) +
                           1);
    prevheights[0] = baseheight + 2;
    hash = tx.GetId();
    mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));

    {
        // Locktime passes.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransactionForCurrentBlock(
            config, tx, state, chainparams.GetConsensus(), flags));
    }

    // Sequence locks fail.
    BOOST_CHECK(!TestSequenceLocks(tx, flags));

    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Trick the MedianTimePast.
        chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime +=
            512;
    }
    // Sequence locks pass 512 seconds later.
    BOOST_CHECK(
        SequenceLocks(tx, flags, &prevheights,
                      CreateBlockIndex(chainActive.Tip()->nHeight + 1)));
    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Undo tricked MTP.
        chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime -=
            512;
    }

    // Absolute height locked.
    tx.vin[0].prevout.hash = txFirst[2]->GetId();
    tx.vin[0].nSequence = CTxIn::SEQUENCE_FINAL - 1;
    prevheights[0] = baseheight + 3;
    tx.nLockTime = chainActive.Tip()->nHeight + 1;
    hash = tx.GetId();
    mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));

    {
        // Locktime fails.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(!ContextualCheckTransactionForCurrentBlock(
            config, tx, state, chainparams.GetConsensus(), flags));
        BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-nonfinal");
    }

    // Sequence locks pass.
    BOOST_CHECK(TestSequenceLocks(tx, flags));

    {
        // Locktime passes on 2nd block.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransaction(
            config, tx, state, chainparams.GetConsensus(),
            chainActive.Tip()->nHeight + 2,
            chainActive.Tip()->GetMedianTimePast()));
    }

    // Absolute time locked.
    tx.vin[0].prevout.hash = txFirst[3]->GetId();
    tx.nLockTime = chainActive.Tip()->GetMedianTimePast();
    prevheights.resize(1);
    prevheights[0] = baseheight + 4;
    hash = tx.GetId();
    mempool.addUnchecked(hash, entry.Time(GetTime()).FromTx(tx));

    {
        // Locktime fails.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(!ContextualCheckTransactionForCurrentBlock(
            config, tx, state, chainparams.GetConsensus(), flags));
        BOOST_CHECK_EQUAL(state.GetRejectReason(), "bad-txns-nonfinal");
    }

    // Sequence locks pass.
    BOOST_CHECK(TestSequenceLocks(tx, flags));

    {
        // Locktime passes 1 second later.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransaction(
            config, tx, state, chainparams.GetConsensus(),
            chainActive.Tip()->nHeight + 1,
            chainActive.Tip()->GetMedianTimePast() + 1));
    }

    // mempool-dependent transactions (not added)
    tx.vin[0].prevout.hash = hash;
    prevheights[0] = chainActive.Tip()->nHeight + 1;
    tx.nLockTime = 0;
    tx.vin[0].nSequence = 0;

    {
        // Locktime passes.
        GlobalConfig config;
        CValidationState state;
        BOOST_CHECK(ContextualCheckTransactionForCurrentBlock(
            config, tx, state, chainparams.GetConsensus(), flags));
    }

    // Sequence locks pass.
    BOOST_CHECK(TestSequenceLocks(tx, flags));
    tx.vin[0].nSequence = 1;
    // Sequence locks fail.
    BOOST_CHECK(!TestSequenceLocks(tx, flags));
    tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG;
    // Sequence locks pass.
    BOOST_CHECK(TestSequenceLocks(tx, flags));
    tx.vin[0].nSequence = CTxIn::SEQUENCE_LOCKTIME_TYPE_FLAG | 1;
    // Sequence locks fail.
    BOOST_CHECK(!TestSequenceLocks(tx, flags));

    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));

    // None of the of the absolute height/time locked tx should have made it
    // into the template because we still check IsFinalTx in CreateNewBlock, but
    // relative locked txs will if inconsistently added to mempool. For now
    // these will still generate a valid template until BIP68 soft fork.
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 3);
    // However if we advance height by 1 and time by 512, all of them should be
    // mined.
    for (int i = 0; i < CBlockIndex::nMedianTimeSpan; i++) {
        // Trick the MedianTimePast.
        chainActive.Tip()->GetAncestor(chainActive.Tip()->nHeight - i)->nTime +=
            512;
    }
    chainActive.Tip()->nHeight++;
    SetMockTime(chainActive.Tip()->GetMedianTimePast() + 1);

    BOOST_CHECK(
        pblocktemplate =
            BlockAssembler(config, chainparams).CreateNewBlock(scriptPubKey));
    BOOST_CHECK_EQUAL(pblocktemplate->block.vtx.size(), 5);

    chainActive.Tip()->nHeight--;
    SetMockTime(0);
    mempool.clear();

    TestPackageSelection(chainparams, scriptPubKey, txFirst);

    fCheckpointsEnabled = true;
}

void CheckBlockMaxSize(const CChainParams &chainparams, uint64_t size,
                       uint64_t expected) {
    GlobalConfig config;

    ForceSetArg("-blockmaxsize", std::to_string(size));

    BlockAssembler ba(config, chainparams);
    BOOST_CHECK_EQUAL(ba.GetMaxGeneratedBlockSize(), expected);
}

BOOST_AUTO_TEST_CASE(BlockAssembler_construction) {
    GlobalConfig config;
    const CChainParams &chainparams = Params();

    // We are working on a fake chain and need to protect ourselves.
    LOCK(cs_main);

    // Activate UAHF the dirty way
    const int64_t uahfHeight =
        config.GetChainParams().GetConsensus().uahfHeight;
    auto pindex = chainActive.Tip();
    for (size_t i = 0; pindex && i < 5; i++) {
        pindex->nHeight = uahfHeight + 5 - i;
        pindex = pindex->pprev;
    }

    BOOST_CHECK(IsUAHFenabledForCurrentBlock(config));

    // Test around historical 1MB (plus one byte because that's mandatory)
    config.SetMaxBlockSize(ONE_MEGABYTE + 1);
    CheckBlockMaxSize(chainparams, 0, 1000);
    CheckBlockMaxSize(chainparams, 1000, 1000);
    CheckBlockMaxSize(chainparams, 1001, 1001);
    CheckBlockMaxSize(chainparams, 12345, 12345);

    CheckBlockMaxSize(chainparams, ONE_MEGABYTE - 1001, ONE_MEGABYTE - 1001);
    CheckBlockMaxSize(chainparams, ONE_MEGABYTE - 1000, ONE_MEGABYTE - 1000);
    CheckBlockMaxSize(chainparams, ONE_MEGABYTE - 999, ONE_MEGABYTE - 999);
    CheckBlockMaxSize(chainparams, ONE_MEGABYTE, ONE_MEGABYTE - 999);

    // Test around higher limit such as 8MB
    static const auto EIGHT_MEGABYTES = 8 * ONE_MEGABYTE;
    config.SetMaxBlockSize(EIGHT_MEGABYTES);
    CheckBlockMaxSize(chainparams, EIGHT_MEGABYTES - 1001,
                      EIGHT_MEGABYTES - 1001);
    CheckBlockMaxSize(chainparams, EIGHT_MEGABYTES - 1000,
                      EIGHT_MEGABYTES - 1000);
    CheckBlockMaxSize(chainparams, EIGHT_MEGABYTES - 999,
                      EIGHT_MEGABYTES - 1000);
    CheckBlockMaxSize(chainparams, EIGHT_MEGABYTES, EIGHT_MEGABYTES - 1000);

    // Test around default cap
    config.SetMaxBlockSize(DEFAULT_MAX_BLOCK_SIZE);
    CheckBlockMaxSize(chainparams, DEFAULT_MAX_BLOCK_SIZE - 1001,
                      DEFAULT_MAX_BLOCK_SIZE - 1001);
    CheckBlockMaxSize(chainparams, DEFAULT_MAX_BLOCK_SIZE - 1000,
                      DEFAULT_MAX_BLOCK_SIZE - 1000);
    CheckBlockMaxSize(chainparams, DEFAULT_MAX_BLOCK_SIZE - 999,
                      DEFAULT_MAX_BLOCK_SIZE - 1000);
    CheckBlockMaxSize(chainparams, DEFAULT_MAX_BLOCK_SIZE,
                      DEFAULT_MAX_BLOCK_SIZE - 1000);

    // If the parameter is not specified, we use
    // DEFAULT_MAX_GENERATED_BLOCK_SIZE
    {
        ClearArg("-blockmaxsize");
        BlockAssembler ba(config, chainparams);
        BOOST_CHECK_EQUAL(ba.GetMaxGeneratedBlockSize(),
                          DEFAULT_MAX_GENERATED_BLOCK_SIZE);
    }
}

BOOST_AUTO_TEST_SUITE_END()
