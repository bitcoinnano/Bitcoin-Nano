// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2016 The Btcnano Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "consensus/merkle.h"

// Equihash
#include "crypto/equihash.h"
#include "validation.h"
#include "net.h"

#include "tinyformat.h"
#include "util.h"
#include "utilstrencodings.h"

#include <cassert>
#include "chainparamsseeds.h"
#include "pow/nano.h"

// turn on to generate genesis
//#define PRINTS_OUT

#ifdef PRINTS_OUT
#include <fstream>
using namespace std;

void mery(CBlock *pblock, std::string net, int n, int k, std::string solver)
{                                                                                                                                                                                                                                                                             
    ofstream fout;                                                                                                                                                                                                                                                            
    fout.open("/root/genesis", std::fstream::in | std::fstream::out | std::fstream::app);
    arith_uint256 hashTarget = arith_uint256().SetCompact(pblock->nBits);

    cout << "calculating net = " << net << ", n = " << n
         << ", k = " << k << ", using solver = " << solver << endl;
    fout << "calculating net = " << net << ", n = " << n
         << ", k = " << k << ", using solver = " << solver << endl;

    cout << "hash target = " << hashTarget.ToString() << endl;                                                                                                                                                                                                                
    fout << "hash target = " << hashTarget.ToString() << endl;

    while (true)
    {
	    cout << "nonce = " << pblock->nNonce.ToString() << endl;

        if (equihash_(solver, pblock, n, k)) break;
        pblock->nNonce = ArithToUint256(UintToArith256(pblock->nNonce) + 1);
    }

    cout << "\t\t\t" << net << " genesis" << endl;
    fout << "\t\t\t" << net << " genesis" << endl;
    cout << pblock->ToString()<< endl;
    fout << pblock->ToString()<< endl;
	
	fout.close();
}
#endif

typedef std::vector<unsigned char> vuc;

// Far into the future.
static const std::string ANTI_REPLAY_COMMITMENT =
    "Btcnano: A Peer-to-Peer Electronic Nano System";

static std::vector<uint8_t> GetAntiReplayCommitment() {
    return std::vector<uint8_t>(std::begin(ANTI_REPLAY_COMMITMENT),
                                std::end(ANTI_REPLAY_COMMITMENT));
}

static CBlock CreateGenesisBlock(const char *pszTimestamp,
                                 const CScript &genesisOutputScript,
                                 uint32_t nTime, uint256 nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const Amount genesisReward, vuc sol) {
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig =
        CScript() << 486604799 << CScriptNum(4)
                  << std::vector<uint8_t>((const uint8_t *)pszTimestamp,
                                          (const uint8_t *)pszTimestamp +
                                              strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nNonce = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
	genesis.hashReserved.SetNull();
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
	genesis.nSolution = sol;
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint256 nNonce,
                                 uint32_t nBits, int32_t nVersion,
                                 const Amount genesisReward, vuc sol) {
    const char *pszTimestamp = "Merry Christmas";
    const CScript genesisOutputScript =
        CScript() << ParseHex("040a70ee17d16b202b7087e059ad63"
                              "94b618a1982cf14a90f927f388154a83580fe195733f5d0141440a88d0e9"
							  "be1df348456fb652a705d4bea2637c"
							  "bbfb622c2d")
                  << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce,
                              nBits, nVersion, genesisReward, sol);
}

arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidySlowStartInterval = 4370; // slow start mining in roughly a month, 50 blocks excluded for premine.
		consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x0001c7adb02e5d228747383a9d95e480c971224369696ccf181c16fdae72a250");
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.antiReplayOpReturnSunsetHeight = 530000;
        consensus.antiReplayOpReturnCommitment = GetAntiReplayCommitment();
        consensus.powLimit = uint256S(
            "0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;                                                                                                                                                                                                                                   
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up

        // two weeks
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60;
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        // 95% of 2016
        consensus.nRuleChangeActivationThreshold = 1916;
        // nPowTargetTimespan / nPowTargetSpacing
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        // December 13, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            1513130600;
        // December 13, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            1544630400;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        // December 13, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1513130600;
        // December 13, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1544630400;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x0");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid =
            uint256S("0x00");

        //
        consensus.uahfHeight = 0;

        /**
         * The message start string is designed to be unlikely to occur in
         * normal data. The characters are rarely used upper ASCII, not valid as
         * UTF-8, and produce a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0xf9;
        pchMessageStart[1] = 0xbe;
        pchMessageStart[2] = 0xb4;
        pchMessageStart[3] = 0xd9;
		pchCashMessageStart[0] = 0xfe;
        pchCashMessageStart[1] = 0xbe;
        pchCashMessageStart[2] = 0xbf;
        pchCashMessageStart[3] = 0xde;						

        nDefaultPort = 9556;
        nPruneAfterHeight = 100000;

		// Equihash params
        const size_t N = 200, K = 9;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

		vuc sol{0,1,215,76,9,70,148,25,201,222,1,144,89,46,45,153,115,246,190,173,167,43,190,198,87,74,151,227,3,95,45,163,32,116,249,14,17,187,225,156,37,76,1,73,1,206,151,5,34,23,181,95,112,118,47,147,213,137,18,26,46,184,143,19,133,2,198,207,87,255,169,73,97,201,174,99,208,48,91,59,76,125,33,57,3,247,103,114,29,31,178,217,66,5,212,66,254,205,31,138,253,28,185,36,69,33,40,57,175,165,80,12,81,136,88,199,231,201,73,88,147,41,11,127,217,199,9,178,13,208,175,144,204,255,60,74,215,75,135,216,142,14,12,190,53,204,197,34,125,255,37,140,237,196,7,126,126,115,111,151,179,244,22,24,115,113,182,60,12,119,235,229,232,160,4,33,105,208,34,196,184,72,56,210,147,181,249,94,216,27,183,125,93,231,208,190,221,71,13,66,203,173,246,96,84,190,56,61,161,71,23,41,182,90,1,78,66,19,248,158,69,231,158,118,197,86,151,29,28,136,218,64,56,221,102,180,226,254,251,207,208,203,215,173,123,5,211,14,170,26,61,171,14,46,140,11,180,137,35,121,45,209,242,147,134,192,17,28,194,49,45,157,150,44,154,97,126,234,234,13,103,136,39,21,210,165,216,33,142,2,125,151,78,27,33,83,52,90,51,239,28,85,210,225,115,0,194,111,72,109,81,23,153,230,255,85,119,99,87,206,220,224,87,145,105,245,250,176,210,35,6,115,199,159,105,51,0,204,21,93,44,70,102,222,235,224,210,66,40,238,47,156,199,239,255,53,82,14,195,79,179,52,214,120,106,224,182,103,221,240,84,168,162,208,70,58,166,31,15,221,239,78,199,142,112,189,5,253,179,168,229,68,236,10,233,5,252,193,43,21,227,163,200,177,26,47,44,231,86,184,241,129,212,143,234,112,158,220,155,98,10,192,13,40,248,213,241,92,218,71,72,157,78,217,198,10,194,48,123,192,228,17,230,175,73,109,250,27,35,243,224,129,111,196,195,118,197,9,21,215,247,218,32,103,21,200,95,169,154,103,130,0,162,65,167,184,159,169,30,160,241,196,165,61,142,117,192,222,144,188,72,204,65,243,247,102,198,0,109,73,163,153,198,141,0,248,40,120,188,192,88,127,246,1,32,193,145,211,148,228,162,142,122,124,226,32,64,201,84,118,214,173,7,220,26,120,41,255,84,160,218,195,17,223,168,220,45,52,230,182,154,37,154,251,195,239,3,210,172,200,79,189,42,164,191,147,77,87,85,115,229,30,35,201,235,71,171,105,30,235,228,240,210,156,213,186,79,217,5,74,40,116,128,2,118,240,88,179,64,119,115,19,237,71,88,18,253,135,183,72,190,10,99,56,221,197,91,196,236,180,154,253,219,210,21,131,216,21,112,29,32,111,18,3,133,204,136,164,219,100,131,201,216,46,8,189,79,91,223,140,199,37,195,166,172,112,155,60,223,175,148,119,242,204,82,184,147,1,156,124,16,209,2,16,95,216,253,213,175,154,244,89,218,3,21,107,129,182,166,177,159,251,119,8,23,233,17,211,148,82,45,252,199,19,141,243,69,128,50,191,171,250,157,78,2,207,105,69,143,8,228,41,72,240,82,114,137,106,29,96,163,206,145,102,153,79,59,102,251,140,169,145,57,201,186,21,32,132,96,8,71,160,191,189,85,231,6,96,47,137,98,96,99,195,236,195,245,208,210,75,50,130,1,173,146,29,230,9,23,113,61,101,74,149,132,101,82,241,235,58,221,42,199,51,169,221,140,244,8,222,122,153,64,22,220,78,247,80,210,215,202,251,75,133,37,240,88,42,227,28,98,178,212,155,21,122,73,79,160,226,115,59,164,42,69,239,113,20,103,61,9,10,172,153,182,90,14,63,197,18,87,241,74,249,79,18,92,69,24,35,18,89,35,67,39,236,34,96,199,185,77,183,129,215,236,112,138,66,86,253,190,22,35,254,181,147,128,31,136,87,41,214,179,98,25,248,174,21,126,241,14,124,66,61,137,253,243,51,97,94,41,164,181,216,234,143,208,51,94,227,58,184,78,47,14,164,235,207,89,179,2,47,156,219,146,148,241,184,2,209,231,164,113,127,25,47,98,254,202,155,211,211,10,221,145,181,114,225,62,50,250,32,110,17,73,161,22,149,115,51,93,184,101,219,225,174,35,32,107,44,45,39,68,168,29,74,143,34,231,209,221,80,13,189,85,134,153,152,11,201,237,159,119,143,198,190,225,226,4,213,33,173,123,221,208,111,146,72,48,119,254,62,164,237,143,192,180,143,118,17,139,232,152,33,10,241,35,34,64,241,238,164,247,255,52,140,34,73,85,254,14,63,47,234,166,68,180,73,181,203,193,11,185,179,199,70,136,27,157,23,159,25,219,146,54,216,213,153,5,140,99,199,185,250,217,234,130,72,143,253,147,41,10,183,57,6,172,88,60,144,245,156,235,215,245,245,171,143,209,114,30,242,126,11,90,214,140,190,68,21,98,36,139,210,86,187,196,57,117,235,166,211,131,216,13,33,232,160,184,87,159,192,210,102,68,184,57,185,115,57,63,114,46,254,6,19,210,196,193,147,75,115,177,166,237,84,154,169,89,226,85,117,244,54,71,76,12,46,68,18,224,84,27,10,230,230,147,240,171,83,138,65,39,118,206,150,1,85,47,164,68,232,155,192,54,234,94,10,174,122,226,71,123,25,248,62,177,252,22,80,230,102,210,79,32,157,168,47,24,79,110,245,242,146,171,23,253,228,63,66,38,252,171,165,43,128,167,238,185,197,4,182,227,147,126,73,59,61,118,215,25,182,223,71,26,99,243,215,177,90,4,131,103,232,35,245,45,107,189,113,97,78,135,167,225,40,172,72,59,131,112,123,175,175,96,72,15,152,57,31,30,250,47,201,235,240,122,172,16,79,159,147,51,100,193,97,227,78,149,221,215,166,63,107,68,68,127,91,219,222,247,127,17,71,109,239,214,253,182,34,195,126,15,63};
        genesis = CreateGenesisBlock(1513755219, uint256S("1240f"), 0x1f07ffff, 1, 50 * COIN, sol);

        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "main", nEquihashN, nEquihashK, "tromp");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x0001c7adb02e5d228747383a9d95e480c971224369696ccf181c16fdae72a250"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x6952a3e7c73d5a56fefe3a2ceb40e99b7a92c4b84ff9b8f803734e380f463f92"));

        // Note that of those with the service bits flag, most only support a
        // subset of possible options.
        // Btcnano seeder
        vSeeds.push_back(
            CDNSSeedData("btcnano.org", "seed.btcnano.org", true));
/*		vSeeds.push_back(
			CDNSSeedData("btcnano1.org", "seed.btcnano1.org", true));
        vSeeds.push_back(
            CDNSSeedData("btcnano2.org", "seed.btcnano2.org", true));
        vSeeds.push_back(
            CDNSSeedData("btcnano3.org", "seed.btcnano4.org", true));
*/
		// start with 'N'
        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 53);
		// start with '8'
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 18);
		// private key start with 'y'
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 140);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        vFixedSeeds = std::vector<SeedSpec6>(
            pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            .mapCheckpoints = {
				{0, uint256S("0x0001c7adb02e5d228747383a9d95e480c971224369696ccf181c16fdae72a250")}
                                  }};

        chainTxData = ChainTxData{
            // UNIX timestamp of last known number of transactions.
            1513755219,
            // Total number of transactions between genesis and that timestamp
            // (the tx=... number in the SetBestChain debug.log lines)
            0,
            // Estimated number of transactions per second after that timestamp.
            0};
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
		consensus.nSubsidySlowStartInterval = 4370;
        consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x06c9d468d834f7b9d67dcffe01f93a55ee9654cc62ae6bb284f22696eb8ed4e2");
        // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP65Height = 0;
        // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.BIP66Height = 0;
        consensus.antiReplayOpReturnSunsetHeight = 1250000;
        consensus.antiReplayOpReturnCommitment = GetAntiReplayCommitment();
        consensus.powLimit = uint256S(
            "07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
		// Equihash
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        // two weeks
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60;
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        // 75% for testchains
        consensus.nRuleChangeActivationThreshold = 1512;
        // nPowTargetTimespan / nPowTargetSpacing
        consensus.nMinerConfirmationWindow = 2016;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        // December 13, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            1513652643;
        // December 13, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            1545188568;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        // December 13, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime =
            1513652643 ;
        // December 13, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
			1545188568;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork =
            uint256S("0x0");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid =
            uint256S("0x0");

        // Aug, 1 hard fork
        consensus.uahfHeight = 0;

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;
        pchCashMessageStart[0] = 0x0f;                                                                                                                                                                                                                                        
        pchCashMessageStart[1] = 0x1e;
        pchCashMessageStart[2] = 0xab;
        pchCashMessageStart[3] = 0xcd;
        nDefaultPort = 19556;
        nPruneAfterHeight = 1000;

        // Equihash params
        const size_t N = 144, K = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;		
		
		vuc sol{27,127,201,210,226,236,11,17,126,15,196,171,20,224,175,214,83,212,147,207,49,95,243,235,108,30,144,192,254,45,48,204,183,77,181,224,58,134,160,229,245,248,134,146,33,198,249,137,87,194,28,62,100,100,220,64,149,203,80,109,159,34,251,197,154,174,170,187,71,144,190,61,203,110,71,117,210,134,107,250,222,34,144,44,212,114,239,232,22,85,79,149,11,62,134,198,23,162,105,85};
        genesis = CreateGenesisBlock(1513758241, uint256S("7"), 0x2007ffff, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "test", nEquihashN, nEquihashK, "default");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x06c9d468d834f7b9d67dcffe01f93a55ee9654cc62ae6bb284f22696eb8ed4e2"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x6952a3e7c73d5a56fefe3a2ceb40e99b7a92c4b84ff9b8f803734e380f463f92"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        // Btcnano Nano seeder
/*        vSeeds.push_back(CDNSSeedData(
			"btcnano.org", "testnet-seed.btcnano.org", true));*/
        vSeeds.push_back(CDNSSeedData(
            "btcnano1.org", "testnet-seed.btcnano1.org", true));
        vSeeds.push_back(CDNSSeedData(
            "btcnano2.org", "testnet-seed.btcnano2.org", true));
        vSeeds.push_back(CDNSSeedData(
            "btcnano3.org", "testnet-seed.btcnano3.org", true));

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 127);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
        vFixedSeeds = std::vector<SeedSpec6>(
            pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = {
            .mapCheckpoints = {
				{0, uint256S("0x06c9d468d834f7b9d67dcffe01f93a55ee9654cc62ae6bb284f22696eb8ed4e2")}
            }};

        // Data as of block
        // 00000000c2872f8f8a8935c8e3c5862be9038c97d4de2cf37ed496991166928a
        // (height 1063660)
        chainTxData = ChainTxData{1513758241, 0, 0};
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
		consensus.nSubsidySlowStartInterval = 0;
        consensus.nSubsidyHalvingInterval = 150;
        // BIP34 has not activated on regtest (far in the future so block v1 are
        // not rejected in tests)
        consensus.BIP34Height = 100000000;
        consensus.BIP34Hash = uint256();
        // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP65Height = 1351;
        // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251;
        consensus.antiReplayOpReturnSunsetHeight = 530000;
        consensus.antiReplayOpReturnCommitment = GetAntiReplayCommitment();
        consensus.powLimit = uint256S(
            "0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowAveragingWindow = 17;
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up
        // two weeks
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60;
        consensus.nPowTargetSpacing = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        // 75% for testchains
        consensus.nRuleChangeActivationThreshold = 108;
        // Faster than normal for regtest (144 instead of 2016)
        consensus.nMinerConfirmationWindow = 1;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            999999999999ULL;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
            999999999999ULL;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are
        // valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        // Hard fork is always enabled on regtest.
        consensus.uahfHeight = 0;

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        pchCashMessageStart[0] = 0xfd;                                                                                                                                                                                                                                        
        pchCashMessageStart[1] = 0xbb;
        pchCashMessageStart[2] = 0x5b;
        pchCashMessageStart[3] = 0xdf;
        nDefaultPort = 18556;
        nPruneAfterHeight = 1000;

		// Equihash
        const size_t N = 48, K = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

		vuc sol{5,97,82,140,128,226,134,135,128,21,216,210,222,181,239,35,63,239,17,255,206,191,145,234,189,221,107,46,106,86,239,119,173,198,99,221};
        genesis = CreateGenesisBlock(1513758281, uint256S("8"), 0x200f0f0f, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();

#ifdef PRINTS_OUT
		mery(&genesis, "reg", nEquihashN, nEquihashK, "default");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x02bf056f28ea74c06444d9439d4c96f61f414611a92c8963e020cd212a81bd23"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x6952a3e7c73d5a56fefe3a2ceb40e99b7a92c4b84ff9b8f803734e380f463f92"));

        //!< Regtest mode doesn't have any fixed seeds.
        vFixedSeeds.clear();
        //!< Regtest mode doesn't have any DNS seeds.
        vSeeds.clear();

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {.mapCheckpoints = {
                              {0, uint256S("0x02bf056f28ea74c06444d9439d4c96f61f414611a92c8963e020cd212a81bd23")},
                          }};

        chainTxData = ChainTxData{0, 0, 0};

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<uint8_t>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<uint8_t>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<uint8_t>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};
    }

    void UpdateBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime,
                              int64_t nTimeout) {
        consensus.vDeployments[d].nStartTime = nStartTime;
        consensus.vDeployments[d].nTimeout = nTimeout;
    }
};

static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(const std::string &chain) {
    if (chain == CBaseChainParams::MAIN) {
        return mainParams;
    }

    if (chain == CBaseChainParams::TESTNET) {
        return testNetParams;
    }

    if (chain == CBaseChainParams::REGTEST) {
        return regTestParams;
    }

    throw std::runtime_error(
        strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string &network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);
}

void UpdateRegtestBIP9Parameters(Consensus::DeploymentPos d, int64_t nStartTime,
                                 int64_t nTimeout) {
    regTestParams.UpdateBIP9Parameters(d, nStartTime, nTimeout);
}
