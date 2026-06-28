#include "bus.hpp"
#include <cstdint>


uint8_t Bus::read(uint16_t addr) const {
    // 0x0000 - 0x7FFF : ROM de la cartouche (32 Ko)
    if (addr <= 0x7FFF) {
        return rom ? rom->read(addr) : 0xFF; //on renvoie 0xFF si aucune ROM n'est insérée
    }
    // 0xC000 - 0xDFFF : WRAM (Work RAM - 8 Ko)
    else if (addr >= 0xC000 && addr <= 0xDFFF) {
        return ram.read_wram(addr);
    }
    // 0xE000 - 0xFDFF : Echo RAM (Miroir de la zone 0xC000 - 0xDDFF)
    else if (addr >= 0xE000 && addr <= 0xFDFF) {
        return ram.read_wram(addr - 0x2000);
    }
    // 0xFF80 - 0xFFFE : HRAM (High RAM - 127 octets)
    else if (addr >= 0xFF80 && addr <= 0xFFFE) {
        return ram.read_hram(addr);
    }

    // Comportement par défaut pour les zones non connectées (PPU, I/O, etc.)
    return 0xFF;
}

void Bus::write(uint16_t addr, uint8_t value) {
    // 0x0000 - 0x7FFF : ROM de la cartouche (Lecture seule, on ignore l'écriture)
    if (addr <= 0x7FFF) {
        return;
    }
    // 0xC000 - 0xDFFF : WRAM
    else if (addr >= 0xC000 && addr <= 0xDFFF) {
        ram.write_wram(addr, value);
    }
    // 0xE000 - 0xFDFF : Echo RAM
    else if (addr >= 0xE000 && addr <= 0xFDFF) {
        ram.write_wram(addr - 0x2000, value);
    }
    // 0xFF80 - 0xFFFE : HRAM
    else if (addr >= 0xFF80 && addr <= 0xFFFE) {
        ram.write_hram(addr, value);
    }
}

void Bus::insert_rom(std::unique_ptr<ROM> new_rom) {
    rom = std::move(new_rom);
}