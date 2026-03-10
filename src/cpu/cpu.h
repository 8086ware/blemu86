#pragma once
#include <cstdint>
#include "chipset/pic.h"
#include <variant>
#include "chipset/bus.h"
#include <chrono>
#include <array>

// emulates 186 feature 16 bit cpu

class CPU 
{
	friend class Instruction;
	friend class Operand;

	// (lowest to highest bit) destination/instruction width 0, reg to reg / mem 1 (opposite for 1), modrm 2, data 1 3, data 2 4, segment encoded 5, reg encoded in last three bits 6, reg = sreg 7, data 1 width 8, data 2 width 9,
	static constexpr std::array<uint16_t, 256> opcode_desc
	{
		//octal 0xx
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x21,
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x21,
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x21,
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x21,
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x0,
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x0,
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x0,
		0x4, 0x5, 0x6, 0x7, 0x8, 0x109, 0x21, 0x0,
		// octal 1xx
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x109, 0x10D, 0x8, 0xC, 0x0, 0x1, 0x0, 0x1,
		0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
		0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8,
		// octal 2xx
		0xC, 0x10D, 0x0, 0xD, 0x4, 0x5, 0x4, 0x5,
		0x4, 0x5, 0x6, 0x7, 0x85, 0x7, 0x87, 0x5,
		0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
		0x0, 0x1, 0x319, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x108, 0x109, 0x108, 0x109, 0x0, 0x1, 0x0, 0x1,
		0x8, 0x109, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1,
		0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48, 0x48,
		0x149, 0x149, 0x149, 0x149, 0x149, 0x149, 0x149, 0x149,
		// octal 3xx
		0xC, 0xD, 0x109, 0x0, 0x7, 0x7, 0xC, 0x10D,
		0x0, 0x0, 0x109, 0x0, 0x0, 0x8, 0x0, 0x0,
		0x4, 0x5, 0x4, 0x5, 0x8, 0x8, 0x0, 0x0,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
		0x8, 0x8, 0x8, 0x8, 0x8, 0x9, 0x8, 0x9,
		0x109, 0x109, 0x319, 0x8, 0x0, 0x1, 0x0, 0x1,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x105,
		0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x5,
	};

	enum class CPU_Group_Opcode_80 : uint8_t
	{
		ADD_RM8_IMM8 = 0x0,// Opcode group 80
		OR_RM8_IMM8 = 0x1, // Opcode group 80
		ADC_RM8_IMM8 = 0x2, // Opcode group 80
		SBB_RM8_IMM8 = 0x3, // Opcode group 80
		AND_RM8_IMM8 = 0x4, // Opcode group 80
		SUB_RM8_IMM8 = 0x5, // Opcode group 80
		XOR_RM8_IMM8 = 0x6, // Opcode group 80
		CMP_RM8_IMM8 = 0x7, // Opcode group 80
	};

	enum class CPU_Group_Opcode_81 : uint8_t
	{
		ADD_RM16_IMM16 = 0x0,// Opcode group 81
		OR_RM16_IMM16 = 0x1, // Opcode group 81
		ADC_RM16_IMM16 = 0x2, // Opcode group 81
		SBB_RM16_IMM16 = 0x3,// Opcode group 81
		AND_RM16_IMM16 = 0x4, // Opcode group 81
		SUB_RM16_IMM16 = 0x5, // Opcode group 81
		XOR_RM16_IMM16 = 0x6, // Opcode group 81
		CMP_RM16_IMM16 = 0x7, // Opcode group 81
	};

	enum class CPU_Group_Opcode_83 : uint8_t
	{
		ADD_RM16_IMM8 = 0x0, // Opcode group 83
		OR_RM16_IMM8 = 0x1, // Opcode group 83
		ADC_RM16_IMM8 = 0x2, // Opcode group 83
		SBB_RM16_IMM8 = 0x3, // Opcode group 83
		AND_RM16_IMM8 = 0x4, // Opcode group 83
		SUB_RM16_IMM8 = 0x5, // Opcode group 83
		XOR_RM16_IMM8 = 0x6, // Opcode group 83
		CMP_RM16_IMM8 = 0x7, // Opcode group 83
	};


	enum class CPU_Group_Opcode_C0 : uint8_t
	{
		ROL_RM8_IMM8 = 0x0,
		ROR_RM8_IMM8 = 0x1,
		RCL_RM8_IMM8 = 0x2,
		RCR_RM8_IMM8 = 0x3,
		SAL_RM8_IMM8 = 0x4,
		SHR_RM8_IMM8 = 0x5,
		SAR_RM8_IMM8 = 0x7,
	};

