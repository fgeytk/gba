#include "cpu.hpp"
#include "bus.hpp"

// Le constructeur initialise les registres avec les valeurs exactes
// laissées par le Boot ROM (le BIOS Game Boy) avant de donner la main à la cartouche.
// Notamment le PC commence à 0x0100 (point d'entrée du jeu) et le SP à 0xFFFE.
CPU::CPU(Bus& bus)
    : a(0x01), f(0xB0), b(0x00), c(0x13), d(0x00), e(0xD8), h(0x01), l(0x4D),
      pc(0x0100), sp(0xFFFE), bus(bus) {}

// --- Paire AF ---
uint16_t CPU::get_af() const {
    return (a << 8) | f;
}

void CPU::set_af(uint16_t value) {
    a = (value >> 8) & 0xFF;
    f = value & 0xF0; // Règle importante : les 4 bits de poids faible de F restent à 0
}

// --- Paire BC ---
uint16_t CPU::get_bc() const {
    return (b << 8) | c;
}

void CPU::set_bc(uint16_t value) {
    b = (value >> 8) & 0xFF;
    c = value & 0xFF;
}

// --- Paire DE ---
uint16_t CPU::get_de() const {
    return (d << 8) | e;
}

void CPU::set_de(uint16_t value) {
    d = (value >> 8) & 0xFF;
    e = value & 0xFF;
}

// --- Paire HL ---
uint16_t CPU::get_hl() const {
    return (h << 8) | l;
}

void CPU::set_hl(uint16_t value) {
    h = (value >> 8) & 0xFF;
    l = value & 0xFF;
}

// --- Flags ---
void CPU::set_flag_z(bool value) { if (value) f |= 0x80; else f &= ~0x80; }
void CPU::set_flag_n(bool value) { if (value) f |= 0x40; else f &= ~0x40; }
void CPU::set_flag_h(bool value) { if (value) f |= 0x20; else f &= ~0x20; }
void CPU::set_flag_c(bool value) { if (value) f |= 0x10; else f &= ~0x10; }

bool CPU::get_flag_z() const { return (f & 0x80) != 0; }
bool CPU::get_flag_n() const { return (f & 0x40) != 0; }
bool CPU::get_flag_h() const { return (f & 0x20) != 0; }
bool CPU::get_flag_c() const { return (f & 0x10) != 0; }

// --- Helpers d'instructions ---
// addition 8 bits avec mise à jour des flags Z, N, H, C
//
void CPU::add_a(uint8_t value) {
    uint8_t a_val = a;
    set_flag_n(false); 
    set_flag_h( ((a_val & 0x0F) + (value & 0x0F)) > 0x0F );
    set_flag_c( ((uint16_t)a_val + value) > 0xFF );
    
    a = a_val + value;
    
    set_flag_z(a == 0);
}

void CPU::push_16(uint16_t value) {
    sp--;
    write(sp, (value >> 8) & 0xFF); // Octet de poids fort d'abord
    sp--;
    write(sp, value & 0xFF);        // Octet de poids faible ensuite
}

uint16_t CPU::pop_16() {
    uint8_t low = read(sp++);
    uint8_t high = read(sp++);
    return (high << 8) | low;
}

// --- Accès mémoire ---
uint8_t CPU::read(uint16_t addr) const {
    return bus.read(addr);
}

void CPU::write(uint16_t addr, uint8_t value) {
    bus.write(addr, value);
}

uint16_t CPU::read_16(uint16_t addr) const {
    uint8_t low = read(addr);
    uint8_t high = read(addr + 1);
    return (high << 8) | low;
}

void CPU::write_16(uint16_t addr, uint16_t value) {
    write(addr, value & 0xFF);         // Octet faible à addr
    write(addr + 1, (value >> 8) & 0xFF); // Octet fort à addr + 1
}

// --- Helpers de fetch ---
// Utilisés par les handlers d'opcodes dans opcodes.cpp
uint8_t CPU::fetch8() {
    return read(pc++);
}

uint16_t CPU::fetch16() {
    uint16_t val = read_16(pc);
    pc += 2;
    return val;
}

// --- Exécution d'instructions ---
// Le dispatch se fait via les tables OPCODES[] et CB_OPCODES[] définies dans opcodes.cpp.
// Chaque handler reçoit une référence au CPU et retourne le nombre de cycles consommés.
#include "opcodes.hpp"

int CPU::step() {
    // Lire l'opcode à l'adresse du PC
    uint8_t opcode = fetch8();

    // Gestion du préfixe CB : redirige vers la table d'opcodes CB-prefixed
    if (opcode == 0xCB) {
        uint8_t cb_opcode = fetch8();
        const Instruction& instr = CB_OPCODES[cb_opcode];
        return instr.execute(*this);
    }

    // Dispatch via la table principale
    const Instruction& instr = OPCODES[opcode];
    return instr.execute(*this);
}