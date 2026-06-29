#include "opcodes.hpp"
#include "cpu.hpp"

#include <cstdio>
#include <stdexcept>

// =============================================================================
// Tous les handlers d'opcodes sont regroupés ici par famille.
// Chaque handler reçoit une référence vers le CPU et retourne le nombre
// de cycles consommés par l'instruction.
// =============================================================================

namespace {

// =============================================================================
//  OPCODE NON IMPLÉMENTÉ
// =============================================================================

int op_undefined(CPU& cpu) {
    // On recule le PC de 1 pour pointer sur l'opcode fautif
    uint16_t fault_addr = cpu.pc - 1;
    uint8_t fault_opcode = cpu.read(fault_addr);

    char error_msg[64];
    snprintf(error_msg, sizeof(error_msg),
             "Opcode non implemente : 0x%02X a l'adresse 0x%04X",
             fault_opcode, fault_addr);
    throw std::runtime_error(error_msg);
}

// =============================================================================
//  NOP
// =============================================================================

int op_nop(CPU& cpu) {
    // Ne fait rien du tout
    return 4;
}

// =============================================================================
//  LOADS 8 BITS — LD r, d8
//  Charge une valeur immédiate 8 bits dans un registre
// =============================================================================

int op_ld_b_d8(CPU& cpu) {
    cpu.b = cpu.fetch8();
    return 8;
}

int op_ld_c_d8(CPU& cpu) {
    cpu.c = cpu.fetch8();
    return 8;
}

int op_ld_d_d8(CPU& cpu) {
    cpu.d = cpu.fetch8();
    return 8;
}

int op_ld_e_d8(CPU& cpu) {
    cpu.e = cpu.fetch8();
    return 8;
}

int op_ld_h_d8(CPU& cpu) {
    cpu.h = cpu.fetch8();
    return 8;
}

int op_ld_l_d8(CPU& cpu) {
    cpu.l = cpu.fetch8();
    return 8;
}

int op_ld_hl_d8(CPU& cpu) {
    // Attention : on écrit la donnée lue en mémoire à l'adresse contenue dans HL
    cpu.write(cpu.get_hl(), cpu.fetch8());
    return 12;
}

int op_ld_a_d8(CPU& cpu) {
    cpu.a = cpu.fetch8();
    return 8;
}

// =============================================================================
//  JUMPS ABSOLUS — JP a16
//  Saut inconditionnel ou conditionnel vers une adresse 16 bits
// =============================================================================

int op_jp_a16(CPU& cpu) {
    // Jump inconditionnel à une adresse 16 bits
    cpu.pc = cpu.fetch16();
    return 16;
}

int op_jp_nz_a16(CPU& cpu) {
    // JP NZ, a16 — saut si le flag Z n'est PAS actif
    uint16_t addr = cpu.fetch16();
    if (!cpu.get_flag_z()) {
        cpu.pc = addr;
        return 16;
    }
    return 12;
}

int op_jp_z_a16(CPU& cpu) {
    // JP Z, a16 — saut si le flag Z EST actif
    uint16_t addr = cpu.fetch16();
    if (cpu.get_flag_z()) {
        cpu.pc = addr;
        return 16;
    }
    return 12;
}

int op_jp_nc_a16(CPU& cpu) {
    // JP NC, a16 — saut si le flag C n'est PAS actif
    uint16_t addr = cpu.fetch16();
    if (!cpu.get_flag_c()) {
        cpu.pc = addr;
        return 16;
    }
    return 12;
}

int op_jp_c_a16(CPU& cpu) {
    // JP C, a16 — saut si le flag C EST actif
    uint16_t addr = cpu.fetch16();
    if (cpu.get_flag_c()) {
        cpu.pc = addr;
        return 16;
    }
    return 12;
}

// =============================================================================
//  JUMPS RELATIFS — JR e8
//  Saut relatif au PC courant avec un offset signé 8 bits
// =============================================================================

int op_jr_e8(CPU& cpu) {
    // JR e8 — saut relatif inconditionnel
    int8_t offset = (int8_t)cpu.fetch8();
    cpu.pc += offset;
    return 12;
}

int op_jr_nz_e8(CPU& cpu) {
    // JR NZ, e8 — saut relatif si Z n'est PAS actif
    int8_t offset = (int8_t)cpu.fetch8();
    if (!cpu.get_flag_z()) {
        cpu.pc += offset;
        return 12;
    }
    return 8;
}

int op_jr_z_e8(CPU& cpu) {
    // JR Z, e8 — saut relatif si Z EST actif
    int8_t offset = (int8_t)cpu.fetch8();
    if (cpu.get_flag_z()) {
        cpu.pc += offset;
        return 12;
    }
    return 8;
}

int op_jr_nc_e8(CPU& cpu) {
    // JR NC, e8 — saut relatif si C n'est PAS actif
    int8_t offset = (int8_t)cpu.fetch8();
    if (!cpu.get_flag_c()) {
        cpu.pc += offset;
        return 12;
    }
    return 8;
}

int op_jr_c_e8(CPU& cpu) {
    // JR C, e8 — saut relatif si C EST actif
    int8_t offset = (int8_t)cpu.fetch8();
    if (cpu.get_flag_c()) {
        cpu.pc += offset;
        return 12;
    }
    return 8;
}

// =============================================================================
//  PILE (STACK) — PUSH / POP
// =============================================================================

int op_push_bc(CPU& cpu) {
    cpu.push_16(cpu.get_bc());
    return 16;
}

int op_push_de(CPU& cpu) {
    cpu.push_16(cpu.get_de());
    return 16;
}

int op_push_hl(CPU& cpu) {
    cpu.push_16(cpu.get_hl());
    return 16;
}

int op_push_af(CPU& cpu) {
    cpu.push_16(cpu.get_af());
    return 16;
}

int op_pop_bc(CPU& cpu) {
    cpu.set_bc(cpu.pop_16());
    return 12;
}

int op_pop_de(CPU& cpu) {
    cpu.set_de(cpu.pop_16());
    return 12;
}

int op_pop_hl(CPU& cpu) {
    cpu.set_hl(cpu.pop_16());
    return 12;
}

int op_pop_af(CPU& cpu) {
    cpu.set_af(cpu.pop_16());
    return 12;
}

// =============================================================================
//  CALL / RET — Appels et retours de fonctions
// =============================================================================

int op_call_a16(CPU& cpu) {
    // CALL a16 — Appel de fonction
    uint16_t call_addr = cpu.fetch16();
    // On sauvegarde l'adresse de retour (PC après l'instruction CALL) sur la pile
    cpu.push_16(cpu.pc);
    // Et on saute vers la fonction
    cpu.pc = call_addr;
    return 24;
}

int op_ret(CPU& cpu) {
    // RET — Retour inconditionnel
    // On récupère l'adresse sauvegardée et on la remet dans le PC
    cpu.pc = cpu.pop_16();
    return 16;
}

// =============================================================================
//  ALU — ADD A, r
//  Addition 8 bits : A = A + registre, avec mise à jour des flags
// =============================================================================

int op_add_a_b(CPU& cpu) {
    cpu.add_a(cpu.b);
    return 4;
}

int op_add_a_c(CPU& cpu) {
    cpu.add_a(cpu.c);
    return 4;
}

int op_add_a_d(CPU& cpu) {
    cpu.add_a(cpu.d);
    return 4;
}

int op_add_a_e(CPU& cpu) {
    cpu.add_a(cpu.e);
    return 4;
}

int op_add_a_h(CPU& cpu) {
    cpu.add_a(cpu.h);
    return 4;
}

int op_add_a_l(CPU& cpu) {
    cpu.add_a(cpu.l);
    return 4;
}

int op_add_a_hl(CPU& cpu) {
    // ADD A, (HL) — lecture mémoire à l'adresse pointée par HL
    cpu.add_a(cpu.read(cpu.get_hl()));
    return 8;
}

int op_add_a_a(CPU& cpu) {
    cpu.add_a(cpu.a);
    return 4;
}

} // namespace anonyme


