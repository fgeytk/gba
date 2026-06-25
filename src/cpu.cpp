#include "cpu.hpp"
#include "bus.hpp"

// Le constructeur initialise tous les registres à 0 et lie la référence au Bus.
CPU::CPU(Bus& bus)
    : a(0), f(0), b(0), c(0), d(0), e(0), h(0), l(0),
      pc(0), sp(0), bus(bus) {}

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

// --- Exécution d'instructions ---
void CPU::step() {
    // Lire l'opcode à l'adresse du PC
    uint8_t opcode = read(pc);

    // Incrémenter le PC pour pointer vers l'instruction suivante
    pc++;
    execute(opcode);
    return;
}

void CPU::execute(uint8_t opcode) {
    //futur switch pour les opcodes 

    switch (opcode) {
        case 0x00: // NOP
            break;

        case 0x06: // LD B, d8
            b = read(pc++);
            break;

        case 0x0E: // LD C, d8
            c = read(pc++);
            break;

        case 0x16: // LD D, d8
            d = read(pc++);
            break;

        case 0x1E: // LD E, d8
            e = read(pc++);
            break;

        case 0x26: // LD H, d8
            h = read(pc++);
            break;

        case 0x2E: // LD L, d8
            l = read(pc++);
            break;

        case 0x36: // LD (HL), d8
            // Attention ici ! On écrit la donnée lue en mémoire à l'adresse contenue dans HL
            write(get_hl(), read(pc++));
            break;

        case 0x3E: // LD A, d8
            a = read(pc++);
            break;

        default:
            // Pour l'instant, si on tombe sur un opcode inconnu, on ne fait rien
            break;
    }
}