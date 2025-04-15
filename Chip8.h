#pragma once
#include <cstdint>
#include <string>
#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int SCALE = 10;
const int IPS = 600;
const int FPS = 60;
const int INSTRUCTIONS_PER_FRAME = IPS / FPS;

class Chip8 {
private:
    uint8_t memory[4096];
    uint8_t V[16];
    uint16_t I, PC;
    uint8_t delayTimer, soundTimer, SP;
    uint16_t stack[16];
    uint8_t keys[16];

    bool quit;
    bool waitingForKey;
    uint8_t keyRegister;

    bool display[SCREEN_WIDTH * SCREEN_HEIGHT];

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

public:
    Chip8();
    ~Chip8();
    void initializeSDL();

    void run();

    void loadROM(std::string fileName);
    void execute(uint16_t opcode);
    void executionLoop();
    uint16_t fetchOpcode();

    void drawSprite(uint8_t Vx, uint8_t Vy, uint8_t n);
    void renderDisplay();
    void processEvents();
};