#include "../include/cpu_bus.h"
#include "../include/mappers/nrom_0.h"

void CPU_Bus::write(uint16_t addr, uint8_t data) {
    if (0 <= addr && addr <= 0x1fff)
        ram[addr % 0x800] = data;
    else if (0x2000 <= addr && addr <= 0x3fff)
        return; 
    else if (0x4000 <= addr && addr <= 0x4017)
        return; 
    else if (0x4018 <= addr && addr <= 0x401f)
        return; 
    else if (0x4020 <= addr && addr <= 0xffff)
        mapper->cpu_write(addr, data);
}

uint8_t CPU_Bus::read(uint16_t addr) {
    if (0 <= addr && addr <= 0x1fff)
        return ram[addr % 0x800];
    else if (0x2000 <= addr && addr <= 0x3fff)
        return 0; 
    else if (0x4000 <= addr && addr <= 0x4017)
        return 0; 
    else if (0x4018 <= addr && addr <= 0x401f)
        return 0;  
    else if (0x4020 <= addr && addr <= 0xffff)
        return mapper->cpu_read(addr);

    return 0;
}

/*
void CPU_Bus::insert_cart(char rom_path[]) {
    Cartridge* cart = (Cartridge*)malloc(sizeof(Cartridge));
    uint8_t mapper_id = load_cart_data(cart, rom_path);

    switch(mapper_id){
        case 0:
            mapper = new NROM_0(cart);
            break;
        default:
            printf("Mapper not implemented\n");
            exit(1);
    }
}

uint8_t load_cart_data(Cartridge* cart, char rom_path[]){
    int val = 0;
    FILE* rom_file = fopen(rom_path, "rb");
    
    fseek(rom_file, 0, SEEK_END);
    long fileSize = ftell(rom_file);
    fseek(rom_file, 5, SEEK_CUR);

    fread(&cart->header.prg_bank_size, 1, 1, rom_file);
    fread(&cart->header.chr_bank_size, 1, 1, rom_file);
    fread(&cart->header.flags_6, 1, 1, rom_file);
    fread(&cart->header.flags_7, 1, 1, rom_file);
    fread(&cart->header.flags_8, 1, 1, rom_file);
    fread(&cart->header.flags_9, 1, 1, rom_file);
    fread(&cart->header.flags_10, 1, 1, rom_file);
    
    if (cart->header.flags_6 & 0b00000100)
        val = 512;
    
    cart->prg_rom.resize(cart->header.prg_bank_size * 16 * 1024);
    cart->prg_rom.resize(cart->header.prg_bank_size * 8 * 1024);

    fseek(rom_file, 16 + val, SEEK_SET);
    fread(&cart->prg_rom, 1, cart->header.prg_bank_size * 16 * 1024, rom_file);
    fread(&cart->chr_rom, 1, cart->header.chr_bank_size * 8 * 1024, rom_file);

    if (cart->header.flags_10 & 0b00010000 == 0)
        cart->prg_ram.resize(0);
    else
        cart->prg_ram.resize(cart->header.flags_8 * 8 * 1024);
    
    return (cart->header.flags_6 & 0xf0) | (cart->header.flags_7 >> 4);
}
*/