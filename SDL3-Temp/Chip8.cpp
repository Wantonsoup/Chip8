#include "Chip8.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <stdexcept>
#include <vector>

Chip8::Chip8()
{
	Initialise();
}

void Chip8::Initialise()
{
	// reset CPU state.
	registers.fill(0);
	memory.fill(0);
	stack.fill(0);
	keypad.fill(0);
	video.fill(0);

	index = 0;
	pc = START_ADDRESS;
	sp = 0;

	delayTimer = 0;
	soundTimer = 0;
	opcode = 0;

	// load fontset into memory.
	for (std::size_t i = 0; i < FONTSET_SIZE; ++i)
	{
		memory[FONTSET_START_ADDRESS + i] = fontset[i];
	}

	// seed random number generator.
	
	randGen.seed(
		static_cast<unsigned int>(
			std::chrono::high_resolution_clock::now()
			.time_since_epoch()
			.count()
			)
	);

	// Initialise all opcode tables to invalid instructions.
	// std::fill better rather than if statements. It is more efficient and easier to read.
	// std::fill creates array and points to OP_Null. When invalid opcode is called, OP_Null will be executed.
	std::fill(std::begin(table), std::end(table), &Chip8::OP_NULL);
	std::fill(std::begin(table0), std::end(table0), &Chip8::OP_NULL);
	std::fill(std::begin(table8), std::end(table8), &Chip8::OP_NULL);
	std::fill(std::begin(tableE), std::end(tableE), &Chip8::OP_NULL);
	std::fill(std::begin(tableF), std::end(tableF), &Chip8::OP_NULL);

	// 0xX table
	table[0x0] = &Chip8::Table0;
	table[0x1] = &Chip8::OP_1NNN;
	table[0x2] = &Chip8::OP_2NNN;
	table[0x3] = &Chip8::OP_3xkk;
	table[0x4] = &Chip8::OP_4xkk;
	table[0x5] = &Chip8::OP_5xy0;
	table[0x6] = &Chip8::OP_6xkk;
	table[0x7] = &Chip8::OP_7xkk;
	table[0x8] = &Chip8::Table8;
	table[0x9] = &Chip8::OP_9xy0;
	table[0xA] = &Chip8::OP_Annn;
	table[0xB] = &Chip8::OP_Bnnn;
	table[0xC] = &Chip8::OP_Cxkk;
	table[0xD] = &Chip8::OP_Dxyn;
	table[0xE] = &Chip8::TableE;
	table[0xF] = &Chip8::TableF;

	// 0x00 table0
	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	// 0x8XY table8
	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	// 0xEX tableE
	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	// 0xFX tableF
	tableF[0x07] = &Chip8::OP_Fx07;
	tableF[0x0A] = &Chip8::OP_Fx0A;
	tableF[0x15] = &Chip8::OP_Fx15;
	tableF[0x18] = &Chip8::OP_Fx18;
	tableF[0x1E] = &Chip8::OP_Fx1E;
	tableF[0x29] = &Chip8::OP_Fx29;
	tableF[0x33] = &Chip8::OP_Fx33;
	tableF[0x55] = &Chip8::OP_Fx55;
	tableF[0x65] = &Chip8::OP_Fx65;
}

void Chip8::LoadROM(const char* filename)
{
	// Open the file as a stream of binary and move the file pointer to the end
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	
	// check if file open successfully
	if (!file)
	{
		throw std::runtime_error("Failed to open ROM: " + std::string(filename));
	}

	// get the size of the file by checking the current position of the file pointer (which is at the end)
	const std::streamsize size = file.tellg();

	if (size < 0)
	{
		throw std::runtime_error("Failed to determine ROM size.");
	}

	// Calculate the available memory for the ROM, which is the total memory size - the start address where the ROM will be loaded
	constexpr std::size_t availableMemory = MEMORY_SIZE - START_ADDRESS;

	if (static_cast<std::size_t>(size) > availableMemory)
	{
		throw std::runtime_error("ROM is too large to fit in CHIP-8 memory.");
	}

	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(static_cast<std::size_t>(size));

	if (!file.read(buffer.data(), size))
	{
		throw std::runtime_error("Failed to read ROM.");
	}

	for (std::size_t i = 0; i < buffer.size(); ++i)
	{
		memory[START_ADDRESS + i] =
			static_cast<uint8_t>(buffer[i]);
	}
}

