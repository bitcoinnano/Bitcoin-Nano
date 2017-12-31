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
#define PRINTS_OUT

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
        consensus.BIP34Hash = uint256S("0x0001b88361f3326fac4266d24092a428a84e874fe8fad936cbd7f5c8166d9123");
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

		vuc sol = {1,9,128,99,24,179,225,161,215,75,6,198,125,90,212,17,186,193,109,220,223,9,163,246,163,75,26,51,113,14,53,99,51,247,224,194,145,10,121,179,127,3,11,46,142,102,42,204,32,103,94,11,211,15,164,165,155,129,172,233,56,42,227,13,126,159,104,59,151,168,73,198,107,131,202,239,34,101,185,73,29,14,37,9,10,7,169,3,161,80,168,166,211,227,52,231,95,205,148,225,190,189,56,39,27,41,52,38,130,216,124,234,37,236,153,100,214,230,80,76,70,132,47,87,93,52,71,60,143,196,13,149,127,181,3,21,215,53,1,62,82,106,217,16,58,77,14,156,127,39,225,234,167,225,49,145,206,73,202,132,245,244,70,248,188,255,132,77,1,71,143,253,89,133,238,40,161,161,1,80,80,227,116,60,194,104,167,12,150,60,146,28,254,53,45,212,125,174,172,36,225,147,181,47,118,255,151,95,54,85,18,139,15,238,134,198,86,86,153,16,114,52,235,86,196,213,187,60,178,135,222,19,16,167,206,101,135,161,105,123,79,69,27,35,47,13,21,81,16,15,210,90,14,18,149,241,239,109,110,179,174,158,82,11,177,98,146,85,7,102,219,251,171,62,129,46,61,76,209,230,193,236,220,166,16,98,86,141,147,138,195,157,225,63,19,190,115,139,86,237,39,157,220,126,180,207,158,233,74,33,242,199,243,48,240,31,61,194,7,7,160,185,51,250,218,186,71,65,215,226,74,225,129,255,0,47,1,97,98,221,91,7,168,219,5,202,66,89,126,34,110,25,147,171,152,105,189,26,25,231,172,12,98,185,213,196,26,182,3,191,87,107,138,14,139,115,152,180,38,97,201,71,181,210,220,192,234,225,54,15,53,65,116,201,216,234,79,83,228,97,0,75,182,69,31,19,251,122,158,119,196,99,215,240,42,113,165,63,157,132,20,85,226,72,67,101,210,203,61,235,132,32,14,69,13,86,24,102,221,210,113,34,81,150,12,208,147,201,66,214,170,37,138,35,104,193,170,172,138,215,92,122,22,26,162,101,184,198,179,78,82,101,179,205,122,125,17,237,245,75,83,168,192,22,170,150,132,28,60,138,83,234,117,66,67,237,52,129,8,179,74,183,224,166,2,196,41,249,167,81,252,33,30,207,98,96,122,51,195,137,37,47,78,215,171,60,19,214,215,83,149,83,113,38,30,165,43,179,48,176,118,129,20,58,8,189,32,13,125,134,176,214,109,39,70,229,84,123,175,120,195,215,55,149,60,250,59,39,171,55,182,216,206,57,201,172,1,148,182,43,217,223,229,84,22,27,156,2,2,248,211,160,148,137,223,160,138,108,146,96,117,241,86,9,224,60,190,14,134,8,253,40,109,112,20,123,2,171,150,37,242,106,92,126,110,242,225,216,111,103,15,75,129,7,238,81,166,84,151,164,161,193,219,79,48,236,138,48,114,130,185,47,49,87,23,242,29,21,157,197,144,131,47,9,183,122,106,110,35,187,51,174,5,145,193,245,107,186,29,75,242,118,163,53,25,106,250,221,36,86,218,186,119,9,197,163,42,231,196,140,89,160,254,212,119,228,37,162,245,109,91,118,195,139,6,90,118,215,79,16,23,62,205,106,226,62,143,186,38,140,162,165,238,32,134,6,251,116,30,59,150,186,185,249,152,19,57,138,41,186,17,59,114,51,109,38,11,248,155,171,82,133,5,182,46,143,67,7,134,200,218,21,121,198,55,202,23,58,65,99,221,17,231,157,73,200,128,198,91,27,110,57,57,236,218,210,121,67,26,11,47,127,58,118,190,47,230,7,100,91,70,105,182,113,76,26,90,181,3,52,76,226,153,215,92,74,65,134,130,21,95,102,112,7,90,229,92,95,22,81,10,101,194,186,95,214,210,113,170,243,97,42,200,18,234,217,128,67,45,124,213,104,174,22,2,244,246,60,67,237,39,167,113,52,85,196,190,108,119,95,34,123,60,144,214,14,149,82,86,25,162,11,202,15,202,89,114,246,133,27,153,11,75,84,75,91,122,100,214,149,53,93,250,53,151,64,82,67,199,227,114,255,120,110,15,85,174,244,254,9,16,194,116,74,125,213,55,255,29,59,121,24,220,159,162,18,39,145,56,14,69,78,217,16,81,107,11,27,253,136,231,156,223,125,140,244,28,145,219,38,83,148,188,231,10,252,66,217,176,228,134,174,36,152,27,50,17,44,178,204,58,230,210,148,31,112,86,150,251,105,103,86,6,98,61,244,181,10,6,148,236,61,144,195,67,223,23,67,129,63,163,38,173,65,12,173,48,117,219,9,113,195,247,243,136,111,55,95,195,67,161,136,64,141,65,14,61,41,251,94,23,38,43,138,127,135,47,43,171,53,34,120,215,209,70,177,194,214,26,194,193,89,129,93,24,1,249,5,83,229,174,136,188,86,121,219,63,17,220,153,140,9,9,81,236,35,81,23,113,103,240,220,113,143,67,79,115,129,81,180,241,240,214,56,249,214,228,206,166,7,157,52,189,115,180,81,204,31,29,175,11,251,93,96,11,60,20,96,175,136,211,123,199,5,193,205,67,88,161,85,161,64,49,210,149,67,45,133,137,201,87,42,139,244,220,199,197,235,234,0,214,143,167,246,214,99,6,173,203,42,190,22,194,123,88,249,152,254,56,231,103,30,175,12,254,83,111,9,125,181,215,255,79,190,215,126,185,160,169,95,210,55,30,24,170,183,207,239,18,37,3,167,21,144,53,249,196,166,7,49,110,92,63,89,250,152,149,142,192,74,117,93,96,242,169,7,249,84,201,233,183,98,252,220,254,127,56,252,42,93,9,224,90,126,133,83,67,87,30,102,162,168,56,232,176,126,9,20,152,189,11,56,104,49,248,64,167,194,47,86,124,68,138,172,168,42,5,227,194,149,194,163,10,216,170,107,225,67,161,16,246,13,65,67,99,20,14,96,229,19,40,141,94,37,180,220,94,126,15,197,7,40,197,196,28,91,32,245,194,227,213,252,132,230};
        genesis = CreateGenesisBlock(1514736000, uint256S("8000"), 0x1f07ffff, 1, 50 * COIN, sol);

        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "main", nEquihashN, nEquihashK, "tromp");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x0001b88361f3326fac4266d24092a428a84e874fe8fad936cbd7f5c8166d9123"));
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
				{0, uint256S("0x0001b88361f3326fac4266d24092a428a84e874fe8fad936cbd7f5c8166d9123")}
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
        consensus.BIP34Hash = uint256S("0x06b10fdee77b868501c6c06708cb3bf2a7baab1939a2f931882ad7e43bf247ad");
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
		
		vuc sol = {0,230,175,234,30,9,114,1,149,178,54,42,192,237,21,154,58,179,249,7,54,17,8,141,167,72,40,155,153,103,70,195,15,189,222,53,196,238,244,240,46,214,9,171,221,44,45,217,236,62,208,151,34,107,101,241,32,93,186,107,19,142,167,24,70,94,59,251,42,70,65,85,216,149,211,68,95,59,199,26,159,126,75,183,34,237,75,64,4,140,3,65,103,241,3,9,196,160,63,33,110,163,17,125,39,64,238,85,49,222,217,70,123,154,28,164,136,65,36,109,49,135,233,218,132,55,38,105,89,126,41,77,40,13,235,192,115,249,132,220,22,1,140,47,17,131,228,54,42,123,197,33,23,237,109,134,183,116,102,161,43,157,185,167,24,59,17,195,2,246,58,241,205,209,134,156,155,172,150,116,114,124,141,30,179,71,253,166,207,73,229,188,15,133,154,239,181,83,34,100,160,22,253,7,139,144,235,93,63,168,5,230,19,200,184,72,121,228,84,138,81,184,221,142,175,82,5,111,243,75,190,42,64,212,29,202,89,130,77,92,198,116,91,186,184,158,93,138,186,24,253,51,7,158,34,93,103,85,113,159,92,28,209,191,29,142,25,233,49,106,87,229,176,31,58,35,168,172,91,82,135,45,5,195,107,139,187,47,109,179,232,189,98,143,54,175,238,233,211,222,137,145,100,88,100,162,141,170,37,210,0,195,183,164,223,71,249,158,0,248,210,143,186,211,224,213,105,142,217,246,25,119,15,48,23,88,5,30,183,246,134,239,27,121,225,9,147,208,207,69,174,110,194,10,95,120,96,6,228,186,62,204,29,6,179,251,178,82,82,93,177,50,124,165,21,78,68,88,28,111,131,14,190,27,236,207,234,63,162,65,10,227,87,221,11,71,176,248,211,34,82,188,14,243,211,61,25,165,248,101,95,61,201,49,137,221,162,155,162,183,13,211,79,56,83,215,166,147,192,48,164,216,39,57,242,203,205,51,159,225,191,22,243,196,13,178,214,16,167,224,60,196,134,249,192,80,174,97,211,185,169,37,61,80,220,13,59,209,160,123,179,28,182,67,169,72,140,85,181,88,191,8,223,130,153,110,252,199,97,8,171,186,162,73,78,19,233,67,130,99,193,55,185,233,5,106,27,0,245,140,182,229,39,200,148,45,210,251,112,38,37,106,88,1,66,46,236,110,124,134,53,94,85,224,155,100,241,43,78,8,229,229,38,18,76,61,17,210,90,82,98,75,7,109,115,68,130,254,167,102,188,61,121,240,20,60,42,18,6,13,252,7,12,10,71,221,223,34,47,74,208,87,70,27,101,158,160,88,10,146,125,31,93,79,11,51,183,4,132,88,194,191,115,166,111,32,222,12,62,15,61,229,75,118,138,223,222,169,86,230,180,250,206,48,14,3,15,208,67,7,24,201,110,152,135,236,34,89,155,250,70,199,158,187,211,145,186,211,213,111,61,67,204,250,187,3,47,191,85,224,118,21,68,165,171,21,217,245,95,156,142,46,3,195,198,225,84,228,187,245,148,152,49,129,183,172,43,57,50,255,239,213,106,58,143,110,64,222,212,38,107,21,45,167,255,204,77,99,246,23,164,124,173,201,14,187,117,122,24,232,240,247,248,21,227,208,114,85,115,242,6,83,50,38,17,17,64,78,148,101,73,155,30,240,154,242,134,92,194,51,22,68,63,245,22,11,5,247,100,172,176,75,41,49,161,107,177,5,226,210,199,29,112,186,115,219,65,12,85,141,64,33,143,131,244,238,190,3,35,119,92,125,45,175,102,27,220,32,48,6,101,210,218,230,52,223,144,251,5,57,185,43,78,82,183,105,216,245,3,84,230,111,84,100,26,119,29,6,49,231,249,126,241,92,114,61,214,20,216,153,3,229,137,155,115,155,172,233,43,141,213,156,196,108,46,45,207,32,146,235,220,12,70,66,93,206,154,231,9,111,237,54,186,50,225,219,198,253,194,253,81,45,64,239,207,237,151,112,16,9,151,48,149,129,190,88,4,223,82,126,124,147,128,87,151,251,70,57,24,174,105,167,208,11,32,100,92,46,254,227,227,95,162,117,10,69,160,224,204,15,244,193,65,51,81,87,136,94,60,128,166,124,84,211,174,75,197,52,164,197,165,2,243,182,43,117,85,104,79,105,163,184,77,157,224,182,33,3,98,157,204,25,93,134,242,37,212,182,168,200,159,155,3,84,155,201,194,89,181,131,33,120,173,31,107,115,63,89,33,193,212,186,219,66,152,92,213,17,6,196,217,28,145,231,86,71,88,160,176,188,112,39,243,40,213,227,8,72,207,34,58,43,1,186,202,251,227,114,91,180,135,81,252,200,169,88,131,107,34,196,12,43,98,45,213,14,46,181,14,31,163,51,230,97,13,154,99,244,183,86,50,28,47,140,30,250,235,238,33,223,110,146,9,216,34,244,236,155,165,175,243,135,6,197,100,66,9,15,87,209,134,179,2,187,52,196,186,45,93,216,56,76,251,30,90,59,175,175,222,104,217,14,253,149,90,148,63,51,139,150,113,190,16,40,14,78,230,176,90,209,205,218,156,5,34,185,92,36,203,139,17,199,220,136,39,26,116,143,224,210,232,167,143,168,98,2,173,107,242,202,230,10,112,50,205,115,18,99,70,45,187,87,255,145,8,255,131,151,88,189,45,194,191,109,93,254,164,42,27,167,119,165,205,118,2,135,116,149,229,130,121,16,98,193,207,219,242,132,22,99,126,195,76,92,5,21,156,77,18,113,156,237,228,60,226,12,77,185,36,71,34,78,126,188,162,190,189,214,41,71,131,56,76,248,127,68,133,220,235,102,33,238,205,43,73,164,45,97,214,99,130,247,251,98,243,76,215,102,167,183,231,56,207,234,251,103,120,112,73,245,161,121,173,232,117,110,54,129,156,148,139,133,103,113,39,58,218,104,183,137,70,140,202,155,166,218,77,150,224,68,28,24,67,144,27,180,155,235,171,24,195,112,40,107,12,210,218,91,115,173,65,126,100,243};
        genesis = CreateGenesisBlock(1514736000, uint256S("0"), 0x2007ffff, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();
#ifdef PRINTS_OUT
		mery(&genesis, "test", nEquihashN, nEquihashK, "tromp");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x06b10fdee77b868501c6c06708cb3bf2a7baab1939a2f931882ad7e43bf247ad"));
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
				{0, uint256S("0x06b10fdee77b868501c6c06708cb3bf2a7baab1939a2f931882ad7e43bf247ad")}
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

		vuc sol = {9,15,150,251,193,143,201,116,255,100,103,184,30,167,20,107,59,197,10,112,100,184,101,29,54,17,216,65,210,40,254,244,131,90,63,169};
        genesis = CreateGenesisBlock(1514736000, uint256S("0"), 0x200f0f0f, 1, 50 * COIN, sol);
        consensus.hashGenesisBlock = genesis.GetHash();

#ifdef PRINTS_OUT
		mery(&genesis, "reg", nEquihashN, nEquihashK, "default");
#endif
        assert(consensus.hashGenesisBlock ==
               uint256S("0x0cb3cf9eb04524afa6b8872d56ff64e8183296bd887c7c8d859f6203f734f5ac"));
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
                              {0, uint256S("0x0cb3cf9eb04524afa6b8872d56ff64e8183296bd887c7c8d859f6203f734f5ac")},
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
