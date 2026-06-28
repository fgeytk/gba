#pragma once

#include <cstdint>


class Bus;

class CPU {
public:
    // Le CPU est connecté au Bus dès sa création via une référence C++
    explicit CPU(Bus& bus);

    // --- Registres 8 bits individuels ---
    uint8_t a; // Accumulateur (résultats des opérations)
    uint8_t f; // Flags (registre d'état contenant les drapeaux Z, N, H, C)
    uint8_t b;
    uint8_t c;
    uint8_t d;
    uint8_t e;
    uint8_t h;
    uint8_t l;

    // --- Registres 16 bits ---
    uint16_t pc; // Program Counter (pointeur d'instruction)
    uint16_t sp; // Stack Pointer (pointeur de pile)

    // -- Accesseurs pour les registres 16 bits combinés (AF, BC, DE, HL) ---
    uint16_t get_af() const;
    void set_af(uint16_t value);

    uint16_t get_bc() const;
    void set_bc(uint16_t value);

    uint16_t get_de() const;
    void set_de(uint16_t value);

    uint16_t get_hl() const;
    void set_hl(uint16_t value);

    // --- Manipulation des Flags (Registre F) ---
    void set_flag_z(bool value);
    void set_flag_n(bool value);
    void set_flag_h(bool value);
    void set_flag_c(bool value);

    bool get_flag_z() const;
    bool get_flag_n() const;
    bool get_flag_h() const;
    bool get_flag_c() const;

    // --- Helpers d'instructions ---
    void add_a(uint8_t value);
    void push_16(uint16_t value);
    uint16_t pop_16();

    // --- Accès mémoire via le Bus (8-bit et 16-bit Little-Endian) ---
    uint8_t read(uint16_t addr) const;
    void write(uint16_t addr, uint8_t value);

    uint16_t read_16(uint16_t addr) const;
    void write_16(uint16_t addr, uint16_t value);

    void step(); // Exécute une instruction CPU (à implémenter)

private:
    void execute(uint8_t opcode); // Exécute une instruction spécifique (à implémenter)
    Bus& bus; // Référence vers le bus mémoire pour interagir avec la RAM/ROM
};