	enum class CPU_Group_Opcode_C1 : uint8_t
	{
		ROL_RM16_IMM8 = 0x0,
		ROR_RM16_IMM8 = 0x1,
		RCL_RM16_IMM8 = 0x2,
		RCR_RM16_IMM8 = 0x3,
		SAL_RM16_IMM8 = 0x4,
		SHR_RM16_IMM8 = 0x5,
		SAR_RM16_IMM8 = 0x7,
	};

	enum class CPU_Group_Opcode_D0 : uint8_t
	{
		ROL_RM8_1 = 0x0,
		ROR_RM8_1 = 0x1,
		RCL_RM8_1 = 0x2,
		RCR_RM8_1 = 0x3,
		SAL_RM8_1 = 0x4, // Opcode group D0
		SHR_RM8_1 = 0x5, // Opcode group D0
		SAR_RM8_1 = 0x7, // Opcode group D0
	};

	enum class CPU_Group_Opcode_D1 : uint8_t
	{
		ROL_RM16_1 = 0x0,
		ROR_RM16_1 = 0x1,
		RCL_RM16_1 = 0x2,
		RCR_RM16_1 = 0x3,
		SAL_RM16_1 = 0x4, // Opcode group D1
		SHR_RM16_1 = 0x5, // Opcode group D1
		SAR_RM16_1 = 0x7, // Opcode group D1
	};

	enum class CPU_Group_Opcode_D2 : uint8_t
	{
		ROL_RM8_CL = 0x0,
		ROR_RM8_CL = 0x1,
		RCL_RM8_CL = 0x2,
		RCR_RM8_CL = 0x3,
		SAL_RM8_CL = 0x4, // Opcode group D2
		SHR_RM8_CL = 0x5, // Opcode group D2
		SAR_RM8_CL = 0x7, // Opcode group D2
	};

	enum class CPU_Group_Opcode_D3 : uint8_t
	{
		ROL_RM16_CL = 0x0,
		ROR_RM16_CL = 0x1,
		RCL_RM16_CL = 0x2,
		RCR_RM16_CL = 0x3,
		// Same as SHL
		SAL_RM16_CL = 0x4, // Opcode group D3
		SHR_RM16_CL = 0x5, // Opcode group D3
		SAR_RM16_CL = 0x7, // Opcode group D3
	};

	enum class CPU_Group_Opcode_F6 : uint8_t
	{
		TEST_RM8_IMM8 = 0x0, // Opcode group F6
		NOT_RM8 = 0x2, // Opcode group F6
		NEG_RM8 = 0x3, // Opcode group F6
		MUL_RM8 = 0x4, // Opcode group F6
		IMUL_RM8 = 0x5,
		DIV_RM8 = 0x6,
		IDIV_RM8 = 0x7, // Opcode group F6
	};

	enum class CPU_Group_Opcode_F7 : uint8_t
	{
		TEST_RM16_IMM16 = 0x0, // Opcode group F7
		NOT_RM16 = 0x2, // Opcode group F7
		NEG_RM16 = 0x3, // Opcode group F7
		MUL_RM16 = 0x4, // Opcode group F7
		DIV_RM16 = 0x6,
		IDIV_RM16 = 0x7, // Opcode group F6
	};

	enum class CPU_Group_Opcode_FE : uint8_t
	{
		INC_RM8 = 0x0, // Opcode group FE
		DEC_RM8 = 0x1, // Opcode group FE
	};

	enum class CPU_Group_Opcode_FF : uint8_t
	{
		INC_RM16 = 0x0, // Opcode group FF
		DEC_RM16 = 0x1, // Opcode group FF
		CALL_RM16 = 0x2, // Opcode group FF
		CALL_M16_16 = 0x3, // Opcode group FF
		JMP_RM16 = 0x4, // Opcode group FF
		JMP_M16_16 = 0x5,
		PUSH_RM16 = 0x6, // Opcode group FF
	};


	// http://forthworks.com:8800/temp/opcodes.html

	enum class CPU_Opcode : uint8_t
	{
		PREFIX_ES = 0x26,
		PREFIX_CS = 0x2E,
		PREFIX_SS = 0x36,
		PREFIX_DS = 0x3E,

		PREFIX_REPNE = 0xF2,
		PREFIX_REP_OR_REPE = 0xF3,

