#pragma once

#include "ram.hpp"
#include "rom.hpp"
#include <array>
#include <cstdint>
#include <memory>

class Bus {
public:
  // lecture  16-bit
  uint8_t read(uint16_t addr) const;

  // écriture  16-bit
  void write(uint16_t addr, uint8_t value);

  void insert_rom(std::unique_ptr<ROM> rom);

private:
  std::unique_ptr<ROM> rom; //un
  RAM ram;                        // Notre WRAM et HRAM
};

