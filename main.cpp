#include "Chip8.h"

int main(int argc, char* args[]){

    Chip8 chip;
    chip.execute(0x8016);

    return 0;
}