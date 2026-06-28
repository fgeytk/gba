#pragma once

#include <array>
#include <cstdint>

class RAM {
public:
    RAM();

    // Accesseurs pour la WRAM (Work RAM : 0xC000 - 0xDFFF)
    uint8_t read_wram(uint16_t addr) const;
    void write_wram(uint16_t addr, uint8_t value);

    // Accesseurs pour la HRAM (High RAM : 0xFF80 - 0xFFFE)
    uint8_t read_hram(uint16_t addr) const;
    void write_hram(uint16_t addr, uint8_t value);

private:
    std::array<uint8_t, 8192> wram; // 8 Ko de RAM de travail
    std::array<uint8_t, 127> hram;  // 127 octets de RAM rapide (High RAM)
};

