#ifndef ROM_HPP
#define ROM_HPP


#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <cstdint>
#include <stdexcept>

class ROM {
public:
    ROM(const std::string& filename);
    uint8_t read(uint16_t addr) const;

private:
    std::vector<uint8_t> data;
};

#endif // ROM_HPP