		GROUP_OPCODE_80 = 0x80,
		GROUP_OPCODE_81 = 0x81,
		GROUP_OPCODE_83 = 0x83,
		GROUP_OPCODE_C0 = 0xC0,
		GROUP_OPCODE_C1 = 0xC1,
		GROUP_OPCODE_D0 = 0xD0,
		GROUP_OPCODE_D1 = 0xD1,
		GROUP_OPCODE_D2 = 0xD2,
		GROUP_OPCODE_D3 = 0xD3,
		GROUP_OPCODE_F6 = 0xF6,
		GROUP_OPCODE_F7 = 0xF7,
		GROUP_OPCODE_FE = 0xFE,
		GROUP_OPCODE_FF = 0xFF,

		AAD = 0xD5,
		AAM = 0xD4,
		AAS = 0x3F,

		ADC_AL_IMM8 = 0x14,
		ADC_AX_IMM16 = 0x15,
		ADC_RM8_R8 = 0x10,
		ADC_RM16_R16 = 0x11,
		ADC_R8_RM8 = 0x12,
		ADC_R16_RM16 = 0x13,

		ADD_RM8_R8 = 0x00,
		ADD_RM16_R16 = 0x01,
		ADD_R8_RM8 = 0x02,
		ADD_R16_RM16 = 0x03,
		ADD_AL_IMM8 = 0x04,
		ADD_AX_IMM16 = 0x05,

		AND_AL_IMM8 = 0x24,
		AND_AX_IMM16 = 0x25,
		AND_RM8_R8 = 0x20,
		AND_RM16_R16 = 0x21,
		AND_R8_RM8 = 0x22,
		AND_R16_RM16 = 0x23,

		CALL_REL16 = 0xE8,
		CALL_PTR16_16 = 0x9A,

		CBW = 0x98,

		CLC = 0xF8,
		CLD = 0xFC,
		CLI = 0xFA,
		CMC = 0xF5,

		CMP_AL_IMM8 = 0x3C,
		CMP_AX_IMM16 = 0x3D,
		CMP_RM8_R8 = 0x38,
		CMP_RM16_R16 = 0x39,
		CMP_R8_RM8 = 0x3A,
		CMP_R16_RM16 = 0x3B,
		CMPSB = 0xA6,
		CMPSW = 0xA7,

		CWD = 0x99,

		DAA = 0x27,
		DAS = 0x2F,

		// Affects flag register
		DEC_AX = 0x48,
		DEC_CX = DEC_AX + 1,
		DEC_DX = DEC_AX + 2,
		DEC_BX = DEC_AX + 3,
		DEC_SP = DEC_AX + 4,
		DEC_BP = DEC_AX + 5,
		DEC_SI = DEC_AX + 6,
		DEC_DI = DEC_AX + 7,

		HLT = 0xF4,

		IMUL_R16_RM16_IMM8 = 0x6B,
		IMUL_R16_RM16_IMM16 = 0x69,

		IN_AL_IMM8 = 0xE4,
		IN_AX_IMM8 = 0xE5,
		IN_AL_DX = 0xEC,
		IN_AX_DX = 0xED,

		// Affects flag register
		INC_AX = 0x40,
		INC_CX = INC_AX + 1,
		INC_DX = INC_AX + 2,
		INC_BX = INC_AX + 3,
		INC_SP = INC_AX + 4,
		INC_BP = INC_AX + 5,
		INC_SI = INC_AX + 6,
		INC_DI = INC_AX + 7,

		INSB = 0x6C,
		INSW = 0x6D,

		INT_IMM8 = 0xCD,

		IRET = 0xCF,

		// Affects flag register
		JMP_REL8 = 0xEB,
		JMP_REL16 = 0xE9,
		JMP_PTR16_16 = 0xEA,

		JA_REL8 = 0x77,
		JAE_REL8 = 0x73,
		JB_REL8 = 0x72,
		JBE_REL8 = 0x76,
		JCXZ_REL8 = 0xE3,
		JE_REL8 = 0x74,
		JG_REL8 = 0x7F,
		JGE_REL8 = 0x7D,
		JL_REL8 = 0x7C,
		JLE_REL8 = 0x7E,
		JNE_REL8 = 0x75,
		JNO_REL8 = 0x71,
		JNP_REL8 = 0x7B,
		JNS_REL8 = 0x79,
		JO_REL8 = 0x70,
		JP_REL8 = 0x7A,
		JS_REL8 = 0x78,

		LAHF = 0x9F,

		LDS = 0xC5,
		LEA = 0x8D,
		LES = 0xC4,

