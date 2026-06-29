#pragma once

#include <cstdint>

class CPU; // forward declaration

// Structure décrivant une instruction du Game Boy (Sharp LR35902).
// Chaque opcode (0x00–0xFF) et chaque opcode CB-prefixed possède une entrée.
struct Instruction {
    const char* name;                  // Mnémonique lisible (ex: "LD B, d8")
    int cycles;                        // Nombre de cycles machine de base
    int alt_cycles;                    // Cycles alternatifs (branche prise/non prise), 0 si non applicable
    int (*execute)(CPU& cpu);          // Handler d'exécution, retourne les cycles réellement consommés
};

// Table principale : opcodes 0x00 à 0xFF
extern const Instruction OPCODES[256];

// Table secondaire : opcodes CB-prefixed (0xCB 0x00 à 0xCB 0xFF)
extern const Instruction CB_OPCODES[256];
