#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <random>

constexpr std::size_t VIDEO_WIDTH = 64;
constexpr std::size_t VIDEO_HEIGHT = 32;
constexpr std::size_t MEMORY_SIZE = 4096;
constexpr std::size_t STACK_SIZE = 16;
constexpr std::size_t REGISTER_COUNT = 16;
constexpr std::size_t KEYPAD_SIZE = 16;
constexpr uint16_t START_ADDRESS = 0x200;
constexpr uint16_t FONTSET_START_ADDRESS = 0x50;
constexpr std::size_t FONTSET_SIZE = 80;

class Chip8
{
public:
	Chip8();

	void Initialise();
	void LoadROM(const char* filename);

	// Executes one CHIP-8 instruction.
	void Cycle();

	// Updates the 60 Hz CHIP-8 timers.
	void UpdateTimers();

	// Access to display/input for the platform layer.
	uint32_t* GetVideo();
	uint8_t* GetKeypad();
private:
	// CPU state
	std::array<uint8_t, REGISTER_COUNT> registers{};
	std::array<uint8_t, MEMORY_SIZE> memory{};

	uint16_t index{};
	uint16_t pc{};

	std::array<uint16_t, STACK_SIZE> stack{};
	uint8_t sp{};

	uint8_t delayTimer{};
	uint8_t soundTimer{};

	std::array<uint8_t, KEYPAD_SIZE> keypad{};

	// 64 x 32 display.
	// Each pixel is either 0x00000000 or 0xFFFFFFFF.
	std::array<uint32_t, VIDEO_WIDTH* VIDEO_HEIGHT> video{};

	uint16_t opcode{};

	// CHIP-8 font data.
	std::array<uint8_t, FONTSET_SIZE> fontset = {
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	// Random number generation for CXKK.
	std::default_random_engine randGen;
	std::uniform_int_distribution<> randByte{ 0, 255 };

	// Function pointer tables for opcode handling.
	using Chip8Func = void (Chip8::*)();

	void Table0();
	void Table8();
	void TableE();
	void TableF();
	void OP_NULL();

	Chip8Func table[0x10]{};
	Chip8Func table0[0xF]{};
	Chip8Func table8[0xF]{};
	Chip8Func tableE[0xF]{};
	Chip8Func tableF[0x66]{};

	// Opcodes
	void OP_00E0(); // Clear the display
	void OP_00EE(); // Return from subroutine (RET)

	void OP_1NNN(); // Jump to address NNN
	void OP_2NNN(); // Call subroutine at NNN
	void OP_3xkk(); // Skip next instruction if Vx == kk
	void OP_4xkk(); // Skip next instruction if Vx != kk
	void OP_5xy0(); // Skip next instruction if Vx == Vy
	void OP_6xkk(); // Set Vx = kk
	void OP_7xkk(); // Set Vx = Vx + kk

	void OP_8xy0(); // Set Vx = Vy
	void OP_8xy1(); // Set Vx = Vx OR Vy
	void OP_8xy2(); // Set Vx = Vx AND Vy
	void OP_8xy3(); // Set Vx = Vx XOR Vy
	void OP_8xy4(); // Set Vx = Vx + Vy, set VF = carry
	void OP_8xy5(); // Set Vx = Vx - Vy, set VF = NOT borrow
	void OP_8xy6(); // Set Vx = Vx SHR 1
	void OP_8xy7(); // Set Vx = Vy - Vx, set VF = NOT borrow
	void OP_8xyE(); // Set Vx = Vx SHL 1

	void OP_9xy0(); // Skip next instruction if Vx != Vy
	void OP_Annn(); // Set I = NNN
	void OP_Bnnn(); // Jump to address NNN + V0
	void OP_Cxkk(); // Set Vx = random byte AND kk
	void OP_Dxyn(); // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
	void OP_Ex9E(); // Skip next instruction if key with the value of Vx is pressed
	void OP_ExA1(); // Skip next instruction if key with the value of Vx is not pressed

	void OP_Fx07(); // Set Vx = delay timer value
	void OP_Fx0A(); // Wait for a key press, store the value of the key in Vx
	void OP_Fx15(); // Set delay timer = Vx
	void OP_Fx18(); // Set sound timer = Vx
	void OP_Fx1E(); // Set I = I + Vx
	void OP_Fx29(); // Set I = location of sprite for digit Vx
	void OP_Fx33(); // Store BCD representation of Vx in memory locations I, I+1, and I+2
	void OP_Fx55(); // Store registers V0 through Vx in memory starting at location I
	void OP_Fx65(); // Read registers V0 through Vx from memory starting at location I
};