#ifndef BTCNANO_SEEDER_BTCNANO_H
#define BTCNANO_SEEDER_BTCNANO_H

#include "protocol.h"

bool TestNode(const CService &cip, int &ban, int &client, std::string &clientSV,
              int &blocks, std::vector<CAddress> *vAddr);

#endif
