
#ifndef _PERSISTENT_RAM_
#define _PERSISTENT_RAM_

#include <cstdint>
#include <map> 

using namespace std;

class PersistentRAM {
private:
    const char *gameName;
    map<int, uint8_t *> memoryBanks;

public:
    PersistentRAM(const char *gameName);
    ~PersistentRAM();

    uint8_t *getRamBank(int);
};

#endif