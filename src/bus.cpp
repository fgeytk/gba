#include "bus.hpp"
#include <cstdint>

Bus::Bus() { memory.fill(0); }

uint8_t Bus::read(uint16_t addr) const { return memory[addr]; }

void Bus::write(uint16_t addr, uint8_t value) { memory[addr] = value; }
