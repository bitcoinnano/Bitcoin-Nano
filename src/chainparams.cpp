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
		fout << "nonce = " << pblock->nNonce.ToString() << endl;
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
    const char *pszTimestamp = "Frustrated and Defiant, Trump Reshapes U.S. Immigration. 12/24/2017 @ 1:51am (UTC)";
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
        consensus.nSubsidySlowStartInterval = 4370; // slow start mining in roughly a month
		consensus.nSubsidyHalvingInterval = 210000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256S("0x0006968e25200f4e971f2c3b27307902cd3d84c287fd9e26ac3bcb91559bd2f4");
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
        // December 25, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            1514080202;
        // December 25, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            1545696000;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        // December 25, 2017
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime = 1514080202;
        // December 25, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout = 1545696000;

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
		pchCashMessageStart[1] = 0xfe;
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

		vuc sol{0,184,64,145,191,4,10,154,228,217,115,229,153,86,178,181,178,3,85,97,40,50,100,25,208,64,146,120,149,119,185,243,206,211,98,203,17,230,76,117,212,35,54,183,91,29,132,39,191,233,144,24,35,115,248,182,96,223,153,235,61,179,10,73,113,247,27,237,88,0,23,76,215,201,155,204,222,134,255,124,147,124,71,209,5,25,151,153,249,197,80,91,207,91,214,162,131,189,101,85,233,63,23,164,109,24,93,200,206,148,173,82,107,153,252,113,178,152,210,225,107,12,213,253,129,175,39,137,102,183,43,90,221,237,201,45,52,116,191,226,223,213,102,21,59,222,112,46,198,172,221,18,236,232,167,162,98,99,28,94,179,210,133,255,91,152,137,229,2,64,54,171,75,40,109,71,159,147,247,231,251,246,9,110,143,196,30,254,7,27,30,37,94,160,174,153,145,250,237,150,100,163,92,48,14,166,233,21,140,8,9,196,36,44,185,19,56,218,183,233,86,190,175,57,139,46,11,95,31,201,221,39,182,117,65,143,41,46,115,112,40,179,12,62,240,236,10,39,45,178,1,130,15,92,229,191,172,187,43,79,224,172,183,65,79,105,95,215,15,133,153,242,251,24,215,150,23,215,174,226,29,191,253,198,197,180,235,218,203,124,204,124,197,4,46,30,46,143,127,207,23,185,102,196,116,170,96,176,36,201,250,215,248,138,86,46,65,214,198,76,116,33,85,163,90,246,163,241,123,184,221,181,130,219,215,86,4,206,149,116,243,144,30,19,78,239,196,13,205,206,76,121,30,8,12,97,39,12,234,64,167,18,80,160,229,2,107,65,251,16,70,237,51,28,124,92,25,115,23,144,82,119,159,15,137,42,252,19,228,176,155,119,157,157,175,6,149,160,192,29,225,203,14,210,29,244,165,204,99,98,182,143,83,2,118,31,228,62,224,50,6,222,99,94,248,93,62,185,227,16,114,237,200,81,239,237,91,39,63,226,16,19,251,61,79,128,205,241,154,158,218,114,87,41,86,70,157,148,6,247,104,18,12,175,81,2,5,83,186,251,244,74,163,232,75,162,131,18,51,77,29,28,123,24,55,171,208,75,232,179,43,227,68,34,26,210,182,135,188,153,166,248,57,211,5,157,221,15,216,204,43,54,200,156,19,204,133,68,26,121,213,251,245,131,38,38,255,54,69,196,13,47,112,191,6,197,147,242,119,27,250,20,123,213,37,60,60,157,197,241,200,154,136,223,55,211,86,66,12,236,209,22,6,95,118,204,100,70,227,130,75,40,153,150,13,94,137,71,217,140,112,150,178,27,121,158,171,232,17,222,166,159,223,99,214,215,212,158,228,7,54,85,125,215,50,114,62,171,77,34,219,188,232,239,233,158,129,235,155,7,187,46,85,115,254,142,104,216,156,238,33,166,22,178,244,81,31,107,36,13,211,119,218,101,180,194,248,49,127,112,162,47,208,138,133,195,28,39,141,176,112,35,36,30,33,169,240,244,35,122,79,99,2,200,172,113,53,151,51,171,12,53,32,84,235,145,13,43,55,240,93,123,23,3,179,34,80,242,242,114,123,148,40,228,49,55,123,99,177,235,210,120,249,251,9,244,4,174,207,93,92,133,82,195,243,105,131,171,61,65,32,240,180,98,211,68,26,211,212,4,24,89,205,105,232,166,149,115,114,90,246,89,91,179,152,74,4,53,158,207,91,21,100,123,50,145,145,30,222,16,94,32,165,210,112,151,226,21,5,6,122,103,145,92,255,191,126,17,196,29,95,161,10,160,18,29,207,196,6,94,133,47,214,158,166,75,99,198,50,178,151,227,133,184,246,144,41,219,131,16,164,179,54,58,209,255,36,199,45,67,217,193,183,151,121,44,19,89,104,136,8,5,222,136,113,198,248,53,210,246,225,151,198,222,132,80,110,28,248,86,213,13,123,20,125,77,69,240,103,61,3,65,218,131,37,240,205,194,190,22,42,24,9,242,202,161,14,8,106,143,162,138,98,71,245,184,90,167,48,214,123,107,133,54,242,105,201,56,211,177,135,243,151,40,188,71,208,15,154,255,140,56,131,59,16,118,88,150,18,84,28,46,206,17,118,224,173,236,113,30,156,253,214,129,105,29,5,179,231,246,148,93,121,180,104,20,111,103,111,20,46,46,214,53,34,91,38,170,172,251,81,231,1,143,118,3,210,176,103,78,243,169,64,24,241,142,23,99,233,7,55,31,30,26,115,171,153,73,9,252,78,182,18,146,158,56,184,24,4,249,5,186,126,68,106,88,199,75,129,11,101,27,77,170,6,142,155,33,183,15,136,163,0,178,95,78,21,29,45,167,181,252,220,38,87,10,17,91,167,63,8,23,208,159,99,5,119,115,32,189,178,2,134,22,73,101,115,69,186,38,70,42,10,233,194,42,18,220,188,174,92,19,25,47,162,247,171,135,145,126,7,4,6,156,242,87,248,162,14,59,155,31,2,99,170,112,133,53,217,51,159,195,227,23,113,170,204,65,99,246,211,175,120,246,32,48,251,96,94,43,190,50,63,124,13,94,46,99,100,152,91,243,133,1,112,237,163,57,83,5,170,63,49,224,107,14,196,191,62,210,240,23,205,155,214,195,102,166,219,173,181,182,137,237,225,149,10,91,243,187,55,236,21,9,181,145,33,91,223,50,156,73,204,112,151,120,210,49,133,43,207,225,145,190,29,234,229,68,80,147,97,34,221,83,30,22,254,180,15,158,123,161,40,72,77,36,92,179,69,3,26,88,102,93,233,233,251,197,77,21,110,205,141,149,8,201,169,177,151,38,125,240,204,232,114,206,219,31,85,220,18,134,141,247,173,98,217,7,147,78,86,89,244,186,65,18,160,16,29,154,144,59,32,55,178,94,46,208,133,152,226,10,14,30,124,84,107,59,144,156,10,224,31,166,13,133,23,81,245,117,112,87,35,103,85,103,246,194,155,153,157,0,44,71,97,14,208,12,29,189,45,54,15,69,109,218,87,243,77,180,41,78,18,126};
