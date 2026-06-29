import re

def main():
    filepath = 'src/opcodes.cpp'
    with open(filepath, 'r') as f:
        content = f.read()

    handlers = "// =============================================================================\n"
    handlers += "//  LOADS 8 BITS ET INSTRUCTIONS AJOUTEES (AUTO-GENERATED)\n"
    handlers += "// =============================================================================\n\n"
    
    registers = ["B", "C", "D", "E", "H", "L", "(HL)", "A"]
    reg_vars = ["b", "c", "d", "e", "h", "l", "hl", "a"]
    
    table_replacements = {}
    
    # 0x40 - 0x7F
    for i in range(64):
        opcode = 0x40 + i
        if opcode == 0x76:
            continue # HALT
        
        dst_idx = i // 8
        src_idx = i % 8
        
        dst_name = registers[dst_idx]
        src_name = registers[src_idx]
        
        dst_var = reg_vars[dst_idx]
        src_var = reg_vars[src_idx]
        
        func_name = f"op_ld_{dst_var}_{src_var}"
        
        cycles = 4
        if src_var == "hl" or dst_var == "hl":
            cycles = 8
            
        handlers += f"int {func_name}(CPU& cpu) {{\n"
        if src_var == "hl":
            handlers += f"    cpu.{dst_var} = cpu.read(cpu.get_hl());\n"
        elif dst_var == "hl":
            handlers += f"    cpu.write(cpu.get_hl(), cpu.{src_var});\n"
        else:
            handlers += f"    cpu.{dst_var} = cpu.{src_var};\n"
        handlers += f"    return {cycles};\n"
        handlers += "}\n\n"
        
        table_replacements[opcode] = f"    /* 0x{opcode:02X} */ {{ \"LD {dst_name}, {src_name}\", {cycles:3},  0, {func_name} }},"

    # Additional loads and instructions we implemented earlier
    extras = [
        (0x02, "LD (BC), A", "cpu.write(cpu.get_bc(), cpu.a);", 8),
        (0x12, "LD (DE), A", "cpu.write(cpu.get_de(), cpu.a);", 8),
        (0x22, "LD (HL+), A", "cpu.write(cpu.get_hl(), cpu.a); cpu.set_hl(cpu.get_hl() + 1);", 8),
        (0x32, "LD (HL-), A", "cpu.write(cpu.get_hl(), cpu.a); cpu.set_hl(cpu.get_hl() - 1);", 8),
        
        (0x0A, "LD A, (BC)", "cpu.a = cpu.read(cpu.get_bc());", 8),
        (0x1A, "LD A, (DE)", "cpu.a = cpu.read(cpu.get_de());", 8),
        (0x2A, "LD A, (HL+)", "cpu.a = cpu.read(cpu.get_hl()); cpu.set_hl(cpu.get_hl() + 1);", 8),
        (0x3A, "LD A, (HL-)", "cpu.a = cpu.read(cpu.get_hl()); cpu.set_hl(cpu.get_hl() - 1);", 8),
        
        (0xE0, "LDH (a8), A", "cpu.write(0xFF00 + cpu.fetch8(), cpu.a);", 12),
        (0xF0, "LDH A, (a8)", "cpu.a = cpu.read(0xFF00 + cpu.fetch8());", 12),
        
        (0xE2, "LD (addr_C), A", "cpu.write(0xFF00 + cpu.c, cpu.a);", 8),
        (0xF2, "LD A, (addr_C)", "cpu.a = cpu.read(0xFF00 + cpu.c);", 8),
        
        (0xEA, "LD (a16), A", "cpu.write(cpu.fetch16(), cpu.a);", 16),
        (0xFA, "LD A, (a16)", "cpu.a = cpu.read(cpu.fetch16());", 16),
        
        (0xF3, "DI", "cpu.ime = false;", 4),
        (0xFB, "EI", "cpu.ime = true;", 4),
        (0x01, "LD BC, d16", "cpu.set_bc(cpu.fetch16());", 12),
        (0x11, "LD DE, d16", "cpu.set_de(cpu.fetch16());", 12),
        (0x21, "LD HL, d16", "cpu.set_hl(cpu.fetch16());", 12),
        (0x31, "LD SP, d16", "cpu.sp = cpu.fetch16();", 12)
    ]
    
    for op, name, body, cyc in extras:
        func_name = "op_" + name.lower().replace("(", "").replace(")", "").replace(",", "").replace("+", "_inc").replace("-", "_dec").replace(" ", "_")
        handlers += f"int {func_name}(CPU& cpu) {{\n"
        handlers += f"    {body}\n"
        handlers += f"    return {cyc};\n"
        handlers += "}\n\n"
        table_replacements[op] = f"    /* 0x{op:02X} */ {{ \"{name}\", {cyc:3},  0, {func_name} }},"

    # Insert handlers before "} // namespace anonyme"
    # Find the namespace ending
    lines = content.split('\n')
    insert_idx = -1
    for i, line in enumerate(lines):
        if line.startswith("} // namespace"):
            insert_idx = i
            break
            
    if insert_idx != -1:
        lines.insert(insert_idx, handlers)
    else:
        print("Warning: Could not find namespace end.")
        
    # Replace table lines
    for i in range(len(lines)):
        line = lines[i]
        m = re.search(r'\/\*\s*0x([0-9A-Fa-f]{2})\s*\*\/', line)
        if m:
            opcode = int(m.group(1), 16)
            if opcode in table_replacements:
                lines[i] = table_replacements[opcode]
                
    with open(filepath, 'w') as f:
        f.write('\n'.join(lines))

if __name__ == '__main__':
    main()