		LOOP_REL8 = 0xE2,
		LOOPE_REL8 = 0xE1,
		LOOPNE_REL8 = 0xE0,

		LODSB = 0xAC,
		LODSW = 0xAD,

		MOV_RM8_R8 = 0x88,
		MOV_RM16_R16 = 0x89,
		MOV_R8_RM8 = 0x8A,
		MOV_R16_RM16 = 0x8B,
		MOV_RM16_SREG = 0x8C,
		MOV_SREG_RM16 = 0x8E,
		MOV_AL_MOFFS8 = 0xA0,
		MOV_AX_MOFFS16 = 0xA1,
		MOV_MOFFS8_AL = 0xA2,
		MOV_MOFFS16_AX = 0xA3,
		MOV_AL_IMM8 = 0xB0,
		MOV_CL_IMM8 = MOV_AL_IMM8 + 1,
		MOV_DL_IMM8 = MOV_AL_IMM8 + 2,
		MOV_BL_IMM8 = MOV_AL_IMM8 + 3,
		MOV_AH_IMM8 = MOV_AL_IMM8 + 4,
		MOV_CH_IMM8 = MOV_AL_IMM8 + 5,
		MOV_DH_IMM8 = MOV_AL_IMM8 + 6,
		MOV_BH_IMM8 = MOV_AL_IMM8 + 7,
		MOV_AX_IMM16 = 0xB8,
		MOV_CX_IMM16 = MOV_AX_IMM16 + 1,
		MOV_DX_IMM16 = MOV_AX_IMM16 + 2,
		MOV_BX_IMM16 = MOV_AX_IMM16 + 3,
		MOV_SP_IMM16 = MOV_AX_IMM16 + 4,
		MOV_BP_IMM16 = MOV_AX_IMM16 + 5,
		MOV_SI_IMM16 = MOV_AX_IMM16 + 6,
		MOV_DI_IMM16 = MOV_AX_IMM16 + 7,
		MOV_RM8_IMM8 = 0xC6,
		MOV_RM16_IMM16 = 0xC7,

		MOVSB = 0xA4,
		MOVSW = 0xA5,

		OR_AL_IMM8 = 0x0C,
		OR_AX_IMM16 = 0x0D,
		OR_RM8_R8 = 0x08,
		OR_RM16_R16 = 0x09,
		OR_R8_RM8 = 0x0A,
		OR_R16_RM16 = 0x0B,

		OUT_IMM8_AL = 0xE6,
		OUT_IMM8_AX = 0xE7,
		OUT_DX_AL = 0xEE,
		OUT_DX_AX = 0xEF,

		OUTSB = 0x6E,
		OUTSW = 0x6F,

		PUSHA = 0x60,

		PUSH_AX = 0x50,
		PUSH_CX = PUSH_AX + 1,
		PUSH_DX = PUSH_AX + 2,
		PUSH_BX = PUSH_AX + 3,
		PUSH_SP = PUSH_AX + 4,
		PUSH_BP = PUSH_AX + 5,
		PUSH_SI = PUSH_AX + 6,
		PUSH_DI = PUSH_AX + 7,
		PUSH_ES = 0x6,
		PUSH_CS = PUSH_ES + 0x8,
		PUSH_SS = PUSH_ES + 0x10,
		PUSH_DS = PUSH_ES + 0x18,
		PUSHF = 0x9C,

		PUSH_IMM8 = 0x6A,
		PUSH_IMM16 = 0x68,

		POPA = 0x61,

		POP_AX = 0x58,
		POP_CX = POP_AX + 1,
		POP_DX = POP_AX + 2,
		POP_BX = POP_AX + 3,
		POP_SP = POP_AX + 4,
		POP_BP = POP_AX + 5,
		POP_SI = POP_AX + 6,
		POP_DI = POP_AX + 7,

		POP_RM16 = 0x8F,
		POP_ES = 0x07,
		POP_CS = POP_ES + 0x8,
		POP_SS = POP_ES + 0x10,
		POP_DS = POP_ES + 0x18,
		POPF = 0x9D,

		RET_NEAR = 0xC3,
		RET_FAR = 0xCB,
		RET_NEAR_IMM16 = 0xC2,
		RET_FAR_IMM16 = 0xCA,

		SAHF = 0x9E,

		SBB_AL_IMM8 = 0x1C,
		SBB_AX_IMM16 = 0x1D,
		SBB_RM8_R8 = 0x18,
		SBB_RM16_R16 = 0x19,
		SBB_R8_RM8 = 0x1A,
		SBB_R16_RM16 = 0x1B,

