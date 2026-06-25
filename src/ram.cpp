#include "ram.hpp"

RAM::RAM() {
    wram.fill(0);
    hram.fill(0);
}

uint8_t RAM::read_wram(uint16_t addr) const {
    // 0xC000 - 0xDFFF se traduit par un index de 0 à 8191 (0x1FFF)
    return wram[addr - 0xC000];
}

void RAM::write_wram(uint16_t addr, uint8_t value) {
    wram[addr - 0xC000] = value;
}

uint8_t RAM::read_hram(uint16_t addr) const {
    // 0xFF80 - 0xFFFE se traduit par un index de 0 à 126
    return hram[addr - 0xFF80];
}

void RAM::write_hram(uint16_t addr, uint8_t value) {
    hram[addr - 0xFF80] = value;
}
