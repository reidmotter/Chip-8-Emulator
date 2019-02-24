#pragma warning(disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"

FILE * myfile;

unsigned char fontset[80] =
{
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

chip8::chip8()
{
	// empty
}

chip8::~chip8()
{
	// empty
}

void chip8::initialize() {
	// initialize registers and memory
	PC = 0x200;
	opcode = 0;
	I = 0;
	SP = 0;

	for (int i = 0; i < 2048; i++) {
		memory[i] = 0;
		graphics[i] = 0;
	}

	for (int i = 2048; i < 4096; i++)
		memory[i] = 0;

	for (int i = 0; i < 16; i++) {
		V[i] = 0;
		stack[i] = 0;
		keypad[i] = 0;
	}

	for (int i = 0; i < 80; i++)
		memory[i] = fontset[i];

	delay_timer = 0;
	sound_timer = 0;

	drawFlag = true;

	//myfile = fopen("opcodes.txt", "w");
	printf("Initialize in chip8.cpp\n");
}

bool chip8::loadApplication(const char * filename)
{
	initialize();
	printf("Loading: %s\n", filename);

	// Open file
	FILE * rom = fopen(filename, "rb");
	if (rom == NULL)
	{
		fputs("ROM error", stderr);
		return false;
	}

	// Check file size
	fseek(rom, 0, SEEK_END);
	long romSize = ftell(rom);
	rewind(rom);
	printf("Filesize: %d\n", (int)romSize);

	// Allocate memory to contain the whole file
	char * buffer = (char*)malloc(sizeof(char) * romSize);
	if (buffer == NULL)
	{
		fputs("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread(buffer, 1, romSize, rom);
	if (result != romSize)
	{
		fputs("Reading error", stderr);
		return false;
	}

	// Copy buffer to Chip8 memory
	if ((4096 - 512) > romSize)
	{
		for (int i = 0; i < romSize; i++)
			memory[i + 512] = buffer[i];
	}

	else {
		printf("Error: ROM too big for memory");
		return false;
	}

	// Close file, free buffer
	fclose(rom);
	free(buffer);
	printf("Load Application Succesfull\n");
	return true;
}

void chip8::emulateCycle() {
	// fetch
	opcode = (memory[PC] << 8) | memory[PC+1];
	//fprintf(myfile, "Opcode: %X\n", opcode);
	// decode
	switch (opcode & 0xF000) {

		case 0x0000: opcode0(opcode);
		break;
		case 0x1000: opcode1(opcode);
		break;
		case 0x2000: opcode2(opcode);
		break;
		case 0x3000: opcode3(opcode);
		break;
		case 0x4000: opcode4(opcode);
		break;
		case 0x5000: opcode5(opcode);
		break;
		case 0x6000: opcode6(opcode);
		break;
		case 0x7000: opcode7(opcode);
		break;
		case 0x8000: opcode8(opcode);
		break;
		case 0x9000: opcode9(opcode);
		break;
		case 0xA000: opcodeA(opcode);
		break;
		case 0xB000: opcodeB(opcode);
		break;
		case 0xC000: opcodeC(opcode);
		break;
		case 0xD000: opcodeD(opcode);
		break;
		case 0xE000: opcodeE(opcode);
		break;
		case 0xF000: opcodeF(opcode);
		break;
		default: 
			printf("Invalid Opcode : %X\n", opcode);
			fprintf(myfile, "Invalid Opcode: %X\n", opcode);
		break;
	}

	// update timers
	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0) {
		if (sound_timer == 1)
			//printf("BEEP!\n");
		--sound_timer;
	}
}

void chip8::opcode0(unsigned short opcode)
{
	switch (opcode & 0x000F) {
		case 0x0000:						// 0x00E0 clears the screen
			for (int i = 0; i < 2048; i++)
				graphics[i] = 0;
			PC += 2;
			drawFlag = true;
		break;

		case 0x000E:						// 0x00EE returns from a subroutine
			--SP;
			PC = stack[SP];
			PC += 2;
		break;

		default:
			fprintf(myfile, "Invalid Opcode: %X\n", opcode);
			printf("Invalid Opcode : %X\n", opcode);
	}
}
/**
 * Handles all the 0x1*** opcodes
 *
 * 0x1NNN : jumps to address NNN
 *
 */
void chip8::opcode1(unsigned short opcode)
{
	PC = opcode & 0x0FFF;
}

/**
 * Handles all the 0x2*** opcodes
 *
 * 0x2NNN : calls a subroutine at NNN
 */
void chip8::opcode2(unsigned short opcode)
{
	stack[SP] = PC;
	SP++;
	PC = opcode & 0x0FFF;
}

/**
 * Handles all the 0x3*** opcodes
 *
 * 0x3XNN : skips the next instr. if the value in register X is equal to NN
 */
void chip8::opcode3(unsigned short opcode)
{
	if ((opcode & 0x00FF) == V[(opcode & 0x0F00) >> 8])
		PC += 4;
	else
		PC += 2;
}

/**
 * Handles all the 0x4*** opcodes
 *
 * 0x4XNN : skips the next instr. if the value in register X is not equal to NN
 */
void chip8::opcode4(unsigned short opcode)
{
	if ((opcode & 0x00FF) != V[(opcode & 0x0F00) >> 8])
		PC += 4;
	else
		PC += 2;
}

/**
 * Handles all the 0x5*** opcodes
 *
 * 0x5XY0 : skips the next instr. if the values in registers X and Y are equal
 */
void chip8::opcode5(unsigned short opcode)
{
	if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
		PC += 4;
	else
		PC += 2;
}

/**
 * Handles all the 0x6*** opcodes
 *
 * 0x6XNN : set V[X] to NN
 */
void chip8::opcode6(unsigned short opcode)
{
	V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
	PC += 2;
}

/**
 * Handles all the 0x7*** opcodes
 *
 * 0x7XNN : V[X] += NN
 */
void chip8::opcode7(unsigned short opcode)
{
	V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
	PC += 2;
}

/**
 * Handles all the 0x8*** opcodes
 *
 * 0x8XY0 : sets V[X] to value to V[Y]
 * 0x8XY1 : sets V[X] to value of V[X] OR V[Y]
 * 0x8XY2 : sets V[X] to value of V[X] AND V[Y]
 * 0x8XY3 : sets V[X] to value of V[X] XOR V[Y]
 * 0x8XY4 : sets V[X] to value of V[X] + V[Y], set V[F] to 1 if there's carry
 * 0x8XY5 : sets V[X] to value of V[X] - V[Y], set V[F] to 1 if there's borrow
 * 0x8XY6 : sets V[X] to value of V[X] >> 1 and V[F] is set to the least
 * 			significant bit of V[X]
 * 0x8XY7 : sets V[X] to value of V[Y] - V[X], set V[F] to 1 if there's borrow
 * 0x8XYE : sets V[X] to value of V[X] << 1 and V[F] is set to the most
 * 			significant bit of V[Y]
 */
void chip8::opcode8(unsigned short opcode)
{
	unsigned short x = (opcode & 0x0F00) >> 8;
	unsigned short y = (opcode & 0x00F0) >> 4;
	switch (opcode & 0x000F) {
		case 0x0000:
			V[x] = V[y];
			PC += 2;
		break;

		case 0x0001:
			V[x] = V[x] | V[y];
			PC += 2;
		break;

		case 0x0002:
			V[x] = V[x] & V[y];
			PC += 2;
		break;

		case 0x0003:
			V[x] = V[x] ^ V[y];
			PC += 2;
		break;

		case 0x0004:
			V[0xF] = 0;
			if (V[x] > (0xFF - V[y]))
				V[0xF] = 1;
			V[x] += V[y];
			PC += 2;
		break;

		case 0x0005:
			V[0xF] = 1;
			if (V[x] < V[y])
				V[0xF] = 0;
			V[x] -= V[y];
			PC += 2;
		break;

		case 0x0006:
			V[0xF] = V[x] & 0x1;
			V[x] = V[x] >> 1;
			PC += 2;
		break;

		case 0x0007:
			V[0xF] = 0;
			if (V[x] > V[y])
				V[0xF] = 1;
			V[x] = V[y] - V[x];
			PC += 2;
		break;

		case 0x000E:
			V[0xF] = 0;
			if (V[x] & 0x80)
				V[0xF] = 1;
			V[x] = V[x] << 1;
			PC += 2;
		break;
	}

}

/**
 * Handles all the 0x9*** opcodes
 *
 * 0x9XY0 : skips next instr. if V[X] doesn't equal V[Y]
 */
void chip8::opcode9(unsigned short opcode)
{
	if(V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
		PC += 4;
	else
		PC += 2;
}

/**
 * Handles all the 0xA*** opcodes
 *
 * 0xANNN : sets I to address NNN
 */
void chip8::opcodeA(unsigned short opcode)
{
	I = opcode & 0x0FFF;
	PC += 2;
}

/**
 * Handles all the 0xB*** opcodes
 *
 * 0xBNNN : jumps to address NNN + V[0]
 */
void chip8::opcodeB(unsigned short opcode)
{
	PC = (opcode & 0x0FFF) + V[0];
}

/**
 * Handles all the 0xC*** opcodes
 *
 * 0xCXNN : sets V[X] to the result of NN AND a random number 0-255
 */
void chip8::opcodeC(unsigned short opcode)
{
	V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF) & (rand() % 0xFF);
	PC += 2;
}

/**
 * Handles all the 0xD*** opcodes
 *
 * 0xDXYN : draws a sprite at coordinate (V[X], V[Y]) that 8 pixels wide and N
 * 			pixels tall, the sprite's pixels are stored at memory location I,
 * 			and if any screen pixels are changed from 1 to 0 V[F] is set to 1
 */
void chip8::opcodeD(unsigned short opcode)
{
	unsigned short originX = V[(opcode & 0x0F00) >> 8];
	unsigned short originY = V[(opcode & 0x00F0) >> 4];
	unsigned short n = opcode & 0x000F;
	unsigned short drawLocation;
	unsigned short pixel;

	V[0xF] = 0;

	for (int yindex = 0; yindex < n; yindex++) {
		pixel = memory[I + yindex];
		for (int xindex = 0; xindex < 8; xindex++) {
			if ((pixel & (0x80 >> xindex)) != 0) {
				drawLocation = (originX + xindex) + ((originY + yindex) * 64);
				if (graphics[drawLocation] == 1)
					V[0xF] = 1;
				graphics[drawLocation] ^= 1;
			}
		}
	}
	PC += 2;
	drawFlag = true;
}

/**
 * Handles all the 0xE*** opcodes
 *
 * 0xEX9E : if the key stored in V[X] is pressed then skip the next instr.
 * 0xEXA1 : if the key stored in V[X] is not pressed then skip the next instr.
 */
void chip8::opcodeE(unsigned short opcode)
{
	if ((opcode & 0x00FF) == 0x009E) {
		if (keypad[V[(opcode & 0x0F00) >> 8]])
			PC += 4;
		else
			PC += 2;
	}

	else if ((opcode & 0x00FF) == 0x00A1) {
		if (keypad[V[(opcode & 0x0F00) >> 8]])
			PC += 2;
		else
			PC += 4;
	}
}

/**
 * Handles all the 0xF*** opcodes
 *
 * 0xFX07 : set V[X] to delay timer
 * 0xFX0A : halt all instructions until a key is pressed
 * 0xFX15 : set delay timer to V[X]
 * 0xFX18 : set sound timer to V[X]
 * 0xFX1E : adds V[X] to I
 * 0xFX29 : sets I to the location of the sprite in V[X]
 * 0xFX55 : stores registers V[0]-V[X] inclusive into memory starting at I
 * 0xFX65 : loads registers V[0]-V[X] inclusive with data from memory starting
 * 			at I
 * 0xFX33 : stores the binary coded decimal representation of V[X] at address I
 */
void chip8::opcodeF(unsigned short opcode)
{
	bool temp;
	unsigned short x = (opcode & 0x0F00) >> 8;
	switch (opcode & 0x00FF) {
		case 0x0007:
			V[x] = delay_timer;
			PC += 2;
		break;

		case 0x000A:
			temp = false;
			for (int i = 0; i < 16; i++) {
				if (keypad[i] != 0) {
					V[x] = i;
					temp = true;
				}
			}
			if (!temp)
				return;

			PC += 2;
		break;

		case 0x0015:
			delay_timer = V[x];
			PC += 2;
		break;

		case 0x0018:
			sound_timer = V[x];
			PC += 2;
		break;

		case 0x001E:
			V[0xF] = 0;
			if (I + V[x] > 0xFFF)
				V[0xF] = 1;
			I += V[x];
			PC += 2;
		break;

		case 0x0029:
			I = V[x] * 5;
			PC += 2;
		break;

		case 0x0055:
			for (int i = 0; i <= x; i++)
				memory[I+i] = V[i];
			PC += 2;
			I += (x + 1);
		break;

		case 0x0065:
			for (int i = 0; i <= x; i++)
				V[i] = memory[I+i];
			PC += 2;
			I += (x + 1);
		break;

		case 0x0033:
			unsigned char tmp = V[x] % 100;
			memory[I] = V[x] / 100;
			memory[I + 1] = (V[x] / 10) % 10;
			memory[I + 2] = tmp % 10;
			PC += 2;
		break;
	}
}