		SCASB = 0xAE,
		SCASW = 0xAF,

		STC = 0xF9,
		STD = 0xFD,
		STI = 0xFB,

		STOSB = 0xAA,
		STOSW = 0xAB,

		SUB_AL_IMM8 = 0x2C,
		SUB_AX_IMM16 = 0x2D,
		SUB_RM8_R8 = 0x28,
		SUB_RM16_R16 = 0x29,
		SUB_R8_RM8 = 0x2A,
		SUB_R16_RM16 = 0x2B,

		TEST_AL_IMM8 = 0xA8,
		TEST_AX_IMM16 = 0xA9,
		TEST_RM8_R8 = 0x84,
		TEST_RM16_R16 = 0x85,

		XCHG_AX_AX = 0x90,
		XCHG_AX_CX = XCHG_AX_AX + 1,
		XCHG_AX_DX = XCHG_AX_AX + 2,
		XCHG_AX_BX = XCHG_AX_AX + 3,
		XCHG_AX_SP = XCHG_AX_AX + 4,
		XCHG_AX_BP = XCHG_AX_AX + 5,
		XCHG_AX_SI = XCHG_AX_AX + 6,
		XCHG_AX_DI = XCHG_AX_AX + 7,

		XCHG_RM8_R8 = 0x86,
		XCHG_RM16_R16 = 0x87,

		XLAT = 0xD7,

		XOR_AL_IMM8 = 0x34,
		XOR_AX_IMM16 = 0x35,
		XOR_RM8_R8 = 0x30,
		XOR_RM16_R16 = 0x31,
		XOR_R8_RM8 = 0x32,
		XOR_R16_RM16 = 0x33,
	};
public:
	enum class Registers_8
	{
		// These are for accessing 8 bit registers
		AL,
		CL,
		DL,
		BL,
		AH,
		CH,
		DH,
		BH,
	};

	enum class Registers
	{
		AX,
		CX,
		DX,
		BX,
		SP,
		BP,
		SI,
		DI,
		IP,
		Flags,
		Count,
	};

	enum class Segment_Registers
	{
		ES,
		CS,
		SS,
		DS,
		Count
	};

	static constexpr uint16_t flag_carry{ 0x1 };
	static constexpr uint16_t flag_parity{ 0x4 };
	static constexpr uint16_t flag_half_carry{ 0x10 };
	static constexpr uint16_t flag_zero{ 0x40 };
	static constexpr uint16_t flag_sign{ 0x80 };
	static constexpr uint16_t flag_trap{ 0x100 };
	static constexpr uint16_t flag_interrupt{ 0x200 };
	static constexpr uint16_t flag_direction{ 0x400 };
	static constexpr uint16_t flag_overflow{ 0x800 };
private:
	union Register
	{
		uint8_t byte[2];
		uint16_t word{};
	};

	PIC& _pic; // The pic connected to the cpu
	Bus& _bus;

	std::array<Register, std::to_underlying<Registers>(Registers::Count)> _regs{};
	std::array<Register, std::to_underlying<Segment_Registers>(Segment_Registers::Count)> _sregs{};

	size_t _hz{ 5000000 };
	size_t _last_tick{};

	bool _halted{};
	bool _sti_enable{};
	bool _zero_flag_check{};
public:
	class Instruction
	{
	private:
		CPU& _cpu;

		CPU_Opcode _rep{};
		CPU_Opcode _operation{};

		int _length{};

		uint16_t _data_seg{};

		bool _modrm{};

		union Data
		{
			uint16_t data16{};
			uint8_t data8[2];
		};

		Data _data1{};
		Data _data2{};

		bool _regmem_to_reg{}; // 1 = opposite

		bool _width{}; // 8bit = 0, 16bit = 1

		bool _data1_width{};
		bool _data2_width{};

		bool _default_seg{ true };

		uint16_t _segment{};
		uint16_t _offset{};
	public:
		Instruction(CPU& cpu);
		void exec();

		class Operand
		{
			CPU& _cpu;

			bool _width{}; // true = 16, false = 8
			std::variant<uint8_t*, uint16_t*> _operand{};
		public:
			// operator overloading for instructions
			Operand(CPU& cpu, bool width);

			Operand(const Operand& operand) : _cpu { operand._cpu }
			{
				_width = operand._width;
				_operand = operand._operand;
			}