// =============================================================================
//  TABLE PRINCIPALE — OPCODES[256]
//
//  Chaque ligne correspond à un opcode de 0x00 à 0xFF.
//  Format : { "NOM", cycles, alt_cycles, handler }
//
//  Les entrées non implémentées pointent vers op_undefined qui lèvera
//  une exception avec l'adresse et l'opcode fautif.
// =============================================================================

const Instruction OPCODES[256] = {
    // 0x00 — 0x0F
    /* 0x00 */ { "NOP",          4,  0, op_nop },
    /* 0x01 */ { "LD BC, d16",   0,  0, op_undefined },
    /* 0x02 */ { "LD (BC), A",   0,  0, op_undefined },
    /* 0x03 */ { "INC BC",       0,  0, op_undefined },
    /* 0x04 */ { "INC B",        0,  0, op_undefined },
    /* 0x05 */ { "DEC B",        0,  0, op_undefined },
    /* 0x06 */ { "LD B, d8",     8,  0, op_ld_b_d8 },
    /* 0x07 */ { "RLCA",         0,  0, op_undefined },
    /* 0x08 */ { "LD (a16), SP", 0,  0, op_undefined },
    /* 0x09 */ { "ADD HL, BC",   0,  0, op_undefined },
    /* 0x0A */ { "LD A, (BC)",   0,  0, op_undefined },
    /* 0x0B */ { "DEC BC",       0,  0, op_undefined },
    /* 0x0C */ { "INC C",        0,  0, op_undefined },
    /* 0x0D */ { "DEC C",        0,  0, op_undefined },
    /* 0x0E */ { "LD C, d8",     8,  0, op_ld_c_d8 },
    /* 0x0F */ { "RRCA",         0,  0, op_undefined },

    // 0x10 — 0x1F
    /* 0x10 */ { "STOP",         0,  0, op_undefined },
    /* 0x11 */ { "LD DE, d16",   0,  0, op_undefined },
    /* 0x12 */ { "LD (DE), A",   0,  0, op_undefined },
    /* 0x13 */ { "INC DE",       0,  0, op_undefined },
    /* 0x14 */ { "INC D",        0,  0, op_undefined },
    /* 0x15 */ { "DEC D",        0,  0, op_undefined },
    /* 0x16 */ { "LD D, d8",     8,  0, op_ld_d_d8 },
    /* 0x17 */ { "RLA",          0,  0, op_undefined },
    /* 0x18 */ { "JR e8",       12,  0, op_jr_e8 },
    /* 0x19 */ { "ADD HL, DE",   0,  0, op_undefined },
    /* 0x1A */ { "LD A, (DE)",   0,  0, op_undefined },
    /* 0x1B */ { "DEC DE",       0,  0, op_undefined },
    /* 0x1C */ { "INC E",        0,  0, op_undefined },
    /* 0x1D */ { "DEC E",        0,  0, op_undefined },
    /* 0x1E */ { "LD E, d8",     8,  0, op_ld_e_d8 },
    /* 0x1F */ { "RRA",          0,  0, op_undefined },

    // 0x20 — 0x2F
    /* 0x20 */ { "JR NZ, e8",   12,  8, op_jr_nz_e8 },
    /* 0x21 */ { "LD HL, d16",   0,  0, op_undefined },
    /* 0x22 */ { "LD (HL+), A",  0,  0, op_undefined },
    /* 0x23 */ { "INC HL",       0,  0, op_undefined },
    /* 0x24 */ { "INC H",        0,  0, op_undefined },
    /* 0x25 */ { "DEC H",        0,  0, op_undefined },
    /* 0x26 */ { "LD H, d8",     8,  0, op_ld_h_d8 },
    /* 0x27 */ { "DAA",          0,  0, op_undefined },
    /* 0x28 */ { "JR Z, e8",    12,  8, op_jr_z_e8 },
    /* 0x29 */ { "ADD HL, HL",   0,  0, op_undefined },
    /* 0x2A */ { "LD A, (HL+)",  0,  0, op_undefined },
    /* 0x2B */ { "DEC HL",       0,  0, op_undefined },
    /* 0x2C */ { "INC L",        0,  0, op_undefined },
    /* 0x2D */ { "DEC L",        0,  0, op_undefined },
    /* 0x2E */ { "LD L, d8",     8,  0, op_ld_l_d8 },
    /* 0x2F */ { "CPL",          0,  0, op_undefined },

    // 0x30 — 0x3F
    /* 0x30 */ { "JR NC, e8",   12,  8, op_jr_nc_e8 },
    /* 0x31 */ { "LD SP, d16",   0,  0, op_undefined },
    /* 0x32 */ { "LD (HL-), A",  0,  0, op_undefined },
    /* 0x33 */ { "INC SP",       0,  0, op_undefined },
    /* 0x34 */ { "INC (HL)",     0,  0, op_undefined },
    /* 0x35 */ { "DEC (HL)",     0,  0, op_undefined },
    /* 0x36 */ { "LD (HL), d8", 12,  0, op_ld_hl_d8 },
    /* 0x37 */ { "SCF",          0,  0, op_undefined },
    /* 0x38 */ { "JR C, e8",    12,  8, op_jr_c_e8 },
    /* 0x39 */ { "ADD HL, SP",   0,  0, op_undefined },
    /* 0x3A */ { "LD A, (HL-)",  0,  0, op_undefined },
    /* 0x3B */ { "DEC SP",       0,  0, op_undefined },
    /* 0x3C */ { "INC A",        0,  0, op_undefined },
    /* 0x3D */ { "DEC A",        0,  0, op_undefined },
    /* 0x3E */ { "LD A, d8",     8,  0, op_ld_a_d8 },
    /* 0x3F */ { "CCF",          0,  0, op_undefined },

    // 0x40 — 0x4F : LD B/C, r
    /* 0x40 */ { "LD B, B",      0,  0, op_undefined },
    /* 0x41 */ { "LD B, C",      0,  0, op_undefined },
    /* 0x42 */ { "LD B, D",      0,  0, op_undefined },
    /* 0x43 */ { "LD B, E",      0,  0, op_undefined },
    /* 0x44 */ { "LD B, H",      0,  0, op_undefined },
    /* 0x45 */ { "LD B, L",      0,  0, op_undefined },
    /* 0x46 */ { "LD B, (HL)",   0,  0, op_undefined },
    /* 0x47 */ { "LD B, A",      0,  0, op_undefined },
    /* 0x48 */ { "LD C, B",      0,  0, op_undefined },
    /* 0x49 */ { "LD C, C",      0,  0, op_undefined },
    /* 0x4A */ { "LD C, D",      0,  0, op_undefined },
    /* 0x4B */ { "LD C, E",      0,  0, op_undefined },
    /* 0x4C */ { "LD C, H",      0,  0, op_undefined },
    /* 0x4D */ { "LD C, L",      0,  0, op_undefined },
    /* 0x4E */ { "LD C, (HL)",   0,  0, op_undefined },
    /* 0x4F */ { "LD C, A",      0,  0, op_undefined },

    // 0x50 — 0x5F : LD D/E, r
    /* 0x50 */ { "LD D, B",      0,  0, op_undefined },
    /* 0x51 */ { "LD D, C",      0,  0, op_undefined },
    /* 0x52 */ { "LD D, D",      0,  0, op_undefined },
    /* 0x53 */ { "LD D, E",      0,  0, op_undefined },
    /* 0x54 */ { "LD D, H",      0,  0, op_undefined },
    /* 0x55 */ { "LD D, L",      0,  0, op_undefined },
    /* 0x56 */ { "LD D, (HL)",   0,  0, op_undefined },
    /* 0x57 */ { "LD D, A",      0,  0, op_undefined },
    /* 0x58 */ { "LD E, B",      0,  0, op_undefined },
    /* 0x59 */ { "LD E, C",      0,  0, op_undefined },
    /* 0x5A */ { "LD E, D",      0,  0, op_undefined },
    /* 0x5B */ { "LD E, E",      0,  0, op_undefined },
    /* 0x5C */ { "LD E, H",      0,  0, op_undefined },
    /* 0x5D */ { "LD E, L",      0,  0, op_undefined },
    /* 0x5E */ { "LD E, (HL)",   0,  0, op_undefined },
    /* 0x5F */ { "LD E, A",      0,  0, op_undefined },

    // 0x60 — 0x6F : LD H/L, r
    /* 0x60 */ { "LD H, B",      0,  0, op_undefined },
    /* 0x61 */ { "LD H, C",      0,  0, op_undefined },
    /* 0x62 */ { "LD H, D",      0,  0, op_undefined },
    /* 0x63 */ { "LD H, E",      0,  0, op_undefined },
    /* 0x64 */ { "LD H, H",      0,  0, op_undefined },
    /* 0x65 */ { "LD H, L",      0,  0, op_undefined },
    /* 0x66 */ { "LD H, (HL)",   0,  0, op_undefined },
    /* 0x67 */ { "LD H, A",      0,  0, op_undefined },
    /* 0x68 */ { "LD L, B",      0,  0, op_undefined },
    /* 0x69 */ { "LD L, C",      0,  0, op_undefined },
    /* 0x6A */ { "LD L, D",      0,  0, op_undefined },
    /* 0x6B */ { "LD L, E",      0,  0, op_undefined },
    /* 0x6C */ { "LD L, H",      0,  0, op_undefined },
    /* 0x6D */ { "LD L, L",      0,  0, op_undefined },
    /* 0x6E */ { "LD L, (HL)",   0,  0, op_undefined },
    /* 0x6F */ { "LD L, A",      0,  0, op_undefined },

    // 0x70 — 0x7F : LD (HL)/A, r
    /* 0x70 */ { "LD (HL), B",   0,  0, op_undefined },
    /* 0x71 */ { "LD (HL), C",   0,  0, op_undefined },
    /* 0x72 */ { "LD (HL), D",   0,  0, op_undefined },
    /* 0x73 */ { "LD (HL), E",   0,  0, op_undefined },
    /* 0x74 */ { "LD (HL), H",   0,  0, op_undefined },
    /* 0x75 */ { "LD (HL), L",   0,  0, op_undefined },
    /* 0x76 */ { "HALT",         0,  0, op_undefined },
    /* 0x77 */ { "LD (HL), A",   0,  0, op_undefined },
    /* 0x78 */ { "LD A, B",      0,  0, op_undefined },
    /* 0x79 */ { "LD A, C",      0,  0, op_undefined },
    /* 0x7A */ { "LD A, D",      0,  0, op_undefined },
    /* 0x7B */ { "LD A, E",      0,  0, op_undefined },
    /* 0x7C */ { "LD A, H",      0,  0, op_undefined },
    /* 0x7D */ { "LD A, L",      0,  0, op_undefined },
    /* 0x7E */ { "LD A, (HL)",   0,  0, op_undefined },
    /* 0x7F */ { "LD A, A",      0,  0, op_undefined },

    // 0x80 — 0x8F : ADD / ADC
    /* 0x80 */ { "ADD A, B",     4,  0, op_add_a_b },
    /* 0x81 */ { "ADD A, C",     4,  0, op_add_a_c },
    /* 0x82 */ { "ADD A, D",     4,  0, op_add_a_d },
    /* 0x83 */ { "ADD A, E",     4,  0, op_add_a_e },
    /* 0x84 */ { "ADD A, H",     4,  0, op_add_a_h },
    /* 0x85 */ { "ADD A, L",     4,  0, op_add_a_l },
    /* 0x86 */ { "ADD A, (HL)",  8,  0, op_add_a_hl },
    /* 0x87 */ { "ADD A, A",     4,  0, op_add_a_a },
    /* 0x88 */ { "ADC A, B",     0,  0, op_undefined },
    /* 0x89 */ { "ADC A, C",     0,  0, op_undefined },
    /* 0x8A */ { "ADC A, D",     0,  0, op_undefined },
    /* 0x8B */ { "ADC A, E",     0,  0, op_undefined },
    /* 0x8C */ { "ADC A, H",     0,  0, op_undefined },
    /* 0x8D */ { "ADC A, L",     0,  0, op_undefined },
    /* 0x8E */ { "ADC A, (HL)",  0,  0, op_undefined },
    /* 0x8F */ { "ADC A, A",     0,  0, op_undefined },

    // 0x90 — 0x9F : SUB / SBC
    /* 0x90 */ { "SUB B",        0,  0, op_undefined },
    /* 0x91 */ { "SUB C",        0,  0, op_undefined },
    /* 0x92 */ { "SUB D",        0,  0, op_undefined },
    /* 0x93 */ { "SUB E",        0,  0, op_undefined },
    /* 0x94 */ { "SUB H",        0,  0, op_undefined },
    /* 0x95 */ { "SUB L",        0,  0, op_undefined },
    /* 0x96 */ { "SUB (HL)",     0,  0, op_undefined },
    /* 0x97 */ { "SUB A",        0,  0, op_undefined },
    /* 0x98 */ { "SBC A, B",     0,  0, op_undefined },
    /* 0x99 */ { "SBC A, C",     0,  0, op_undefined },
    /* 0x9A */ { "SBC A, D",     0,  0, op_undefined },
    /* 0x9B */ { "SBC A, E",     0,  0, op_undefined },
    /* 0x9C */ { "SBC A, H",     0,  0, op_undefined },
    /* 0x9D */ { "SBC A, L",     0,  0, op_undefined },
    /* 0x9E */ { "SBC A, (HL)",  0,  0, op_undefined },
    /* 0x9F */ { "SBC A, A",     0,  0, op_undefined },

    // 0xA0 — 0xAF : AND / XOR
    /* 0xA0 */ { "AND B",        0,  0, op_undefined },
    /* 0xA1 */ { "AND C",        0,  0, op_undefined },
    /* 0xA2 */ { "AND D",        0,  0, op_undefined },
    /* 0xA3 */ { "AND E",        0,  0, op_undefined },
    /* 0xA4 */ { "AND H",        0,  0, op_undefined },
    /* 0xA5 */ { "AND L",        0,  0, op_undefined },
    /* 0xA6 */ { "AND (HL)",     0,  0, op_undefined },
    /* 0xA7 */ { "AND A",        0,  0, op_undefined },
    /* 0xA8 */ { "XOR B",        0,  0, op_undefined },
    /* 0xA9 */ { "XOR C",        0,  0, op_undefined },
    /* 0xAA */ { "XOR D",        0,  0, op_undefined },
    /* 0xAB */ { "XOR E",        0,  0, op_undefined },
    /* 0xAC */ { "XOR H",        0,  0, op_undefined },
    /* 0xAD */ { "XOR L",        0,  0, op_undefined },
    /* 0xAE */ { "XOR (HL)",     0,  0, op_undefined },
    /* 0xAF */ { "XOR A",        0,  0, op_undefined },

    // 0xB0 — 0xBF : OR / CP
    /* 0xB0 */ { "OR B",         0,  0, op_undefined },
    /* 0xB1 */ { "OR C",         0,  0, op_undefined },
    /* 0xB2 */ { "OR D",         0,  0, op_undefined },
    /* 0xB3 */ { "OR E",         0,  0, op_undefined },
    /* 0xB4 */ { "OR H",         0,  0, op_undefined },
    /* 0xB5 */ { "OR L",         0,  0, op_undefined },
    /* 0xB6 */ { "OR (HL)",      0,  0, op_undefined },
    /* 0xB7 */ { "OR A",         0,  0, op_undefined },
    /* 0xB8 */ { "CP B",         0,  0, op_undefined },
    /* 0xB9 */ { "CP C",         0,  0, op_undefined },
    /* 0xBA */ { "CP D",         0,  0, op_undefined },
    /* 0xBB */ { "CP E",         0,  0, op_undefined },
    /* 0xBC */ { "CP H",         0,  0, op_undefined },
    /* 0xBD */ { "CP L",         0,  0, op_undefined },
    /* 0xBE */ { "CP (HL)",      0,  0, op_undefined },
    /* 0xBF */ { "CP A",         0,  0, op_undefined },

    // 0xC0 — 0xCF
    /* 0xC0 */ { "RET NZ",       0,  0, op_undefined },
    /* 0xC1 */ { "POP BC",      12,  0, op_pop_bc },
    /* 0xC2 */ { "JP NZ, a16",  16, 12, op_jp_nz_a16 },
    /* 0xC3 */ { "JP a16",      16,  0, op_jp_a16 },
    /* 0xC4 */ { "CALL NZ, a16", 0,  0, op_undefined },
    /* 0xC5 */ { "PUSH BC",     16,  0, op_push_bc },
    /* 0xC6 */ { "ADD A, d8",    0,  0, op_undefined },
    /* 0xC7 */ { "RST 00H",     0,  0, op_undefined },
    /* 0xC8 */ { "RET Z",        0,  0, op_undefined },
    /* 0xC9 */ { "RET",         16,  0, op_ret },
    /* 0xCA */ { "JP Z, a16",   16, 12, op_jp_z_a16 },
    /* 0xCB */ { "PREFIX CB",    0,  0, op_undefined }, // Géré spécialement dans step()
    /* 0xCC */ { "CALL Z, a16",  0,  0, op_undefined },
    /* 0xCD */ { "CALL a16",    24,  0, op_call_a16 },
    /* 0xCE */ { "ADC A, d8",    0,  0, op_undefined },
    /* 0xCF */ { "RST 08H",     0,  0, op_undefined },

    // 0xD0 — 0xDF
    /* 0xD0 */ { "RET NC",       0,  0, op_undefined },
    /* 0xD1 */ { "POP DE",      12,  0, op_pop_de },
    /* 0xD2 */ { "JP NC, a16",  16, 12, op_jp_nc_a16 },
    /* 0xD3 */ { "ILLEGAL_D3",  0,  0, op_undefined },
    /* 0xD4 */ { "CALL NC, a16", 0,  0, op_undefined },
    /* 0xD5 */ { "PUSH DE",     16,  0, op_push_de },
    /* 0xD6 */ { "SUB d8",       0,  0, op_undefined },
    /* 0xD7 */ { "RST 10H",     0,  0, op_undefined },
    /* 0xD8 */ { "RET C",        0,  0, op_undefined },
    /* 0xD9 */ { "RETI",         0,  0, op_undefined },
    /* 0xDA */ { "JP C, a16",   16, 12, op_jp_c_a16 },
    /* 0xDB */ { "ILLEGAL_DB",   0,  0, op_undefined },
    /* 0xDC */ { "CALL C, a16",  0,  0, op_undefined },
    /* 0xDD */ { "ILLEGAL_DD",   0,  0, op_undefined },
    /* 0xDE */ { "SBC A, d8",    0,  0, op_undefined },
    /* 0xDF */ { "RST 18H",     0,  0, op_undefined },

    // 0xE0 — 0xEF
    /* 0xE0 */ { "LDH (a8), A",  0,  0, op_undefined },
    /* 0xE1 */ { "POP HL",      12,  0, op_pop_hl },
    /* 0xE2 */ { "LD (C), A",    0,  0, op_undefined },
    /* 0xE3 */ { "ILLEGAL_E3",   0,  0, op_undefined },
    /* 0xE4 */ { "ILLEGAL_E4",   0,  0, op_undefined },
    /* 0xE5 */ { "PUSH HL",     16,  0, op_push_hl },
    /* 0xE6 */ { "AND d8",       0,  0, op_undefined },
    /* 0xE7 */ { "RST 20H",     0,  0, op_undefined },
    /* 0xE8 */ { "ADD SP, e8",   0,  0, op_undefined },
    /* 0xE9 */ { "JP (HL)",      0,  0, op_undefined },
    /* 0xEA */ { "LD (a16), A",  0,  0, op_undefined },
    /* 0xEB */ { "ILLEGAL_EB",   0,  0, op_undefined },
    /* 0xEC */ { "ILLEGAL_EC",   0,  0, op_undefined },
    /* 0xED */ { "ILLEGAL_ED",   0,  0, op_undefined },
    /* 0xEE */ { "XOR d8",       0,  0, op_undefined },
    /* 0xEF */ { "RST 28H",     0,  0, op_undefined },

    // 0xF0 — 0xFF
    /* 0xF0 */ { "LDH A, (a8)",  0,  0, op_undefined },
    /* 0xF1 */ { "POP AF",      12,  0, op_pop_af },
    /* 0xF2 */ { "LD A, (C)",    0,  0, op_undefined },
    /* 0xF3 */ { "DI",           0,  0, op_undefined },
    /* 0xF4 */ { "ILLEGAL_F4",   0,  0, op_undefined },
    /* 0xF5 */ { "PUSH AF",     16,  0, op_push_af },
    /* 0xF6 */ { "OR d8",        0,  0, op_undefined },
    /* 0xF7 */ { "RST 30H",     0,  0, op_undefined },
    /* 0xF8 */ { "LD HL, SP+e8", 0,  0, op_undefined },
    /* 0xF9 */ { "LD SP, HL",    0,  0, op_undefined },
    /* 0xFA */ { "LD A, (a16)",  0,  0, op_undefined },
    /* 0xFB */ { "EI",           0,  0, op_undefined },
    /* 0xFC */ { "ILLEGAL_FC",   0,  0, op_undefined },
    /* 0xFD */ { "ILLEGAL_FD",   0,  0, op_undefined },
    /* 0xFE */ { "CP d8",        0,  0, op_undefined },
    /* 0xFF */ { "RST 38H",     0,  0, op_undefined },
};


