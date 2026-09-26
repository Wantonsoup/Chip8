#pragma once
#include <cstdint>

class Chip8
{
public:
	Chip8();
	void Initialise();
	void LoadROM(const char* filename);
private:
	uint8_t registers[16]{}; // V0-VF
	uint8_t memory[4096]{}; // 4K memory
	uint16_t index{}; // I register
	uint16_t pc{}; // Program Counter
	uint16_t stack[16]{}; // Stack for subroutine calls
	uint8_t sp{}; // Stack Pointer
	uint8_t delayTimer{};
	uint8_t soundTimer{};
	uint8_t keypad[16]{}; // 16-key keypad state
	uint32_t video[64 * 32]{};
	uint16_t opcode{};
};