void Chip8::Cycle()
{
	if (pc >= MEMORY_SIZE - 1)
	{
		throw std::runtime_error("PC out of bounds");
	}
	// Fetch
	opcode =
		static_cast<uint16_t>(memory[pc]) << 8u |
		static_cast<uint16_t>(memory[pc + 1]);
	pc += 2;

	// Decode and Execute
	const uint8_t instruction =
		static_cast<uint8_t>((opcode & 0xF000u) >> 12u);
	
	// ((*this).* 
	((*this).*(table[instruction]))();
}

void Chip8::UpdateTimers()
{
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	if (soundTimer > 0)
	{
		--soundTimer;
	}
}

uint32_t* Chip8::GetVideo()
{
	return video.data();
}

uint8_t* Chip8::GetKeypad()
{
	return keypad.data();
}

// Opcode tables

void Chip8::Table0()
{
	const uint8_t instruction =
		static_cast<uint8_t>(opcode & 0x000Fu);

	((*this).*(table0[instruction]))();
}

void Chip8::Table8()
{
	const uint8_t instruction =
		static_cast<uint8_t>(opcode & 0x000Fu);

	((*this).*(table8[instruction]))();
}

void Chip8::TableE()
{
	const uint8_t instruction =
		static_cast<uint8_t>(opcode & 0x000Fu);

	((*this).*(tableE[instruction]))();
}

void Chip8::TableF()
{
	const uint8_t instruction =
		static_cast<uint8_t>(opcode & 0x00FFu);

	((*this).*(tableF[instruction]))();
}

void Chip8::OP_NULL()
{
	// Invalid or unsupported opcode.
}

// Opcodes

void Chip8::OP_00E0()
{
	video.fill(0);
}

void Chip8::OP_00EE()
{
	--sp;
	pc = stack[sp];
}

void Chip8::OP_1NNN()
{
	uint16_t address = static_cast<uint16_t>(opcode & 0x0FFFu);

	pc = address;
}

void Chip8::OP_2NNN()
{
	uint16_t address = static_cast<uint16_t>(opcode & 0x0FFFu);

	stack[sp] = pc;
	++sp;
	pc = address;
}

void Chip8::OP_3xkk()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t byte = static_cast<uint8_t>(opcode & 0x00FFu);

	if (registers[Vx] == byte)
	{
		pc += 2;
	}
}

void Chip8::OP_4xkk()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t byte = static_cast<uint8_t>(opcode & 0x00FFu);

	if (registers[Vx] != byte)
	{
		pc += 2;
	}
}

void Chip8::OP_5xy0()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	if (registers[Vx] == registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_6xkk()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t byte = static_cast<uint8_t>(opcode & 0x00FFu);

	registers[Vx] = byte;
}

void Chip8::OP_7xkk()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t byte = static_cast<uint8_t>(opcode & 0x00FFu);

	registers[Vx] += byte;
}

void Chip8::OP_8xy0()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	registers[Vx] = registers[Vy];
}

void Chip8::OP_8xy1()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	registers[Vx] |= registers[Vy];
}

void Chip8::OP_8xy2()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	registers[Vx] &= registers[Vy];
}

void Chip8::OP_8xy3()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	registers[Vx] ^= registers[Vy];
}

void Chip8::OP_8xy4()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	uint16_t sum = registers[Vx] + registers[Vy];

	if (sum > 255U)
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = sum & 0xFFu;
}

void Chip8::OP_8xy5()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] -= registers[Vy];
}

void Chip8::OP_8xy6()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	// Save LSB in VF
	registers[0xF] = (registers[Vx] & 0x1u);

	registers[Vx] >>= 1;
}

void Chip8::OP_8xy7()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[Vx] = registers[Vy] - registers[Vx];
}

void Chip8::OP_8xyE()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	// Save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

	registers[Vx] <<= 1;
}

void Chip8::OP_9xy0()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);

	if (registers[Vx] != registers[Vy])
	{
		pc += 2;
	}
}

