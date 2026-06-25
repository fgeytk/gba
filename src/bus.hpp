#ifndef BUS_HPP
#define BUS_HPP

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
  // 64 KB de mémoire adressable (0x0000 - 0xFFFF)
  std::array<uint8_t, 65536> memory;
};

#endif // BUS_HPP
