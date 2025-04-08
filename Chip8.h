#pragma once
#include <cstdint>
#include <SDL2/SDL.h>

class Chip8 {
private:
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I, PC;
    uint8_t delayTimer, soundTimer, SP;
    uint16_t stack[16];

    SDL_Window* window = nullptr;
    SDL_Surface* screenSurface = nullptr;

public:
    Chip8();
    void execute(uint16_t opcode);


};