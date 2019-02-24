#ifndef CHIP8H
#define CHIP8H
class chip8{
public:
	chip8();
	~chip8();
	void emulateCycle();


	unsigned char keypad[16];		// stores curr state of keypad
	unsigned char graphics[64 * 32];

	bool loadApplication(const char * filename);

	bool drawFlag;
	void opcode0(unsigned short opcode);
	void opcode1(unsigned short opcode);
	void opcode2(unsigned short opcode);
	void opcode3(unsigned short opcode);
	void opcode4(unsigned short opcode);
	void opcode5(unsigned short opcode);
	void opcode6(unsigned short opcode);
	void opcode7(unsigned short opcode);
	void opcode8(unsigned short opcode);
	void opcode9(unsigned short opcode);
	void opcodeA(unsigned short opcode);
	void opcodeB(unsigned short opcode);
	void opcodeC(unsigned short opcode);
	void opcodeD(unsigned short opcode);
	void opcodeE(unsigned short opcode);
	void opcodeF(unsigned short opcode);
private:
	unsigned short opcode; 			// current opcode
	unsigned char memory[4096]; 	// emulates chip8 memory
	unsigned char V[16];			// holds the general purpose registers
	unsigned short stack[16];		// emulates the stack

	unsigned short SP;				// stack pointer
	unsigned short I;				// instruction pointer
	unsigned short PC;				// program counter
	unsigned char delay_timer;
	unsigned char sound_timer;


	void initialize();
};
#endif
