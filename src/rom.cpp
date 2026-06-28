#include "rom.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>

ROM::ROM(const std::string& filename) {
    // 1. Vérification propre de l'existence avec std::filesystem (C++17)
    if (!std::filesystem::exists(filename)) {
        throw std::runtime_error("Le fichier ROM est introuvable : " + filename);
    }

    
    auto filesize = std::filesystem::file_size(filename);
    data.resize(filesize);

    //open
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Impossible d'ouvrir le fichier ROM : " + filename);
    }

    if (!file.read(reinterpret_cast<char*>(data.data()), filesize)) {
        throw std::runtime_error("Erreur lors de la lecture du fichier ROM : " + filename);
    }
}

uint8_t ROM::read(uint16_t addr) const {
    if (addr < data.size()) {
        return data[addr];
    }
    // Comportement "matériel" : on ne crash pas, on renvoie une valeur par défaut
    // (sur le bus Game Boy, une lecture non mappée ou hors limites lit souvent 0xFF)
    return 0xFF;
}