void Chip8::OP_Annn()
{
	uint16_t address = static_cast<uint16_t>(opcode & 0x0FFFu);

	index = address;
}

void Chip8::OP_Bnnn()
{
	uint16_t address = static_cast<uint16_t>(opcode & 0x0FFFu);

	pc = registers[0] + address;
}

void Chip8::OP_Cxkk()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t byte = static_cast<uint8_t>(opcode & 0x00FFu);

	registers[Vx] = randByte(randGen) & byte;
}

void Chip8::OP_Dxyn()
{

	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t Vy = static_cast<uint8_t>((opcode & 0x00F0u) >> 4u);
	uint8_t height = static_cast<uint8_t>(opcode & 0x000Fu);

	// Wrap if going beyond screen boundaries
	uint8_t xPos = static_cast<uint8_t>(registers[Vx] % VIDEO_WIDTH);
	uint8_t yPos = static_cast<uint8_t>(registers[Vy] % VIDEO_HEIGHT);

	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row)
	{
		// If row is bigger than screen height, break loop. 
		if(yPos + row >= VIDEO_HEIGHT)
		{
			break;
		}
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col)
		{
			// If column is bigger than screen width, break loop.
			if (xPos + col >= VIDEO_WIDTH)
			{
				break;
			}

			uint8_t spritePixel = static_cast<uint8_t>(spriteByte & (0x80u >> col));
			uint32_t* screenPixel = static_cast<uint32_t*>(&video[(yPos + row) * VIDEO_WIDTH + (xPos + col)]);

			// Sprite pixel is on
			if (spritePixel)
			{
				// Screen pixel also on - collision
				if (*screenPixel == 0xFFFFFFFF)
				{
					registers[0xF] = 1;
				}

				// Effectively XOR with the sprite pixel
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

void Chip8::OP_Ex9E()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	uint8_t key = static_cast<uint8_t>(registers[Vx]);

	if (keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_ExA1()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	uint8_t key = static_cast<uint8_t>(registers[Vx]);

	if (!keypad[key])
	{
		pc += 2;
	}
}

void Chip8::OP_Fx07()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	registers[Vx] = delayTimer;
}

void Chip8::OP_Fx0A()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	if (keypad[0])
	{
		registers[Vx] = 0;
	}
	else if (keypad[1])
	{
		registers[Vx] = 1;
	}
	else if (keypad[2])
	{
		registers[Vx] = 2;
	}
	else if (keypad[3])
	{
		registers[Vx] = 3;
	}
	else if (keypad[4])
	{
		registers[Vx] = 4;
	}
	else if (keypad[5])
	{
		registers[Vx] = 5;
	}
	else if (keypad[6])
	{
		registers[Vx] = 6;
	}
	else if (keypad[7])
	{
		registers[Vx] = 7;
	}
	else if (keypad[8])
	{
		registers[Vx] = 8;
	}
	else if (keypad[9])
	{
		registers[Vx] = 9;
	}
	else if (keypad[10])
	{
		registers[Vx] = 10;
	}
	else if (keypad[11])
	{
		registers[Vx] = 11;
	}
	else if (keypad[12])
	{
		registers[Vx] = 12;
	}
	else if (keypad[13])
	{
		registers[Vx] = 13;
	}
	else if (keypad[14])
	{
		registers[Vx] = 14;
	}
	else if (keypad[15])
	{
		registers[Vx] = 15;
	}
	else
	{
		pc -= 2;
	}
}

void Chip8::OP_Fx15()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	delayTimer = registers[Vx];
}

void Chip8::OP_Fx18()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	soundTimer = registers[Vx];
}

void Chip8::OP_Fx1E()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	index += registers[Vx];
}

void Chip8::OP_Fx29()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t digit = static_cast<uint8_t>(registers[Vx]);

	index = FONTSET_START_ADDRESS + (5 * digit);
}

void Chip8::OP_Fx33()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);
	uint8_t value = static_cast<uint8_t>(registers[Vx]);
	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}

void Chip8::OP_Fx55()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

void Chip8::OP_Fx65()
{
	uint8_t Vx = static_cast<uint8_t>((opcode & 0x0F00u) >> 8u);

	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}