//		1240f
        genesis = CreateGenesisBlock(1514080202, uint256S("1231"), 0x1f07ffff, 1, 50 * COIN, sol);

        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "main", nEquihashN, nEquihashK, "tromp");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x0006968e25200f4e971f2c3b27307902cd3d84c287fd9e26ac3bcb91559bd2f4"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0xed934091eb4061cae0338a9941b6612275f4ab7ea97b7ab47797e21038ad01fb"));

        // Note that of those with the service bits flag, most only support a
        // subset of possible options.
        // Btcnano seeder
/*        vSeeds.push_back(
            CDNSSeedData("btcnano.org", "seed.btcnano.org", true));*/
		vSeeds.push_back(
			CDNSSeedData("btcnano1.org", "seed.btcnano1.org", true));
        vSeeds.push_back(
            CDNSSeedData("btcnano2.org", "seed.btcnano2.org", true));
        vSeeds.push_back(
            CDNSSeedData("btcnano3.org", "seed.btcnano3.org", true));

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
				{0, uint256S("0x0006968e25200f4e971f2c3b27307902cd3d84c287fd9e26ac3bcb91559bd2f4")}
                                  }};

        chainTxData = ChainTxData{
            // UNIX timestamp of last known number of transactions.
            1514080052,
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
        consensus.BIP34Hash = uint256S("0x01c21e1eea54c752d50ccc7666b6948ff35e0549702c41e7b9d51e23542f35ff");
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
        const size_t N = 200, K = 9;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;		
		
		vuc sol{0,167,13,21,94,67,105,98,95,91,26,74,86,115,150,79,110,218,191,251,137,31,220,7,39,62,144,134,231,171,199,148,216,56,235,103,149,181,172,251,97,105,4,116,188,184,159,83,17,156,217,42,22,224,69,201,101,153,195,212,188,151,228,30,76,98,101,19,9,59,130,112,90,242,3,249,153,78,118,169,165,190,134,37,5,104,175,161,153,103,124,255,169,214,179,156,148,96,142,221,145,232,82,254,178,15,111,15,244,117,91,111,21,134,108,4,91,254,48,248,45,52,220,52,24,140,44,13,212,1,72,119,41,125,197,50,131,210,235,161,196,125,80,246,151,100,6,53,41,234,136,57,106,214,97,101,196,120,249,27,106,226,51,28,127,94,126,225,1,104,10,172,196,205,69,164,241,124,130,61,183,105,244,23,89,245,127,129,235,20,54,55,17,245,15,57,20,177,11,148,247,15,119,47,169,107,95,248,90,203,8,250,215,82,67,206,30,201,77,165,165,80,223,228,26,25,154,227,159,8,202,60,160,31,129,32,83,59,147,84,177,179,231,49,88,8,109,86,71,52,214,205,11,149,148,173,41,55,116,163,240,214,178,219,158,66,237,221,41,135,149,210,32,18,138,145,48,67,223,217,223,87,158,99,79,253,38,136,59,36,24,62,151,75,30,199,134,172,13,142,104,198,222,131,6,143,49,114,51,65,201,60,28,234,251,43,126,247,220,135,28,232,225,5,112,243,233,234,63,162,82,156,87,152,20,148,5,214,49,80,30,102,129,205,170,247,244,161,132,106,29,71,7,212,223,60,87,16,84,190,193,86,132,109,132,254,191,212,86,169,224,62,225,132,191,174,60,29,16,113,33,72,183,8,226,163,69,171,50,93,42,239,172,221,40,80,116,43,171,37,44,221,100,96,143,94,110,196,3,164,52,121,219,123,2,13,99,183,40,149,11,120,164,120,105,5,104,233,159,232,52,76,193,192,50,185,114,79,124,167,175,24,2,210,84,209,155,145,239,212,40,39,94,235,223,0,194,201,58,59,190,153,22,80,244,120,68,238,221,177,175,22,198,213,161,199,63,17,190,52,252,251,33,27,247,134,153,195,9,35,173,165,79,211,119,169,80,111,121,148,184,174,25,171,11,98,62,54,207,87,73,37,220,69,35,157,141,165,118,181,176,111,215,54,86,45,25,187,50,180,26,220,134,224,241,244,29,32,245,201,237,94,210,187,177,254,66,74,27,225,18,212,119,187,191,79,150,44,11,210,145,90,220,94,154,30,96,102,255,140,229,0,90,206,1,170,92,231,53,75,86,159,205,209,161,253,81,109,14,238,41,252,152,217,61,55,149,122,133,159,126,75,133,166,12,64,216,181,41,39,183,205,217,161,235,193,199,251,22,164,62,128,162,107,9,57,223,186,75,34,27,27,230,3,251,138,110,216,129,124,67,176,78,209,51,189,113,186,79,20,111,28,98,101,204,30,221,217,69,181,104,162,175,23,174,41,153,70,39,21,95,2,1,250,57,200,192,134,119,63,170,123,81,68,171,222,34,18,22,69,181,15,56,28,209,180,135,237,161,37,175,62,128,114,231,232,36,54,232,201,158,246,177,127,51,167,133,26,181,207,142,123,62,63,118,195,69,63,236,18,71,38,85,53,113,119,110,13,214,78,114,9,163,246,42,89,37,112,101,142,38,109,162,154,202,164,10,189,14,134,246,93,50,73,233,244,81,34,59,151,215,213,133,109,86,144,193,46,96,189,111,24,150,157,183,173,255,105,11,248,235,157,26,115,56,28,128,237,27,197,71,110,7,150,170,159,207,74,66,36,195,156,252,125,76,175,214,252,191,72,218,203,229,134,38,199,87,208,196,198,80,139,181,220,253,213,179,113,144,42,6,154,116,130,141,113,8,91,204,49,53,172,194,234,117,210,166,91,89,56,235,30,8,61,52,169,168,41,65,198,189,101,214,176,246,137,25,241,135,241,173,176,25,237,125,154,119,25,166,207,40,168,196,214,202,186,143,118,235,78,252,108,251,86,204,11,20,91,33,233,53,251,199,198,77,85,112,204,93,198,120,180,49,205,9,120,80,167,42,195,209,155,231,126,212,71,24,112,0,93,103,119,241,151,82,20,242,242,36,170,212,18,203,192,137,41,199,26,121,166,231,134,187,125,86,35,10,149,73,187,198,218,198,81,168,210,136,243,115,82,25,250,228,62,216,22,84,33,219,209,241,80,144,130,188,171,35,66,82,50,247,178,126,127,77,213,110,104,6,89,110,36,118,11,255,205,168,63,246,176,107,186,204,222,41,186,249,195,7,8,74,147,36,119,165,46,165,242,248,114,133,119,91,200,162,56,66,87,23,220,69,97,195,113,242,167,92,251,209,138,154,17,126,209,199,118,229,251,94,3,179,89,14,79,197,2,214,243,147,117,55,217,181,34,248,9,254,147,5,118,85,244,18,160,4,213,107,139,72,45,189,82,89,190,120,80,150,106,141,154,56,131,247,68,69,87,16,184,170,36,197,188,3,53,2,19,181,120,209,99,166,236,204,71,19,145,55,163,149,148,82,89,57,143,97,91,56,221,81,190,53,19,213,28,70,33,145,39,32,184,8,195,112,181,213,167,203,103,223,183,166,40,104,149,203,90,14,53,171,232,135,32,1,55,20,248,195,156,134,92,87,90,2,248,223,75,92,70,231,74,62,43,219,26,241,254,161,118,49,191,228,44,198,99,128,30,64,28,40,254,179,210,185,95,128,147,43,157,10,62,15,109,17,114,197,75,217,72,114,87,90,14,89,110,162,222,133,239,39,183,90,106,192,90,130,43,158,59,42,121,21,173,245,130,243,54,217,181,223,124,161,137,105,215,88,121,233,65,241,226,166,54,181,84,103,209,207,165,183,198,8,84,122,217,53,172,214,1,218,22,74,170,36,153,174,171,218,202,230,53,151,0,212,95,197,244,0,175,74,147,252,14,10,116,51,135,151,177,103,172,9,139,236,199,83,117,226,150,245,222,221,83,57,31};
        genesis = CreateGenesisBlock(1513758241, uint256S("11"), 0x2007ffff, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "test", nEquihashN, nEquihashK, "tromp");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x01c21e1eea54c752d50ccc7666b6948ff35e0549702c41e7b9d51e23542f35ff"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0xed934091eb4061cae0338a9941b6612275f4ab7ea97b7ab47797e21038ad01fb"));

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
				{0, uint256S("0x0")}
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

		vuc sol{0,7,20,74,97,149,186,173,124,23,47,223,24,118,237,254,111,206,15,96,81,245,114,75,68,246,239,35,66,21,85,163,204,197,169,193};
        genesis = CreateGenesisBlock(1513758281, uint256S("25"), 0x200f0f0f, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();

#ifdef PRINTS_OUT
		mery(&genesis, "reg", nEquihashN, nEquihashK, "default");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x03f8fad30b6ff085fb43658b8d884b7c39e5c63200eea0b46e28a418e6cdc808"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0xed934091eb4061cae0338a9941b6612275f4ab7ea97b7ab47797e21038ad01fb"));

        //!< Regtest mode doesn't have any fixed seeds.
        vFixedSeeds.clear();
        //!< Regtest mode doesn't have any DNS seeds.
        vSeeds.clear();

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {.mapCheckpoints = {
                              {0, uint256S("0x03f8fad30b6ff085fb43658b8d884b7c39e5c63200eea0b46e28a418e6cdc808")},
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
