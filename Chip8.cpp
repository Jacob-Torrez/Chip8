#include "Chip8.h"
#include <cstring>
#include <iostream>
#include <random>
#include <fstream>
#include <ctime>

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

uint8_t mapKeyPress(SDL_Keycode keycode){
    switch (keycode){
        case SDLK_1: return 0x1;
        case SDLK_2: return 0x2;
        case SDLK_3: return 0x3;
        case SDLK_4: return 0xC;
        case SDLK_q: return 0x4;
        case SDLK_w: return 0x5;
        case SDLK_e: return 0x6;
        case SDLK_r: return 0xD;
        case SDLK_a: return 0x7;
        case SDLK_s: return 0x8;
        case SDLK_d: return 0x9;
        case SDLK_f: return 0xE;
        case SDLK_z: return 0xA;
        case SDLK_x: return 0x0;
        case SDLK_c: return 0xB;
        case SDLK_v: return 0xF;
        default: return -1;
    }
}

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
    memset(display, 0, sizeof(display));
    memset(keys, 0, sizeof(keys));

    // fill font data into memory
    for (int i = 0; i < 80; i++){
        memory[i] = FONT_SPRITES[i];
    }

    quit = false;
    waitingForKey = false;

    std::srand(std::time(nullptr));
}

void Chip8::initializeSDL(){
    window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * 10, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetScale(renderer, SCALE, SCALE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Chip8::run(){
    char input;
    std::cout << "[1] Pong [2] Tetris [3] Exit" << std::endl;
    std::cin >> input;

    std::string fileName;
    switch (input){
        case '1':
            fileName = "PONG";
            break;

        case '2':
            fileName = "TETRIS";
            break;

        default:
            return;
    }

    loadROM(fileName);
    initializeSDL();
    executionLoop();
}

void Chip8::loadROM(std::string fileName){
    std::ifstream file(fileName, std::ios::binary | std::ios::ate);
    
    std::streampos size = file.tellg();
    char* buffer = new char[size];

    file.seekg(0, std::ios::beg);
    file.read(buffer, size);
    file.close();

    for (size_t i = 0; i < size; i++){
        memory[0x200 + i] = buffer[i];
    }

    delete[] buffer;
}

void Chip8::execute(uint16_t opcode){
    switch (extractNibbles(opcode, 0xF000, 12)){ // first 4-bits
        case 0x0000: {
            if (opcode == 0x00e0){ // 00e0
                memset(display, 0, sizeof(display));
            }
            else if (opcode == 0x00ee){ // 00ee
                PC = stack[SP];
                SP--;
            }

            break;
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
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[0xF] = V[y] & 0x01;
                    V[x] = V[y] >> 1;

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
                    uint8_t y = extractNibbles(opcode, 0x00F0, 4);
                    V[0xF] = (V[y] >> 7) & 0x01;
                    V[x] = V[y] << 1;

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
            uint8_t x = extractNibbles(opcode, 0x0F00, 8);
            uint8_t y = extractNibbles(opcode, 0x00F0, 4);
            uint8_t n = extractNibbles(opcode, 0x000F);
            drawSprite(x, y, n);

            break;
        }

        case 0x000E: {
            switch (extractNibbles(opcode, 0x00FF)){
                case 0x009E: { // Ex9E
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    if (keys[V[x]]) {PC += 2;}

                    break;
                }

                case 0x00A1: { // ExA1
                    uint8_t x = extractNibbles(opcode, 0x0F00, 8);
                    if (!keys[V[x]]) {PC += 2;}

                    break;
                }
            }
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
                    waitingForKey = true;
                    keyRegister = x;

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

void Chip8::processEvents(){
    static uint8_t prevKeys[16] = {0};
    SDL_Event e;

    while (SDL_PollEvent(&e)){
        if (e.type == SDL_QUIT){
            quit = true;
        }
        else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP){
            uint8_t key = mapKeyPress(e.key.keysym.sym);
            if (key != 0xff) {
                keys[key] = (e.type == SDL_KEYDOWN) ? 1 : 0;

                if (e.type == SDL_KEYDOWN && !e.key.repeat && prevKeys[key] == 0){
                    if (waitingForKey){
                        V[keyRegister] = key;
                        waitingForKey = false;
                    }
                }

                prevKeys[key] = keys[key];
            }
        }
    }
}

void Chip8::drawSprite(uint8_t Vx, uint8_t Vy, uint8_t n){
    uint8_t x = V[Vx];
    uint8_t y = V[Vy];

    V[0xF] = 0;

    for (int row = 0; row < n; row++){
        
        uint8_t spriteByte = memory[I + row];

        for (int col = 0; col < 8; col++){

            if (spriteByte & (0x80 >> col)){
                int displayX = (x + col) % SCREEN_WIDTH;
                int displayY = (y + row) % SCREEN_HEIGHT;

                int index = displayY * SCREEN_WIDTH + displayX;

                if (display[index] == 1) {V[0xF] = 1;}

                display[index] ^= 1;
            }
        }
    }
}

void Chip8::renderDisplay(){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int y = 0; y < SCREEN_HEIGHT; y++){
        for (int x = 0; x < SCREEN_WIDTH; x++){
            if (display[y * SCREEN_WIDTH + x] == 1){
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

                SDL_Rect pixel;
                pixel.x = x;
                pixel.y = y;
                pixel.w = 1;
                pixel.h = 1;

                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

uint16_t Chip8::fetchOpcode(){
    return (memory[PC] << 8) | memory[PC + 1];
}

void Chip8::executionLoop(){
    Uint32 lastFrameTime = SDL_GetTicks();

    while (!quit){

        processEvents();

        Uint32 currentTime = SDL_GetTicks();
        double elapsed = currentTime - lastFrameTime;

        if (elapsed >= 1000 / FPS){ // ~16ms

            if (!waitingForKey){
                for (int i = 0; i < INSTRUCTIONS_PER_FRAME; i++){
                    uint16_t opcode = fetchOpcode();
                    PC += 2;
                    execute(opcode);
                }
            }

            if (delayTimer > 0) {delayTimer--;}
            if (soundTimer > 0) {soundTimer--;}

            renderDisplay();
            SDL_RenderPresent(renderer);

            lastFrameTime = currentTime;
        } else {
            SDL_Delay(1);
        }
    }
}

Chip8::~Chip8(){
    if (renderer){
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}
