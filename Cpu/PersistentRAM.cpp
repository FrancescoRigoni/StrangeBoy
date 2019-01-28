
#include <sys/stat.h>
#include <sstream>
#include <iostream>
#include <fstream>

#include "PersistentRAM.hpp"
#include "Memory.hpp"

#define SAVE_GAME_DIR "savegame"
#define RAM_FILE_NAME "ram.bin"

using namespace std;

PersistentRAM::PersistentRAM(const char *gameName) {
    this->gameName = gameName;

    stringstream saveDirName;
    saveDirName << SAVE_GAME_DIR << "/" << this->gameName;
    mkdir(saveDirName.str().c_str(), 0777);

    stringstream saveFileName;
    saveFileName << saveDirName.str() << "/" << RAM_FILE_NAME;

    ifstream saveFile;
    saveFile.open(saveFileName.str().c_str(), ios::in | ios::binary);
    if (!saveFile.good()) {
        cerr << "Persistent RAM file " << saveFileName.str().c_str() << " was not found or not read" << endl;
        return;
    }

    saveFile.seekg(0, ios::end);
    int saveFileSize = saveFile.tellg();
    for (int i = 0; i < saveFileSize; i += RAM_BANK_SIZE) {
        uint8_t *bank = new uint8_t[RAM_BANK_SIZE];
        saveFile.seekg(i, ios::beg);
        saveFile.read((char *)bank, RAM_BANK_SIZE);
        memoryBanks[i/RAM_BANK_SIZE] = bank;
    }

    saveFile.close();
}

PersistentRAM::~PersistentRAM() {
    mkdir(SAVE_GAME_DIR, 0777);

    stringstream saveDirName;
    saveDirName << SAVE_GAME_DIR << "/" << this->gameName;
    mkdir(saveDirName.str().c_str(), 0777);

    stringstream saveFileName;
    saveFileName << saveDirName.str() << "/" << RAM_FILE_NAME;

    ofstream saveFile (saveFileName.str().c_str(), ios::out | ios::binary);
    for (auto it : memoryBanks) {
        saveFile.write((const char*)it.second, RAM_BANK_SIZE);
        delete[] it.second;
    }
    saveFile.close();
}

uint8_t *PersistentRAM::getRamBank(int ramBank) {
    if (memoryBanks.find(ramBank) == memoryBanks.end()) {
        memoryBanks[ramBank] = new uint8_t[RAM_BANK_SIZE];
    }
    return memoryBanks[ramBank];
}
