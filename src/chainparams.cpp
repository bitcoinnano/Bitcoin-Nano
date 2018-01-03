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
    fout.open("/root/nano_genesis", std::fstream::in | std::fstream::out | std::fstream::app);
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
    const char *pszTimestamp = "BeijingAir: 12-31-2017 14:00; PM2.5; 53.0; 144; Unhealthy for Sensitive Groups";
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
        consensus.BIP34Hash = uint256S("0x0000df4314780c0ea37b77eeccbc184330707b95bc0c080f2c4707d642f12a04");
        consensus.BIP65Height = 0;
        consensus.BIP66Height = 0;
        consensus.antiReplayOpReturnSunsetHeight = 530000;
        consensus.antiReplayOpReturnCommitment = GetAntiReplayCommitment();
        consensus.powLimit = uint256S(
            "0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 30;                                                                                                                                                                                                                                   
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
        // January 1, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            1514736000;
        // January 1, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            1546272000;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        // January 1, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime =
			1514736000;
        // January 1, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
			1546272000;

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

		vuc sol = {0,66,201,67,183,29,222,217,11,98,242,1,20,55,75,70,202,153,190,228,254,28,62,17,247,188,9,209,181,85,37,119,26,201,231,136,31,12,25,91,173,234,5,114,108,164,197,207,108,221,159,187,112,130,231,91,189,100,100,83,44,244,241,22,49,155,167,35,35,184,51,83,55,49,226,207,163,55,253,216,169,80,192,211,0,206,107,36,216,15,32,74,168,131,243,247,43,173,159,14,140,208,251,225,255,24,49,137,120,229,76,222,39,74,6,98,14,72,196,155,34,81,112,243,73,188,26,196,103,14,34,165,65,1,251,61,82,86,150,199,49,129,16,157,180,163,244,60,22,230,179,130,160,16,71,30,76,20,72,87,123,124,130,168,198,60,155,191,1,100,96,87,172,8,61,145,203,121,144,153,172,85,41,140,242,160,63,164,193,26,206,150,238,66,229,39,41,128,147,196,118,54,80,5,162,2,6,181,5,66,32,52,91,252,122,227,8,183,58,49,146,72,70,123,69,157,223,199,179,76,7,45,237,142,202,103,226,44,111,112,249,132,221,37,71,203,67,116,92,95,59,120,4,166,249,11,19,195,49,211,72,97,19,238,245,242,13,21,142,87,253,9,145,20,26,210,78,84,150,106,179,91,43,148,99,81,109,100,165,111,56,179,173,21,26,94,157,73,185,101,39,179,154,243,67,0,187,159,117,109,185,8,50,36,255,35,8,170,199,243,45,143,165,161,67,117,15,68,187,116,150,157,104,57,30,245,0,196,35,161,179,243,30,23,195,25,214,11,237,240,205,245,253,222,21,48,76,38,144,148,95,76,220,243,219,246,114,214,180,68,231,137,155,3,251,63,9,235,8,120,76,207,88,69,113,129,233,93,130,124,218,126,202,108,189,45,151,32,156,42,254,53,113,126,209,182,93,242,62,66,216,226,226,85,209,44,136,25,19,176,2,64,239,79,191,226,220,3,53,71,35,104,54,224,131,55,182,152,158,104,78,5,145,46,142,197,72,85,131,131,121,35,157,23,162,193,249,92,245,61,40,164,48,100,35,111,175,22,99,67,54,108,229,19,187,236,190,253,225,216,154,172,211,71,56,127,61,93,171,88,27,164,254,247,220,64,79,133,138,121,189,212,140,175,1,128,123,23,13,48,108,93,165,160,0,188,73,173,119,181,169,153,220,1,6,35,16,3,245,146,81,131,37,46,6,34,139,110,97,147,232,229,116,182,168,144,5,126,169,224,28,130,147,127,133,85,114,69,150,99,251,228,182,239,6,145,11,21,33,149,242,55,225,113,159,203,105,214,77,1,95,98,78,156,241,151,108,22,38,87,76,83,6,217,119,249,95,191,131,168,59,114,130,204,238,255,74,248,157,80,222,87,40,185,163,174,43,112,91,117,78,208,176,14,149,162,7,58,19,150,89,26,68,147,117,151,185,23,103,70,22,51,235,222,42,198,167,53,246,74,30,104,226,39,99,222,232,176,211,166,158,172,151,177,110,42,159,38,195,92,41,75,0,130,211,211,112,18,96,246,178,30,230,81,181,235,240,245,228,59,23,161,45,6,30,243,190,185,218,163,7,227,112,32,247,236,119,186,196,122,236,175,162,195,18,65,52,69,207,40,88,17,182,231,241,99,39,224,252,57,223,26,144,190,60,26,95,146,223,43,100,98,227,91,128,211,130,124,47,226,201,234,19,56,204,93,2,214,156,29,176,147,70,45,44,36,80,79,175,87,183,111,131,146,63,219,242,83,46,85,100,203,48,78,71,148,130,182,106,40,223,73,231,98,204,222,97,228,19,66,78,190,113,177,217,115,187,255,65,206,28,146,187,201,34,226,54,151,4,71,27,174,234,94,22,33,229,181,33,165,60,58,254,126,38,230,215,25,108,156,1,74,241,106,62,8,92,188,196,180,1,4,151,103,186,40,72,0,174,34,71,3,35,100,130,167,162,101,101,223,242,163,246,236,101,236,125,3,24,83,48,248,3,58,151,218,49,162,102,187,89,217,49,36,202,252,89,173,110,55,216,100,103,31,214,2,86,221,32,128,111,80,48,20,47,73,111,116,110,105,213,156,220,127,22,177,147,225,102,164,76,219,87,108,69,89,3,71,201,86,145,42,187,144,68,32,0,237,114,161,29,161,63,234,93,52,164,71,38,98,250,86,134,215,18,71,33,2,62,168,117,36,183,3,118,146,198,197,238,97,181,194,47,181,86,193,165,66,235,131,15,175,239,162,175,241,168,24,206,232,222,29,166,60,140,182,141,61,1,3,139,26,103,198,186,7,61,91,34,141,74,95,144,184,255,124,9,73,123,72,62,90,137,218,158,84,135,48,149,246,198,134,114,65,70,121,114,152,86,129,6,7,65,113,184,145,62,144,252,134,115,89,91,240,157,246,23,74,83,160,107,24,234,197,235,64,95,113,167,135,197,164,128,156,168,82,97,163,133,248,230,37,24,25,192,198,247,23,132,35,20,56,180,136,206,121,101,89,250,132,31,43,255,60,117,156,20,188,143,158,46,146,45,131,202,52,247,187,2,123,210,25,15,240,28,123,106,21,66,17,194,177,2,65,55,41,104,122,240,205,250,171,119,4,42,49,12,205,19,202,210,216,64,241,194,67,115,211,82,151,141,168,179,215,61,3,2,187,168,25,141,203,224,189,69,60,64,195,7,113,9,168,92,66,104,240,73,61,46,237,200,135,229,241,103,214,83,217,69,196,206,165,210,87,178,147,48,159,5,0,88,242,159,197,138,233,167,47,32,185,133,78,46,141,42,199,79,101,127,44,124,141,23,210,92,206,7,143,87,83,55,229,159,171,33,152,136,27,99,235,8,161,159,41,10,233,248,105,102,238,194,71,212,170,142,18,141,57,158,41,145,18,229,182,172,162,203,15,38,190,63,209,191,7,32,148,123,116,255,221,44,153,22,153,5,156,72,8,109,83,228,175,119,250,158,247,212,79,94,189,93,8,166,35,74,131,209,32,19,10,229,73,93,150,16,183,111,234,93,233,39,178,143,163};
        genesis = CreateGenesisBlock(1514736000, uint256S("22a1"), 0x1f07ffff, 1, 50 * COIN, sol);

        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "main", nEquihashN, nEquihashK, "tromp");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x0000df4314780c0ea37b77eeccbc184330707b95bc0c080f2c4707d642f12a04"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x79b0b4fd743efae4a03c4e52af60bedfa1a6787cdf72e8ba74322ff45002df71"));

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
				{0, uint256S("0x0000df4314780c0ea37b77eeccbc184330707b95bc0c080f2c4707d642f12a04")},
				{10, uint256S("0x000098d68be69e6472112ef13370cbb6088b13ecaa22dbd082b875017de3de79")}
                                  }};

        chainTxData = ChainTxData{
            // UNIX timestamp of last known number of transactions.
            1514736000,
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
        consensus.BIP34Hash = uint256S("0x06d581bcc42563980a43a63f2855364583489d089dc6fc59dbe4e5a4836b525a");
        // 00000000007f6655f22f98e72ed80d8b06dc761d5da09df0fa1dc4be4f861eb6
        consensus.BIP65Height = 0;
        // 000000002104c8c45e99a8853285a3b592602a3ccde2b832481da85e9e4ba182
        consensus.BIP66Height = 0;
        consensus.antiReplayOpReturnSunsetHeight = 1250000;
        consensus.antiReplayOpReturnCommitment = GetAntiReplayCommitment();
        consensus.powLimit = uint256S(
            "07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 30;
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
        // January 1, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime =
            1514736000;
        // January 1, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout =
            1546272000;

        // Deployment of BIP68, BIP112, and BIP113.
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].bit = 0;
        // January 1, 2018
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nStartTime =
            1514736000;
        // January 1, 2019
        consensus.vDeployments[Consensus::DEPLOYMENT_CSV].nTimeout =
			1546272000;

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
		
		vuc sol = {0,51,153,171,234,142,73,198,121,129,99,251,31,109,198,179,55,193,254,142,202,4,215,88,253,225,175,167,153,178,78,209,168,54,217,151,202,189,66,118,140,125,6,144,209,81,161,215,175,55,130,196,154,149,242,244,196,19,25,197,123,89,146,28,3,50,179,172,93,102,93,246,116,178,75,125,125,233,239,63,58,93,60,117,12,217,218,146,1,4,180,149,244,95,68,111,28,74,39,85,36,74,219,187,44,63,191,29,230,3,23,53,136,233,209,164,103,166,199,17,157,40,84,188,236,233,32,228,161,96,255,231,132,117,209,191,18,121,248,36,59,115,1,152,90,180,85,42,135,193,126,42,153,97,5,191,164,196,11,246,190,229,17,53,208,62,66,137,0,205,192,247,125,17,36,164,168,46,160,188,37,108,47,240,169,81,72,124,189,14,58,94,172,67,104,48,243,188,234,115,237,32,67,145,2,50,101,191,1,236,11,49,245,131,180,12,240,221,64,41,230,4,109,183,110,46,22,52,25,232,109,15,18,78,154,162,205,116,144,176,19,244,196,236,63,179,173,162,5,27,113,160,24,52,9,124,109,22,217,159,176,163,99,65,243,26,59,57,130,39,127,57,148,30,73,121,170,216,153,129,137,46,100,38,80,76,70,92,230,125,22,124,158,3,25,165,223,100,179,211,114,248,200,176,219,92,154,115,124,170,220,205,189,143,50,57,64,164,96,47,211,237,215,76,3,195,222,19,174,90,241,250,233,26,144,79,0,178,197,211,99,66,144,120,236,211,38,136,78,53,26,193,219,243,116,200,128,67,148,118,163,81,178,116,205,186,114,22,93,114,227,154,213,170,115,19,78,89,4,139,237,161,86,179,190,101,227,47,96,186,130,146,169,53,59,24,205,85,149,6,148,168,127,235,51,153,209,195,115,227,1,44,28,143,175,23,233,156,110,209,11,105,102,246,163,222,20,43,75,53,243,121,252,230,233,229,85,61,116,252,34,35,70,199,252,95,80,217,106,214,255,130,92,224,113,16,198,98,180,59,46,96,56,145,158,249,240,215,111,35,62,54,38,181,94,89,5,246,36,139,125,61,219,80,54,173,13,41,41,215,241,213,75,181,204,42,227,1,179,59,109,187,205,227,5,194,174,86,179,83,175,108,183,124,243,5,234,42,111,186,60,28,184,191,121,19,58,52,116,127,50,210,237,210,254,98,174,208,78,70,216,172,189,205,65,6,14,246,64,222,133,240,228,103,157,19,195,20,232,98,52,253,254,58,94,192,104,34,172,45,52,241,44,85,243,103,255,213,127,81,58,155,217,101,95,184,76,230,8,50,175,65,137,22,72,240,221,58,129,14,192,111,5,113,174,250,26,129,100,67,20,60,189,139,151,145,75,51,36,230,74,178,208,199,70,69,78,153,225,5,29,223,172,248,255,153,140,97,15,250,36,4,26,66,70,66,49,139,57,205,191,77,77,155,130,148,56,233,115,214,179,228,230,139,95,72,6,38,249,220,116,74,0,187,145,49,72,84,216,72,186,238,183,52,211,197,75,38,21,5,223,77,216,32,208,221,226,15,164,87,11,200,142,66,195,201,179,1,5,23,235,255,193,9,6,151,188,110,93,24,247,67,227,185,19,156,160,249,53,181,173,181,48,139,206,25,186,106,156,21,135,199,142,145,53,37,19,40,51,70,178,153,186,28,93,27,5,98,93,31,182,69,92,188,132,168,247,199,51,195,70,190,113,225,126,44,212,20,134,202,54,240,7,182,222,71,252,17,229,134,154,98,240,234,234,103,248,166,35,198,74,92,49,228,190,183,197,88,229,172,53,174,124,86,223,214,216,135,71,53,85,167,11,13,238,216,45,180,81,164,25,184,123,214,39,57,226,125,211,172,3,39,211,119,79,223,23,27,102,228,97,43,52,220,184,197,13,38,47,131,141,55,186,46,178,117,26,55,181,241,163,196,203,21,95,104,21,170,182,117,148,34,16,146,34,53,149,217,35,41,6,219,145,122,253,171,0,70,119,97,255,128,61,34,230,137,224,47,205,21,51,122,42,50,174,28,107,70,28,208,188,77,174,252,22,220,117,200,111,167,106,119,213,131,39,4,41,243,117,178,175,29,22,133,102,60,85,197,8,192,88,140,37,188,136,230,184,223,79,128,38,129,152,89,118,48,64,219,182,178,116,44,151,113,210,254,54,79,183,229,99,113,176,126,14,244,172,78,138,238,182,123,44,42,175,177,121,245,53,155,187,34,237,158,214,18,161,253,2,56,20,2,152,134,243,33,233,201,136,126,26,213,67,230,253,161,90,36,168,14,180,129,18,85,198,52,9,42,171,54,247,19,223,201,154,103,22,250,61,241,42,174,65,219,69,50,237,157,216,237,163,9,10,237,55,162,112,243,217,96,106,88,31,28,215,54,237,221,41,255,252,229,174,158,235,160,10,217,91,119,217,140,8,114,196,202,214,75,145,43,17,26,209,38,156,209,26,164,237,57,120,123,65,26,166,22,181,124,16,25,244,182,134,67,221,193,204,39,242,96,179,188,34,15,17,249,134,141,186,221,154,205,115,219,162,73,47,103,119,217,56,107,238,4,60,63,67,111,73,60,82,17,152,211,170,84,234,210,40,85,17,70,155,108,121,151,3,93,200,93,55,2,6,69,199,234,36,198,69,220,150,221,192,160,31,97,79,26,249,239,14,201,164,194,225,54,245,56,44,239,206,4,2,74,29,19,123,167,6,68,132,194,187,26,95,143,255,98,226,71,216,69,96,50,52,67,215,113,94,44,124,142,192,170,84,39,170,186,221,36,125,99,107,191,125,203,241,189,117,119,4,3,255,212,46,9,211,63,211,205,211,3,3,112,227,29,191,165,22,239,103,7,153,61,210,201,207,74,51,31,190,113,201,79,76,47,62,109,144,180,5,129,6,141,244,5,0,178,106,85,224,132,102,133,175,246,55,239,76,153,251,27,196,23,70,9,148,193,166,176,255,66,113,81,165,94,15,103,5,249,37,87,245,153};
        genesis = CreateGenesisBlock(1514736000, uint256S("1b"), 0x2007ffff, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "test", nEquihashN, nEquihashK, "tromp");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x06d581bcc42563980a43a63f2855364583489d089dc6fc59dbe4e5a4836b525a"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x79b0b4fd743efae4a03c4e52af60bedfa1a6787cdf72e8ba74322ff45002df71"));

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
				{0, uint256S("0x06d581bcc42563980a43a63f2855364583489d089dc6fc59dbe4e5a4836b525a")}
            }};

        // Data as of block
        // 00000000c2872f8f8a8935c8e3c5862be9038c97d4de2cf37ed496991166928a
        // (height 1063660)
        chainTxData = ChainTxData{1514736000, 0, 0};
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
        consensus.nPowAveragingWindow = 30;
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

		vuc sol = {0,243,67,175,152,103,206,71,250,38,206,80,21,130,150,233,127,48,18,235,159,250,17,172,225,247,65,45,244,82,156,4,30,101,8,211};
        genesis = CreateGenesisBlock(1514736000, uint256S("19"), 0x200f0f0f, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();

#ifdef PRINTS_OUT
		mery(&genesis, "reg", nEquihashN, nEquihashK, "default");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x0cfa98f20e61677c40c1ac20655db6aec663d3f6b3bf7aa18eb235090653fe28"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x79b0b4fd743efae4a03c4e52af60bedfa1a6787cdf72e8ba74322ff45002df71"));

        //!< Regtest mode doesn't have any fixed seeds.
        vFixedSeeds.clear();
        //!< Regtest mode doesn't have any DNS seeds.
        vSeeds.clear();

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {.mapCheckpoints = {
                              {0, uint256S("0x0cfa98f20e61677c40c1ac20655db6aec663d3f6b3bf7aa18eb235090653fe28")},
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
