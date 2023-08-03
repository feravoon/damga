#include "GbCPU.h"
#include <stdio.h>

// Accessing pseudo-register M
uint8_t GbCPU::getM()
{
	return memory.read(getHL());
}
void GbCPU::setM(uint8_t value)
{
	memory.write(getHL(), value);
}

// Accessing register as two byte pairs
uint16_t GbCPU::getBC() { return (B << 8) | C; }
uint16_t GbCPU::getDE() { return (D << 8) | E; }
uint16_t GbCPU::getHL() { return (H << 8) | L; }
void GbCPU::setBC(uint16_t BC) { B = BC >> 8; C = BC & 0x00ff; }
void GbCPU::setDE(uint16_t DE) { D = DE >> 8; E = DE & 0x00ff; }
void GbCPU::setHL(uint16_t HL) { H = HL >> 8; L = HL & 0x00ff; }

uint8_t GbCPU::getFlags()
{
	return ((S ? 1 : 0) << 7) | ((Z ? 1 : 0) << 6) | ((AC ? 1 : 0) << 4) | ((P ? 1 : 0) << 2) | (CY ? 1 : 0) | 0x02;
}

void GbCPU::setFlags(uint8_t value)
{
	S = (value & 0x80) == 0x80;
	Z = (value & 0x40) == 0x40;
	AC = (value & 0x10) == 0x10;
	P = (value & 0x02) == 0x02;
	CY = (value & 0x01) == 0x01;
}

bool GbCPU::parity(int x, int size)
{
	int i;
	int p = 0;
	x = (x & ((1 << size) - 1));
	for (i = 0; i < size; i++)
	{
		if ((x & 0x1) == 0x1) p++;
		x >>= 1;
	}
	return (0 == (p & 0x1));
}

void GbCPU::updateFlagsArithmetic(int res)
{
	AC = false;
	CY = (res > 0xff);
	Z = ((res & 0xff) == 0);
	S = (0x80 == (res & 0x80));
	P = parity(res & 0xff, 8);
}

void GbCPU::updateFlagsLogic()
{
	CY = false;
	Z = (A == 0);
	S = (0x80 == (A & 0x80));
	P = parity(A, 8);
}

void GbCPU::updateFlagsZSP(uint8_t val)
{
	Z = (val == 0);
	S = (0x80 == (val & 0x80));
	P = parity(val, 8);
}

void GbCPU::generateInterrupt(int intID)
{
	uint16_t ret = PC;
	memory[SP - 1] = (ret >> 8) & 0xff;
	memory[SP - 2] = ret & 0xff;
	SP -= 2;
	PC = (uint16_t)(8 * intID);
	IE = false;
}
			
GbCPU::GbCPU()
{
	A = 0;
	B = 0;
	C = 0;
	D = 0;
	E = 0;
	H = 0;
	L = 0;
	PC = 0;
	SP = 0;

	Z = false;
	S = false;
	P = false;
	CY = false;
	AC = false;
	IE = true;

	memory = Memory();
	IO = IOcontroller();
}

