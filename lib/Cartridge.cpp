
#include "Cartridge.hpp"
#include "Util/LogUtil.hpp"

void Cartridge::parse(uint8_t *rom, CartridgeInfo** info) {
    *info = (CartridgeInfo *)(rom + CARTRIDGE_INFO_START_OFFSET);

    TRACE_CART_INFO("Game: " << info->gameTitle << endl);
    TRACE_CART_INFO("MBC Type: " << cout8Hex(info->cartridgeType) << endl);
    TRACE_CART_INFO("ROM Size: " << cout8Hex(info->romSize) << endl);

}