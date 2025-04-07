#include "Chip8.h"
#include <cstring>

Chip8::Chip8(){
    uint8_t fontSprites[80] = {
        0xf0, 0x90, 0x90, 0x90, 0xf0, // 0 - 0x000
        0x20, 0x60, 0x20, 0x20, 0x70, // 1 - 0x005
        0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2 - 0x00A
        0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3 - 0x00F
        0x90, 0x90, 0xf0, 0x10, 0x10, // 4 - 0x014
        0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5 - 0x019
        0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6 - 0x01E
        0xf0, 0x10, 0x20, 0x40, 0x40, // 7 - 0x023
        0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8 - 0x028
        0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9 - 0x02D
        0xf0, 0x90, 0xf0, 0x90, 0x90, // A - 0x032
        0xe0, 0x90, 0xe0, 0x90, 0xe0, // B - 0x037
        0xf0, 0x80, 0x80, 0x80, 0xf0, // C - 0x03C
        0xe0, 0x90, 0x90, 0x90, 0xe0, // D - 0x041
        0xf0, 0x80, 0xf0, 0x80, 0xf0, // E - 0x046
        0xf0, 0x80, 0xf0, 0x80, 0x80  // F - 0x04B
    };

    // set PC to location 512
    PC = 0x200;

    // reset registers, memory, and stack
    I = 0;
    SP = 0;
    delayTimer = 0;
    soundTimer = 0;
    memset(memory, 0, sizeof(memory));
    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));

    // fill font data into memory
    for (int i = 0; i < 80; i++){
        memory[i] = fontSprites[i];
    }
}

void Chip8::execute(uint16_t opcode){
    switch (opcode) {
        case 0x00E0: // CLS - clear the display
            break;

        case 0x00EE: // RET - return from subroutine
            break;

        case 0x0000: //0x1nnn: // JP addr - jump to location nnn
            break;

        case 0x0001: //0x2nnn: // CALL addr - call subroutine at nnn
            break;

        case 0x0003: //0x3xkk: // SE Vx, byte - skip next instruction if Vx = kk
            break;

        case 0x0004: //0x4xkk: // SNE Vx, byte - skip next instruction if Vx != kk
            break;

        case 0x0005: //0x5xy0 // SE Vx, Vy - skip next instruction if Vx = Vy
            break;

        case 0x0006: //0x6xkk // LD Vx, byte - set Vx = kk
            break;

        case 0x0007: //0x7xkk // ADD Vx, byte - sest Vx = Vx + kk
            break;

        case 0x0008: //0x8xy0 // LD Vx, Vy - set Vx = Vy
            break;

        case 0x0018: //0x8xy1 // OR Vx, Vy - set Vx = Vx OR Vy
            break;

        case 0x0028: //0x8xy1 // AND Vx, Vy - set Vx = Vx AND Vy
            break;

        case 0x0038: //0x8xy3 // XOR Vx, Vy - set Vx = Vx XOR Vy
            break;

        case 0x0048: //0x8xy4 // ADD Vx, Vy - set Vx = Vx + Vy, set VF = carry
            break;

        case 0x0058: //0x8xy5 // SUB Vx, Vy - set Vx = Vx - Vy, set VF = NOT borrow
            break;

        case 0x0068: //0x8xy6 // SHR Vx {, Vy} - set Vx = Vx SHR 1
            break;

        case 0x0078: //0x8xy7 // SUBN Vx, Vy - set Vx = Vy - Vx, set VF = NOT borrow
            break;

        case 0x00E8: //0x8xyE // SHL Vx {, Vy} - set Vx = Vx SHL 1
            break;

        case 0x0009: //0x9xy0 // SNE Vx, Vy - skip next instruction if Vx != Vy
            break;

        case 0x000A: //0xAnnn // LD I, addr - set I = nnn
            break;

        case 0x000B: //0xBnnn // JP V0, addr - jump to location nnn + V0
            break;

        case 0x000C: //0xCxkk // RND Vx, byte - set Vx = random byte AND kk
            break;

        case 0x000D: //0xDxyn // DRW Vx, Vy, nibble - dispay n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
            break;

        case 0x000E: //0xEx9E // SKIP Vx - skip next instruction if key with the value of Vx is pressed
            break;

        case 0x001E: //0xExA1 // SKNP Vx - skip next instruction if key with the value of Vx is not pressed
            break;

        case 0x000F: //0xFx07 // LD Vx, DT - set Vx = delay timer value
            break;

        case 0x001F: //0xFx0A // LD Vx, K - wait for a key press, store the value of the key in Vx
            break;

        case 0x002F: //0xFx15 // LD DT, Vx - set delay timer = Vx
            break;

        case 0x003F: //0xFx18 // LD ST, Vx - set sound timer = Vx
            break;

        case 0x004F: //0xFx1E // ADD I, Vx - set I = I + Vx
            break;

        case 0x005F: //0xFx29 // LD F, Vx - set I = location of sprite for digit Vx
            break;

        case 0x006F: //0xFx33 // LD B, Vx - store BCD representation of Vx in memory location I, I+1, and I+2
            break;

        case 0x007F: //0xFx55 // LD [I], Vx - store registers V0 through Vx in memory starting at location I
            break;

        case 0x008F: //0xFx65 // LD Vx, [I] - read registers V0 through Vx from memory starting at location I
            break;
    }
}