// =============================================================================
//  TABLE CB — CB_OPCODES[256]
//
//  Opcodes préfixés par 0xCB (opérations sur les bits).
//  Vide pour l'instant — toutes les entrées pointent vers op_undefined.
//  Sera remplie au fur et à mesure de l'implémentation.
// =============================================================================

// Helper pour initialiser les 256 entrées CB à op_undefined
#define CB_UNDEF(n) { "CB " #n, 0, 0, op_undefined }

const Instruction CB_OPCODES[256] = {
    // 0x00 — 0x0F
    CB_UNDEF(0x00), CB_UNDEF(0x01), CB_UNDEF(0x02), CB_UNDEF(0x03),
    CB_UNDEF(0x04), CB_UNDEF(0x05), CB_UNDEF(0x06), CB_UNDEF(0x07),
    CB_UNDEF(0x08), CB_UNDEF(0x09), CB_UNDEF(0x0A), CB_UNDEF(0x0B),
    CB_UNDEF(0x0C), CB_UNDEF(0x0D), CB_UNDEF(0x0E), CB_UNDEF(0x0F),
    // 0x10 — 0x1F
    CB_UNDEF(0x10), CB_UNDEF(0x11), CB_UNDEF(0x12), CB_UNDEF(0x13),
    CB_UNDEF(0x14), CB_UNDEF(0x15), CB_UNDEF(0x16), CB_UNDEF(0x17),
    CB_UNDEF(0x18), CB_UNDEF(0x19), CB_UNDEF(0x1A), CB_UNDEF(0x1B),
    CB_UNDEF(0x1C), CB_UNDEF(0x1D), CB_UNDEF(0x1E), CB_UNDEF(0x1F),
    // 0x20 — 0x2F
    CB_UNDEF(0x20), CB_UNDEF(0x21), CB_UNDEF(0x22), CB_UNDEF(0x23),
    CB_UNDEF(0x24), CB_UNDEF(0x25), CB_UNDEF(0x26), CB_UNDEF(0x27),
    CB_UNDEF(0x28), CB_UNDEF(0x29), CB_UNDEF(0x2A), CB_UNDEF(0x2B),
    CB_UNDEF(0x2C), CB_UNDEF(0x2D), CB_UNDEF(0x2E), CB_UNDEF(0x2F),
    // 0x30 — 0x3F
    CB_UNDEF(0x30), CB_UNDEF(0x31), CB_UNDEF(0x32), CB_UNDEF(0x33),
    CB_UNDEF(0x34), CB_UNDEF(0x35), CB_UNDEF(0x36), CB_UNDEF(0x37),
    CB_UNDEF(0x38), CB_UNDEF(0x39), CB_UNDEF(0x3A), CB_UNDEF(0x3B),
    CB_UNDEF(0x3C), CB_UNDEF(0x3D), CB_UNDEF(0x3E), CB_UNDEF(0x3F),
    // 0x40 — 0x4F
    CB_UNDEF(0x40), CB_UNDEF(0x41), CB_UNDEF(0x42), CB_UNDEF(0x43),
    CB_UNDEF(0x44), CB_UNDEF(0x45), CB_UNDEF(0x46), CB_UNDEF(0x47),
    CB_UNDEF(0x48), CB_UNDEF(0x49), CB_UNDEF(0x4A), CB_UNDEF(0x4B),
    CB_UNDEF(0x4C), CB_UNDEF(0x4D), CB_UNDEF(0x4E), CB_UNDEF(0x4F),
    // 0x50 — 0x5F
    CB_UNDEF(0x50), CB_UNDEF(0x51), CB_UNDEF(0x52), CB_UNDEF(0x53),
    CB_UNDEF(0x54), CB_UNDEF(0x55), CB_UNDEF(0x56), CB_UNDEF(0x57),
    CB_UNDEF(0x58), CB_UNDEF(0x59), CB_UNDEF(0x5A), CB_UNDEF(0x5B),
    CB_UNDEF(0x5C), CB_UNDEF(0x5D), CB_UNDEF(0x5E), CB_UNDEF(0x5F),
    // 0x60 — 0x6F
    CB_UNDEF(0x60), CB_UNDEF(0x61), CB_UNDEF(0x62), CB_UNDEF(0x63),
    CB_UNDEF(0x64), CB_UNDEF(0x65), CB_UNDEF(0x66), CB_UNDEF(0x67),
    CB_UNDEF(0x68), CB_UNDEF(0x69), CB_UNDEF(0x6A), CB_UNDEF(0x6B),
    CB_UNDEF(0x6C), CB_UNDEF(0x6D), CB_UNDEF(0x6E), CB_UNDEF(0x6F),
    // 0x70 — 0x7F
    CB_UNDEF(0x70), CB_UNDEF(0x71), CB_UNDEF(0x72), CB_UNDEF(0x73),
    CB_UNDEF(0x74), CB_UNDEF(0x75), CB_UNDEF(0x76), CB_UNDEF(0x77),
    CB_UNDEF(0x78), CB_UNDEF(0x79), CB_UNDEF(0x7A), CB_UNDEF(0x7B),
    CB_UNDEF(0x7C), CB_UNDEF(0x7D), CB_UNDEF(0x7E), CB_UNDEF(0x7F),
    // 0x80 — 0x8F
    CB_UNDEF(0x80), CB_UNDEF(0x81), CB_UNDEF(0x82), CB_UNDEF(0x83),
    CB_UNDEF(0x84), CB_UNDEF(0x85), CB_UNDEF(0x86), CB_UNDEF(0x87),
    CB_UNDEF(0x88), CB_UNDEF(0x89), CB_UNDEF(0x8A), CB_UNDEF(0x8B),
    CB_UNDEF(0x8C), CB_UNDEF(0x8D), CB_UNDEF(0x8E), CB_UNDEF(0x8F),
    // 0x90 — 0x9F
    CB_UNDEF(0x90), CB_UNDEF(0x91), CB_UNDEF(0x92), CB_UNDEF(0x93),
    CB_UNDEF(0x94), CB_UNDEF(0x95), CB_UNDEF(0x96), CB_UNDEF(0x97),
    CB_UNDEF(0x98), CB_UNDEF(0x99), CB_UNDEF(0x9A), CB_UNDEF(0x9B),
    CB_UNDEF(0x9C), CB_UNDEF(0x9D), CB_UNDEF(0x9E), CB_UNDEF(0x9F),
    // 0xA0 — 0xAF
    CB_UNDEF(0xA0), CB_UNDEF(0xA1), CB_UNDEF(0xA2), CB_UNDEF(0xA3),
    CB_UNDEF(0xA4), CB_UNDEF(0xA5), CB_UNDEF(0xA6), CB_UNDEF(0xA7),
    CB_UNDEF(0xA8), CB_UNDEF(0xA9), CB_UNDEF(0xAA), CB_UNDEF(0xAB),
    CB_UNDEF(0xAC), CB_UNDEF(0xAD), CB_UNDEF(0xAE), CB_UNDEF(0xAF),
    // 0xB0 — 0xBF
    CB_UNDEF(0xB0), CB_UNDEF(0xB1), CB_UNDEF(0xB2), CB_UNDEF(0xB3),
    CB_UNDEF(0xB4), CB_UNDEF(0xB5), CB_UNDEF(0xB6), CB_UNDEF(0xB7),
    CB_UNDEF(0xB8), CB_UNDEF(0xB9), CB_UNDEF(0xBA), CB_UNDEF(0xBB),
    CB_UNDEF(0xBC), CB_UNDEF(0xBD), CB_UNDEF(0xBE), CB_UNDEF(0xBF),
    // 0xC0 — 0xCF
    CB_UNDEF(0xC0), CB_UNDEF(0xC1), CB_UNDEF(0xC2), CB_UNDEF(0xC3),
    CB_UNDEF(0xC4), CB_UNDEF(0xC5), CB_UNDEF(0xC6), CB_UNDEF(0xC7),
    CB_UNDEF(0xC8), CB_UNDEF(0xC9), CB_UNDEF(0xCA), CB_UNDEF(0xCB),
    CB_UNDEF(0xCC), CB_UNDEF(0xCD), CB_UNDEF(0xCE), CB_UNDEF(0xCF),
    // 0xD0 — 0xDF
    CB_UNDEF(0xD0), CB_UNDEF(0xD1), CB_UNDEF(0xD2), CB_UNDEF(0xD3),
    CB_UNDEF(0xD4), CB_UNDEF(0xD5), CB_UNDEF(0xD6), CB_UNDEF(0xD7),
    CB_UNDEF(0xD8), CB_UNDEF(0xD9), CB_UNDEF(0xDA), CB_UNDEF(0xDB),
    CB_UNDEF(0xDC), CB_UNDEF(0xDD), CB_UNDEF(0xDE), CB_UNDEF(0xDF),
    // 0xE0 — 0xEF
    CB_UNDEF(0xE0), CB_UNDEF(0xE1), CB_UNDEF(0xE2), CB_UNDEF(0xE3),
    CB_UNDEF(0xE4), CB_UNDEF(0xE5), CB_UNDEF(0xE6), CB_UNDEF(0xE7),
    CB_UNDEF(0xE8), CB_UNDEF(0xE9), CB_UNDEF(0xEA), CB_UNDEF(0xEB),
    CB_UNDEF(0xEC), CB_UNDEF(0xED), CB_UNDEF(0xEE), CB_UNDEF(0xEF),
    // 0xF0 — 0xFF
    CB_UNDEF(0xF0), CB_UNDEF(0xF1), CB_UNDEF(0xF2), CB_UNDEF(0xF3),
    CB_UNDEF(0xF4), CB_UNDEF(0xF5), CB_UNDEF(0xF6), CB_UNDEF(0xF7),
    CB_UNDEF(0xF8), CB_UNDEF(0xF9), CB_UNDEF(0xFA), CB_UNDEF(0xFB),
    CB_UNDEF(0xFC), CB_UNDEF(0xFD), CB_UNDEF(0xFE), CB_UNDEF(0xFF),
};

#undef CB_UNDEF