			void operator++(int val); // this would be inc...
			void operator--(int val); // this would be dec...
			void operator=(uint16_t value); // this would be mov...
			void operator=(Data value);
			void operator=(Operand value);
			void operator+=(uint16_t value); // this would be add...
			void operator+=(Data value);
			void operator+=(Operand value);
			void operator-=(uint16_t value); // this would be sub...
			void operator-=(Data value);
			void operator-=(Operand value);
			void operator^=(uint16_t value); // this would be xor...
			void operator^=(Data value);
			void operator^=(Operand value);
			void operator&=(uint16_t value); // this would be and...
			void operator&=(Data value);
			void operator&=(Operand value);
			void operator|=(uint16_t value); // this would be or...
			void operator|=(Data value);
			void operator|=(Operand value);
			void operator>>=(uint8_t value); // this would be shr...
			void operator>>=(Data value);
			void operator>>=(Operand value);
			void sar(uint8_t value); // this would be sar...
			void sar(Data value);
			void sar(Operand value);
			void operator<<=(uint8_t value); // this would be shl/sal...
			void operator<<=(Data value);
			void operator<<=(Operand value);
			void rol(uint8_t value); // this would be rol...
			void rol(Data value);
			void rol(Operand value);
			void ror(uint8_t value); // this would be ror/rcr...
			void ror(Data value);
			void ror(Operand value);
			void rcl(uint8_t value); // this would be rcl...
			void rcl(Data value);
			void rcl(Operand value);
			void rcr(uint8_t value); // this would be rcr...
			void rcr(Data value);
			void rcr(Operand value);
			void mul(); // this would be mul...
			void div(); // this would be div
			void imul(); // this would be imul...
			void imul(uint16_t value1, uint16_t value2); // this would be imul (3 operands)...
			void imul(Data value);
			void idiv(); // this would be idiv..
			void neg(); // this would be neg..

			uint16_t get_val16()
			{
				if (std::get<1>(_operand) < reinterpret_cast<uint16_t*>(0x100000))
				{
					return _cpu._bus.read16(reinterpret_cast<int>(std::get<1>(_operand)), false);
				}

				else
				{
					return *(std::get<1>(_operand));
				}
			}

			uint8_t get_val8()
			{
				if (std::get<0>(_operand) < reinterpret_cast<uint8_t*>(0x100000))
				{
					return _cpu._bus.read8(reinterpret_cast<int>(std::get<0>(_operand)), false);
				}

				else
				{
					return *std::get<0>(_operand);
				}
			}

			void write_val16(uint16_t value)
			{
				if (std::get<1>(_operand) < reinterpret_cast<uint16_t*>(0x100000))
				{
					_cpu._bus.write16(reinterpret_cast<int>(std::get<1>(_operand)), value, false);
				}

				else
				{
					*std::get<1>(_operand) = value;
				}
			}

			void write_val8(uint8_t value)
			{
				if (std::get<0>(_operand) < reinterpret_cast<uint8_t*>(0x100000))
				{
					_cpu._bus.write8(reinterpret_cast<int>(std::get<0>(_operand)), value, false);
				}

				else
				{
					*std::get<0>(_operand) = value;
				}
			}

			uint16_t*& get_addr16()
			{
				return std::get<1>(_operand);
			}

			uint8_t*& get_addr8()
			{
				return std::get<0>(_operand);
			}
		};

		void calc_modrm_byte(int modrm_address, bool sreg);

		private:
			// if modrm byte... (well _reg is ax/al by default)
			std::unique_ptr<Operand> _reg{ nullptr };
			std::unique_ptr<Operand> _regmem{ nullptr };
	};

	uint64_t ticks_total{};

	CPU(PIC& pic, Bus& bus);

	size_t get_hz() { return _hz; }
	void set_hz(size_t hz) { _hz = hz; }

	size_t get_last_tick() { return _last_tick; }
	void set_last_tick(size_t last_tick) { _last_tick = last_tick; }

	void reset();

	void push(uint16_t value);
	uint16_t pop();

	void modify_flag_carry(int old_val, int val, bool added, bool word);
	void modify_flag_parity(uint8_t val);
	void modify_flag_half_carry(uint8_t old_val, uint8_t new_val);
	void modify_flag_zero(uint16_t val);
	void modify_flag_sign(int16_t val, bool word);
	void modify_flag_overflow(int16_t op1, int16_t op2, int16_t result, bool word);

	uint16_t& get_reg16(Registers reg);
	uint8_t& get_reg8(Registers_8 reg);
	uint16_t& get_sreg(Segment_Registers reg);

	void check_irq();
};