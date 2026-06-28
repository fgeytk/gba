#include <iostream>
#include <memory>
#include "bus.hpp"
#include "cpu.hpp"

int main(int argc, char* argv[]) {
    std::cout << "DMG-001-PROTOTYPE" << std::endl;
    std::cout << "Nintendo Research and Development 1" << std::endl;
    
    // 1. Vérification des arguments (à faire AVANT d'allouer quoi que ce soit)
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <rom_file>" << std::endl;
        return 1;
    }

    try {
        // 2. Initialisation de la carte mère
        Bus bus;

        // 3. Chargement et insertion de la cartouche
        bus.insert_rom(std::make_unique<ROM>(argv[1]));

        // 4. Initialisation du processeur
        CPU cpu(bus);

        // 5. Boucle d'exécution infinie
        while (true) {
            cpu.step();
        }

    } catch (const std::exception& e) {
        // Si le fichier ROM n'existe pas, rom.cpp lance une exception.
        // On l'attrape ici pour afficher une belle erreur au lieu de planter sauvagement.
        std::cerr << "Erreur : " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
