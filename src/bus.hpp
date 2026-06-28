#pragma once

#include "ram.hpp"
#include <array>
#include <cstdint>

class Bus {
public:
  Bus();

  // lecture  16-bit
  uint8_t read(uint16_t addr) const;

  // écriture  16-bit
  void write(uint16_t addr, uint8_t value);

private:
  std::array<uint8_t, 32768> rom; // 32 KB de ROM Cartouche (0x0000 - 0x7FFF)
  RAM ram;                        // Notre WRAM et HRAM
};

