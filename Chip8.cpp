#include "Chip8.h"
#include <cstring>
#include <random>

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;

const uint8_t FONT_SPRITES[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0 - 0x000
    0x20, 0x60, 0x20, 0x20, 0x70, // 1 - 0x005
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2 - 0x00A
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3 - 0x00F
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4 - 0x014
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5 - 0x019
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6 - 0x01E
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7 - 0x023
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8 - 0x028
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9 - 0x02D
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A - 0x032
    0xe0, 0x90, 0xe0, 0x90, 0xe0, // B - 0x037
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C - 0x03C
    0xe0, 0x90, 0x90, 0x90, 0xe0, // D - 0x041
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E - 0x046
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F - 0x04B
};

uint16_t extractNibbles (uint16_t opcode, uint16_t bitMask = 0xFFFF, int shift = 0){
    return (opcode & bitMask) >> shift;
}

Chip8::Chip8(){
    // set PC to location 512
    PC = 0x200;

    // reset registers, memory, and stack
    I = 0;
    SP = -1;
    delayTimer = 0;
    soundTimer = 0;
    memset(memory, 0, sizeof(memory));
    memset(V, 0, sizeof(V));
    memset(stack, 0, sizeof(stack));

    // fill font data into memory
    for (int i = 0; i < 80; i++){
        memory[i] = FONT_SPRITES[i];
    }

    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    screenSurface = SDL_GetWindowSurface(window);
    SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
}

void Chip8::execute(uint16_t opcode){
    switch (extractNibbles(opcode, 0xFFFF, 12)){ // first 4-bits
        case 0x0000: {
            if (opcode == 0x00e0){ // 00e0
                SDL_FillRect(screenSurface, NULL, SDL_MapRGB(screenSurface->format, 0, 0, 0));
            }
            else if (opcode == 0x00ee){ // 00ee
                PC = stack[SP];
                SP--;
            }
        }

        case 0x0001: { // 1nnn
            uint16_t nnn = extractNibbles(opcode, 0x0FFF);
            PC = nnn;

            break;
        }

        case 0x0002: { // 2nnn
            uint16_t nnn = extractNibbles(opcode, 0x0FFF);
            SP++;
            stack[SP] = PC;
            PC = nnn;

            break;
        }

        case 0x0003: { // 3xkk
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t kk = extractNibbles(opcode, 0x00FF);
            if (V[x] == kk){PC += 2;}

            break;
        }

        case 0x0004: { // 4xkk
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t kk = extractNibbles(opcode, 0x00FF);
            if (V[x] != kk){PC += 2;}

            break;
        }

        case 0x0005: { // 5xy0
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t y = extractNibbles(opcode, 0x00F0, 4);
            if (V[x] == V[y]){PC += 2;}

            break;
        }

        case 0x0006: { // 6xkk
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t kk = extractNibbles(opcode, 0x00FF);
            V[x] = kk;

            break;
        }

        case 0x0007: { // 7xkk
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t kk = extractNibbles(opcode, 0x00FF);
            V[x] += kk;

            break;
        }

        case 0x0008: {
            switch (extractNibbles(opcode, 0x000F)){
                case 0x0000: { // 8xy0
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[x] = V[y];

                    break;
                }

                case 0x0001: { // 8xy1
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[x] |= V[y];
                    
                    break;
                }

                case 0x0002: { // 8xy2
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[x] &= V[y];

                    break;
                }

                case 0x0003: { // 8xy3
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[x] ^= V[y];

                    break;
                }

                case 0x0004: { // 8xy4
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[0xF] = (V[x] + V[y]) > 255;
                    V[x] += V[y];

                    break;
                }

                case 0x0005: { // 8xy5
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[0xF] = V[x] > V[y];
                    V[x] -= V[y];

                    break;
                }

                case 0x0006: { // 8xy6
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    V[0xF] = V[x] & 0x01;
                    V[x] >>= 1;

                    break;
                }

                case 0x0007: { // 8xy7
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[0xF] = V[y] > V[x];
                    V[x] = V[y] - V[x];
                    
                    break;
                }

                case 0x000E: { // 8xyE
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    V[0xF] = V[x] & 0x80;
                    V[x] <<= 1;

                    break;
                }
            }

            break;
        }

        case 0x0009: { // 9xy0
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t y = extractNibbles(opcode, 0x00F0, 4);
            if (V[x] != V[y]){PC += 2;}

            break;
        }

        case 0x000A: { // Annn
            uint16_t nnn = extractNibbles(opcode, 0x0FFF);
            I = nnn;

            break;
        }

        case 0x000B: { // Bnnn
            uint16_t nnn = extractNibbles(opcode, 0x0FFF);
            PC = nnn + V[0];

            break;
        }

        case 0x000C: { // Cxkk
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t kk = extractNibbles(opcode, 0x00FF);
            uint8_t random = rand() % 256;
            V[x] = random & kk;

            break;
        }

        case 0x000D: { // Dxyn
            break;
        }

        case 0x000E: { // EX9E, ExA1
            break;
        }

        case 0x000F: {
            switch (extractNibbles(opcode, 0x00FF)){
                case 0x0007: { // Fx07
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    V[x] = delayTimer;

                    break;
                }

                case 0x000A: { // Fx0A
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    // TODO - WAIT FOR KEY PRESS STORE IN Vx

                    break;
                }

                case 0x0015: { // Fx15
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    delayTimer = V[x];

                    break;
                }

                case 0x0018: { // Fx18
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    soundTimer = V[x];

                    break;
                }

                case 0x001E: { // Fx1E
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    I += V[x];

                    break;
                }

                case 0x0029: { // Fx29
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint16_t locations[] = {0x000, 0x005, 0x00A, 0x00F, 0x014, 0x019, 0x01E, 0x023, 0x028, 0x02D, 0x032, 0x037, 0x03C, 0x041, 0x046, 0x04B};
                    I = locations[V[x]];

                    break;
                }

                case 0x0033: { // Fx33
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    uint8_t number = V[x];
                    memory[I] = (number / 100) % 10;
                    memory[I + 1] = (number / 10) % 10;
                    memory[I + 2] = number % 10;

                    break;
                }

                case 0x0055: { // Fx55
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    for (int i = 0; i <= x; i++){
                        memory[I + i] = V[i];
                    }
                    I += x + 1;  

                    break;
                }

                case 0x0065: { // Fx65
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    for (int i = 0; i <= x; i++){
                        V[i] = memory[I + i];
                    }
                    I += x + 1;

                    break;
                }
            }
        }

            break;
    }
}