int GbCPU::processInstruction() // Main method for processing an instruction (sould be called repeatedly in a loop)
{
	uint16_t opcode = memory[PC]; // Opcode is read from memory location indicated by PC
	uint8_t opcode1, opcode2; // Two bytes are defined for two instruction operands

	int cycleCount = cycles[opcode];

	// Instruction uint8_t length
	int byteLength = byteLengths[opcode];

	if (byteLength > 1)
	{
		// If instruction is longer than 1 byte, take next uint8_t as first operand
		opcode1 = memory[PC + 1];
	}

	if (byteLength > 2)
	{
		// If instruction is longer than 2 bytes, take further next uint8_t as second operand
		opcode2 = memory[PC + 2];
	}

	if(opcode==0xcb)
		opcode = (opcode1 << 8) | opcode;


	PC += byteLength; // Advance the PC by the uint8_t length of current instruction

	switch (opcode)
	{
	case 0x00: // NOP
	case 0x10:
	case 0x20:
	case 0x30:
	case 0x08:
	case 0x18:
	case 0x28:
	case 0x38:
		// Do nothing
		break;

	case 0x07: // RLC
	{
		uint8_t x = A;
		A = (((x & 0x80) >> 7) | (x << 1));
		CY = (0x80 == (x & 0x80));
	}
	break;

	case 0x0f: // RRC
	{
		uint8_t x = A;
		A = (((x & 0x01) << 7) | (x >> 1));
		CY = (0x01 == (x & 0x01));
	}
	break;

	case 0x17: // RAL
	{
		uint8_t x = A;
		A = ((CY ? 1 : 0) | (x << 1));
		CY = (0x80 == (x & 0x80));
	}
	break;

	case 0x1f: // RAR
	{
		uint8_t x = A;
		A = (((CY ? 1 : 0) << 7) | (x >> 1));
		CY = (0x01 == (x & 0x01));
	}
	break;

	// MVI
	case 0x06: B = opcode1; break; //MVI B, byte
	case 0x0e: C = opcode1; break; //MVI C, byte
	case 0x16: D = opcode1; break; //MVI D, byte
	case 0x1e: E = opcode1; break; //MVI E, byte
	case 0x26: H = opcode1; break; //MVI H, byte
	case 0x2e: L = opcode1; break; //MVI L, byte
	case 0x36: setM(opcode1); break; //MVI M, byte
	case 0x3e: A = opcode1; break; //MVI A, byte

	// LXI
	case 0x01: setBC((opcode2 << 8) | opcode1); break; //LXI B, word
	case 0x11: setDE((opcode2 << 8) | opcode1); break; //LXI D, word
	case 0x21: setHL((opcode2 << 8) | opcode1); break; //LXI H, word
	case 0x31: SP = ((opcode2 << 8) | opcode1); break; //LXI SP, word

	// STAX
	case 0x02: memory[getBC()] = A; break; // STAX B
	case 0x12: memory[getDE()] = A; break; // STAX D

	case 0x22: memory[(opcode2 << 8) | opcode1] = L; memory[((opcode2 << 8) | opcode1) + 1] = H; break; // SHLD
	case 0x32: memory[(opcode2 << 8) | opcode1] = A; break; // STA

	case 0x2f: A = ~A; break; // CMA

	case 0x37: CY = true; break; // STC
	case 0x3f: CY = false; break; // CMC

	// LDAX
	case 0x0a: A = memory[getBC()]; break; // LDAX B
	case 0x1a: A = memory[getDE()]; break; // LDAX D

	case 0x2a: L = memory[(opcode2 << 8) | opcode1]; H = memory[((opcode2 << 8) | opcode1) + 1];  break; // LHLD
	case 0x3a: A = memory[(opcode2 << 8) | opcode1]; break; // LDA

	// DAD
	case 0x09: { int res = getHL() + getBC(); setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD B
	case 0x19: { int res = getHL() + getDE(); setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD D
	case 0x29: { int res = getHL() + getHL(); setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD HL
	case 0x39: { int res = getHL() + SP; setHL(res); CY = (res & 0xffff0000) != 0; }; break; // DAD SP

	case 0x27: // DAA
		if (((A & 0xf) > 9) | AC)
			A += 6;
		if (((A & 0xf0) > 0x90) | CY)
		{
			int res = A + 0x60;
			A = (res & 0xff);
			updateFlagsArithmetic(res);
		}
		break;

	// INR
	case 0x04: B++; updateFlagsZSP(B); break; // INR B
	case 0x0c: C++; updateFlagsZSP(C); break; // INR C
	case 0x14: D++; updateFlagsZSP(D); break; // INR D
	case 0x1c: E++; updateFlagsZSP(E); break; // INR E
	case 0x24: H++; updateFlagsZSP(H); break; // INR H
	case 0x2c: L++; updateFlagsZSP(L); break; // INR L
	case 0x34: setM(getM()+1); updateFlagsZSP(getM()); break; // INR M
	case 0x3c: A++; updateFlagsZSP(A); break; // INR A

	// DCR
	case 0x05: B--; updateFlagsZSP(B); break; // DCR B
	case 0x0d: C--; updateFlagsZSP(C); break; // DCR C
	case 0x15: D--; updateFlagsZSP(D); break; // DCR D
	case 0x1d: E--; updateFlagsZSP(E); break; // DCR E
	case 0x25: H--; updateFlagsZSP(H); break; // DCR H
	case 0x2d: L--; updateFlagsZSP(L); break; // DCR L
	case 0x35: setM(getM() - 1); updateFlagsZSP(getM()); break; // DCR M
	case 0x3d: A--; updateFlagsZSP(A); break; // DCR A

	// INX
	case 0x03: setBC(getBC() + 1); break; // INX B
	case 0x13: setDE(getDE() + 1); break; // INX D
	case 0x23: setHL(getHL() + 1); break; // INX H
	case 0x33: SP++; break; // INX SP

	// DCX
	case 0x0b: setBC(getBC() - 1); break; // DCX B
	case 0x1b: setDE(getDE() - 1); break; // DCX D
	case 0x2b: setHL(getHL() - 1); break; // DCX H
	case 0x3b: SP--; break; // DCX SP

	// MOV B
	case 0x40: break; // MOV B,B
	case 0x41: B = C; break; // MOV B,C
	case 0x42: B = D; break; // MOV B,D
	case 0x43: B = E; break; // MOV B,E
	case 0x44: B = H; break; // MOV B,H
	case 0x45: B = L; break; // MOV B,L
	case 0x46: B = getM(); break; // MOV B,M
	case 0x47: B = A; break; // MOV B,A

	// MOV C
	case 0x48: C = B; break; // MOV C,B
	case 0x49: break; // MOV C,C
	case 0x4a: C = D; break; // MOV C,D
	case 0x4b: C = E; break; // MOV C,E
	case 0x4c: C = H; break; // MOV C,H
	case 0x4d: C = L; break; // MOV C,L
	case 0x4e: C = getM(); break; // MOV C,M
	case 0x4f: C = A; break; // MOV C,A

	// MOV D
	case 0x50: D = B; break; // MOV D,B
	case 0x51: D = C; break; // MOV D,C
	case 0x52: break; // MOV D,D
	case 0x53: D = E; break; // MOV D,E
	case 0x54: D = H; break; // MOV D,H
	case 0x55: D = L; break; // MOV D,L
	case 0x56: D = getM(); break; // MOV D,M
	case 0x57: D = A; break; // MOV D,A

	// MOV E
	case 0x58: E = B; break; // MOV E,B
	case 0x59: E = C; break; // MOV E,C
	case 0x5a: E = D; break; // MOV E,D
	case 0x5b: break; // MOV E,E
	case 0x5c: E = H; break; // MOV E,H
	case 0x5d: E = L; break; // MOV E,L
	case 0x5e: E = getM(); break; // MOV E,M
	case 0x5f: E = A; break; // MOV E,A

	// MOV H
	case 0x60: H = B; break; // MOV H,B
	case 0x61: H = C; break; // MOV H,C
	case 0x62: H = D; break; // MOV H,D
	case 0x63: H = E; break; // MOV H,E
	case 0x64: break; // MOV H,H
	case 0x65: H = L; break; // MOV H,L
	case 0x66: H = getM(); break; // MOV H,M
	case 0x67: H = A; break; // MOV H,A

	// MOV L
	case 0x68: L = B; break; // MOV L,B
	case 0x69: L = C; break; // MOV L,C
	case 0x6a: L = D; break; // MOV L,D
	case 0x6b: L = E; break; // MOV L,E
	case 0x6c: L = H; break; // MOV L,H
	case 0x6d: break; // MOV L,L
	case 0x6e: L = getM(); break; // MOV L,M
	case 0x6f: L = A; break; // MOV L,A

	// MOV M
	case 0x70: setM (B); break; // MOV M,B
	case 0x71: setM(C); break; // MOV M,C
	case 0x72: setM(D); break; // MOV M,D
	case 0x73: setM(E); break; // MOV M,E
	case 0x74: setM(H); break; // MOV M,H
	case 0x75: setM(L); break; // MOV M,L
	case 0x76: break; // HLT
	case 0x77: setM(A); break; // MOV M,A

	// MOV A
	case 0x78: A = B; break; // MOV A,B
	case 0x79: A = C; break; // MOV A,C
	case 0x7a: A = D; break; // MOV A,D
	case 0x7b: A = E; break; // MOV A,E
	case 0x7c: A = H; break; // MOV A,H
	case 0x7d: A = L; break; // MOV A,L
	case 0x7e: A = getM(); break; // MOV A,M
	case 0x7f: break; // MOV A,A

	// ADD
	// AC (Auxiliary Carry flag) was not implemented at first. But, I added it, only at the row below, to make the coin count logic work.
	case 0x80: { int res = A + B; updateFlagsArithmetic(res); AC = ((A & 0x0f)+(B & 0x0f) > 0x0f); A = (res & 0xff); } break; // ADD B
	case 0x81: { int res = A + C; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD C
	case 0x82: { int res = A + D; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD D
	case 0x83: { int res = A + E; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD E
	case 0x84: { int res = A + H; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD H
	case 0x85: { int res = A + L; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD L
	case 0x86: { int res = A + getM(); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD M
	case 0x87: { int res = A + A; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADD A

	// ADC
	case 0x88: { int res = A + B + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC B
	case 0x89: { int res = A + C + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC C
	case 0x8a: { int res = A + D + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC D
	case 0x8b: { int res = A + E + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC E
	case 0x8c: { int res = A + H + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC H
	case 0x8d: { int res = A + L + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC L
	case 0x8e: { int res = A + getM() + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC M
	case 0x8f: { int res = A + A + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADC A

	// SUB
	case 0x90: { uint16_t res = (A - B); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB B
	case 0x91: { uint16_t res = (A - C); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB C
	case 0x92: { uint16_t res = (A - D); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB D
	case 0x93: { uint16_t res = (A - E); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB E
	case 0x94: { uint16_t res = (A - H); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB H
	case 0x95: { uint16_t res = (A - L); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB L
	case 0x96: { uint16_t res = (A - getM()); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB M
	case 0x97: { uint16_t res = (A - A); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SUB A

	// SBB
	case 0x98: { uint16_t res = (A - B - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB B
	case 0x99: { uint16_t res = (A - C - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB C
	case 0x9a: { uint16_t res = (A - D - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB D
	case 0x9b: { uint16_t res = (A - E - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB E
	case 0x9c: { uint16_t res = (A - H - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB H
	case 0x9d: { uint16_t res = (A - L - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB L
	case 0x9e: { uint16_t res = (A - getM() - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB M
	case 0x9f: { uint16_t res = (A - A - (CY ? 1 : 0)); updateFlagsArithmetic(res); A = (res & 0xff); } break; // SBB A

	// ANA
	case 0xa0: A &= B; updateFlagsLogic(); break; // ANA B
	case 0xa1: A &= C; updateFlagsLogic(); break; // ANA C
	case 0xa2: A &= D; updateFlagsLogic(); break; // ANA D
	case 0xa3: A &= E; updateFlagsLogic(); break; // ANA E
	case 0xa4: A &= H; updateFlagsLogic(); break; // ANA H
	case 0xa5: A &= L; updateFlagsLogic(); break; // ANA L
	case 0xa6: A &= getM(); updateFlagsLogic(); break; // ANA M
	case 0xa7: A &= A; updateFlagsLogic(); break; // ANA A

	// XRA
	case 0xa8: A ^= B; updateFlagsLogic(); break; // XRA B
	case 0xa9: A ^= C; updateFlagsLogic(); break; // XRA C
	case 0xaa: A ^= D; updateFlagsLogic(); break; // XRA D
	case 0xab: A ^= E; updateFlagsLogic(); break; // XRA E
	case 0xac: A ^= H; updateFlagsLogic(); break; // XRA H
	case 0xad: A ^= L; updateFlagsLogic(); break; // XRA L
	case 0xae: A ^= getM(); updateFlagsLogic(); break; // XRA M
	case 0xaf: A ^= A; updateFlagsLogic(); break; // XRA A

	// ORA
	case 0xb0: A |= B; updateFlagsLogic(); break; // ORA B
	case 0xb1: A |= C; updateFlagsLogic(); break; // ORA C
	case 0xb2: A |= D; updateFlagsLogic(); break; // ORA D
	case 0xb3: A |= E; updateFlagsLogic(); break; // ORA E
	case 0xb4: A |= H; updateFlagsLogic(); break; // ORA H
	case 0xb5: A |= L; updateFlagsLogic(); break; // ORA L
	case 0xb6: A |= getM(); updateFlagsLogic(); break; // ORA M
	case 0xb7: A |= A; updateFlagsLogic(); break; // ORA A

	// CMP
	case 0xb8: { uint16_t res = (A - B); updateFlagsArithmetic(res); } break; // CMP B
	case 0xb9: { uint16_t res = (A - C); updateFlagsArithmetic(res); } break; // CMP C
	case 0xba: { uint16_t res = (A - D); updateFlagsArithmetic(res); } break; // CMP D
	case 0xbb: { uint16_t res = (A - E); updateFlagsArithmetic(res); } break; // CMP E
	case 0xbc: { uint16_t res = (A - H); updateFlagsArithmetic(res); } break; // CMP H
	case 0xbd: { uint16_t res = (A - L); updateFlagsArithmetic(res); } break; // CMP L
	case 0xbe: { uint16_t res = (A - getM()); updateFlagsArithmetic(res); } break; // CMP M
	case 0xbf: { uint16_t res = (A - A); updateFlagsArithmetic(res); } break; // CMP A

	case 0xc6: { int res = A + opcode1; updateFlagsArithmetic(res); A = (res & 0xff); } break; // ADI 
	case 0xd6: { uint16_t res = (A - opcode1); updateFlagsZSP((res & 0xff)); CY = res > 0xff; A = (res & 0xff); } break; // SUI
	case 0xe6: A &= opcode1; updateFlagsLogic(); break; // ANI
	case 0xf6: A |= opcode1; updateFlagsLogic(); break; // ORI

	case 0xce: { int res = A + opcode1 + (CY ? 1 : 0); updateFlagsArithmetic(res); A = (res & 0xff); } break; // ACI
	case 0xde: { uint16_t res = (A - opcode1 - (CY ? 1 : 0)); updateFlagsZSP((res & 0xff)); CY = res > 0xff; A = (res & 0xff); } break; // SBI
	case 0xee: A ^= opcode1; updateFlagsLogic(); break; // XRI
	case 0xfe: { uint8_t res = (A - opcode1); updateFlagsZSP(res); CY = A < opcode1; } break; // CPI

	case 0xc3: PC = ((opcode2 << 8) | opcode1); break; // JMP adr
	case 0xc2: if (!Z) { PC = ((opcode2 << 8) | opcode1); } break; // JNZ adr
	case 0xca: if (Z) { PC = ((opcode2 << 8) | opcode1); } break; // JZ adr
	case 0xd2: if (!CY) { PC = ((opcode2 << 8) | opcode1); } break; // JNC adr
	case 0xda: if (CY) { PC = ((opcode2 << 8) | opcode1); } break; // JC adr
	case 0xe2: if (!P) { PC = ((opcode2 << 8) | opcode1); } break; // JPO adr
	case 0xea: if (P) { PC = ((opcode2 << 8) | opcode1); } break; // JPE adr
	case 0xf2: if (!S) { PC = ((opcode2 << 8) | opcode1); } break; // JP adr
	case 0xfa: if (S) { PC = ((opcode2 << 8) | opcode1); } break; // JM adr

	case 0xc7: // RST 0
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0000;
	}
	break;

	case 0xcf: // RST 1
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0008;
	}
	break;

	case 0xd7: // RST 2
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0010;
	}
	break;

	case 0xdf: // RST 3
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0018;
	}
	break;

	case 0xe7: // RST 4
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0020;
	}
	break;

	case 0xef: // RST 5
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0028;
	}
	break;

	case 0xf7: // RST 6
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0030;
	}
	break;

	case 0xff: // RST 7
	{
		uint16_t ret = (PC + 2);
		memory[SP - 1] = ((ret >> 8) & 0xff);
		memory[SP - 2] = (ret & 0xff);
		SP -= 2;
		PC = 0x0038;
	}
	break;



	case 0xc9: // RET
		PC = (memory[SP] | (memory[SP + 1] << 8));
		SP += 2;
		break;

	case 0xc8: // RZ
		if (Z)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xc0: // RNZ
		if (!Z)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xd8: // RC
		if (CY)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xd0: // RNC
		if (!CY)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xe8: // RPE
		if (P)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xe0: // RPO
		if (!P)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xf8: // RM
		if (S)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xf0: // RP
		if (!S)
		{
			PC = (memory[SP] | (memory[SP + 1] << 8));
			SP += 2;
		}
		break;

	case 0xcd: // CALL adr
		if (5 == ((opcode2 << 8) | opcode1)) // to simulate message printing routine of CP/M for debugging purposes
		{
			if (C == 9)
			{
				printf("\n\n");
				int offset = (D << 8) | (E);
				char str = (char)memory[offset + 3];  //skip the prefix bytes    
				while (str != '$')
				{
					str = (char)memory[offset + 3];
					printf("%s",&str);
					offset++;
				}
				printf("\n\n");
			}
			else if (C == 2)
			{
				//saw this in the inspected code, never saw it called    
				printf("print char routine called\n");
			}
		}
		else if (0 == ((opcode2 << 8) | opcode1))
		{
			printf("CPU FAIL!");
		}
		else // Actual logic of the CALL insturction
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xc4: // CNZ
		if (!Z)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;
	case 0xcc: // CZ
		if (Z)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xd4: // CNC
		if (!CY)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xdc: // CC
		if (CY)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xe4: // CPO
		if (!P)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xec: // CPE
		if (P)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xf4: // CP
		if (!S)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xfc: // CM
		if (S)
		{
			uint16_t ret = PC;
			memory[SP - 1] = ((ret >> 8) & 0xff);
			memory[SP - 2] = (ret & 0xff);
			SP -= 2;
			PC = ((opcode2 << 8) | opcode1);
		}
		break;

	case 0xc5: memory[SP - 1] = B; memory[SP - 2] = C; SP -= 2; break; // PUSH B
	case 0xd5: memory[SP - 1] = D; memory[SP - 2] = E; SP -= 2; break; // PUSH D
	case 0xe5: memory[SP - 1] = H; memory[SP - 2] = L; SP -= 2; break; // PUSH HL
	case 0xf5: memory[SP - 1] = A; memory[SP - 2] = getFlags(); SP -= 2; break; // PUSH PSW

	case 0xc1: B = memory[SP + 1]; C = memory[SP]; SP += 2; break; // POP B
	case 0xd1: D = memory[SP + 1]; E = memory[SP]; SP += 2; break; // POP D
	case 0xe1: H = memory[SP + 1]; L = memory[SP]; SP += 2; break; // POP HL
	case 0xf1: A = memory[SP + 1]; setFlags(memory[SP]); SP += 2; break; // POP PSW

	case 0xeb: { uint16_t tmp; tmp = getHL(); setHL(getDE()); setDE(tmp); } break; // XCHG

	case 0xd3: IO.O[opcode1] = A; IO.newOutput[opcode1] = true; break; // OUT
	case 0xdb: A = IO.I[opcode1]; break; // IN

	case 0xf3: IE = false; break; // DI
	case 0xfb: IE = true; break; // EI

	case 0xe3: // XTHL 
	{
		uint8_t Hval = H;
		uint8_t Lval = L;
		L = memory[SP];
		H = memory[SP + 1];
		memory[SP] = Lval;
		memory[SP + 1] = Hval;
	}
	break;

	case 0xe9: PC = ((H << 8) | L); break; // PCHL
	case 0xf9: SP = ((H << 8) | L); break; // SPHL

	// Extended Opcodes



	default:
		printf("\t\tUnimplemented instruction:");
		PC -= byteLength; // Decrement PC by 1 for disassembly code to work on the right instruction
		disassemble_GbCPU_Op();
		printf("\n");
		PC += byteLength;
		// throw new Exception("Unimplemented Opcode!");
		return 0;
	}


	if (printOutput)
	{
		printf("\t\t");
		printf("%c", Z ? 'Z' : '.');
		printf("%c", S ? 'S' : '.');
		printf("%c", P ? 'P' : '.');
		printf("%c", CY ? 'C' : '.');
		printf("%c  ", AC ? 'A' : '.');
		printf("A $%02x B $%02x C $%02x D $%02x E $%02x H $%02x L $%02x SP %04x\n", A, B, C, D, E, H, L, SP);
	}

	return cycleCount; // return the number of cycles for cycle counting
}

int GbCPU::disassemble_GbCPU_Op()
{
	uint8_t code = memory.read(PC);
	uint8_t code1 = memory.read(PC+1);
	uint8_t code2 = memory.read(PC+2);

	int opbytes = byteLengths[code];
	const char* opString;
	if(code == 0xcb)
	{	
		opString = extOps[code1];
		opbytes = 1;
	}
	else
	{
		opString = ops[code];
	}

	printf("%04x ", PC);
	switch (opbytes)
	{
		case 1:
			printf("%s", opString); break;
		case 2:
			printf(opString, code1); break;
		case 3:
			printf(opString, code2, code1); break;
	}
	
	if(opbytes==1)
		printf("\t");

	if(code == 0xcb)
		opbytes = byteLengths[code];

	return opbytes;
}