#pragma once
#include <cstdint>
#include "chipset/pic.h"
#include <variant>
#include <chrono>
#include <array>
#include <utility>
#include "chipset/bus.h"
#include "cpu_concepts.h"

// emulates 186 feature 16 bit cpu

class CPU;
class Operand
{
public:
	// operator overloading for instructions
	Operand() {}

	auto&& operator++(this auto&& self, int val); // this would be inc...
	auto&& operator--(this auto&& self, int val); // this would be dec...
	auto&& operator+=(this auto&& self, auto value); // this would be add...
	auto&& operator-=(this auto&& self, auto value); // this would be sub...
	auto&& operator^=(this auto&& self, auto value); // this would be xor...
	auto&& operator&=(this auto&& self, auto value); // this would be and...
	auto&& operator|=(this auto&& self, auto value); // this would be or...
	auto&& operator>>=(this auto&& self, auto value); // this would be shr...
	auto&& sar(this auto&& self, auto value); // this would be sar...
	auto&& operator<<=(this auto&& self, auto value); // this would be shl/sal...
	auto&& rol(this auto&& self, auto value); // this would be rol...
	auto&& ror(this auto&& self, auto value); // this would be ror/rcr...
	auto&& rcl(this auto&& self, auto value); // this would be rcl...
	auto&& rcr(this auto&& self, auto value); // this would be rcr...
	auto&& mul(this auto&& self); // this would be mul...
	auto&& div(this auto&& self); // this would be div
	auto&& imul(this auto&& self); // this would be imul...
	auto&& imul(this auto&& self, auto value1, auto value2); // this would be imul (3 operands)...
	auto&& idiv(this auto&& self); // this would be idiv..
	auto&& neg(this auto&& self); // this would be neg..
};


template<CPU_Width T> class Op_Mem;
template<CPU_Width T> class Op_Imm;
template<CPU_Width T> class Op_Regmem;
template<CPU_Width T> class Op_Reg;

template<typename T> concept Op_Normal_Type = std::same_as<T, Op_Imm<uint8_t>> || std::same_as<T, Op_Imm<uint16_t>> || std::same_as<T, Op_Mem<uint8_t>> ||  std::same_as<T, Op_Mem<uint16_t>>;
template<typename T> concept Op_Type = Op_Normal_Type<T> || std::same_as<T, Op_Regmem<uint16_t>> || std::same_as<T, Op_Regmem<uint8_t>> || std::same_as<T, Op_Reg<uint16_t>> || std::same_as<T, Op_Reg<uint8_t>>;

class CPU 
{
	enum class CPU_Group_Opcode_8X : uint8_t
	{
		ADD = 0x0,// Opcode group 80
		OR = 0x1, // Opcode group 80
		ADC = 0x2, // Opcode group 80
		SBB = 0x3, // Opcode group 80
		AND = 0x4, // Opcode group 80
		SUB = 0x5, // Opcode group 80
		XOR = 0x6, // Opcode group 80
		CMP = 0x7, // Opcode group 80
	};


	enum class CPU_Group_Opcode_CX_DX : uint8_t
	{
		ROL = 0x0,
		ROR = 0x1,
		RCL = 0x2,
		RCR = 0x3,
		SAL = 0x4,
		SHR = 0x5,
		SAR = 0x7,
	};

	enum class CPU_Group_Opcode_F6_F7 : uint8_t
	{
		TEST = 0x0, // Opcode group F6
		NOT = 0x2, // Opcode group F6
		NEG = 0x3, // Opcode group F6
		MUL = 0x4, // Opcode group F6
		IMUL = 0x5,
		DIV = 0x6,
		IDIV = 0x7, // Opcode group F6
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

		AAA = 0x37,
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

	std::array<Register, std::to_underlying(Registers::Count)> _regs{};
	std::array<Register, std::to_underlying(Segment_Registers::Count)> _sregs{};

	size_t _hz{ 5000000 };
	size_t _last_tick{};

	bool _halted{};
	bool _sti_enable{};
	bool _zero_flag_check{};
public:
	Bus& _bus;

	uint64_t ticks_total{};

	CPU(PIC& pic, Bus& bus);

	size_t get_hz() const { return _hz; }
	void set_hz(size_t hz) { _hz = hz; }

	size_t get_last_tick() const { return _last_tick; }
	void set_last_tick(size_t last_tick) { _last_tick = last_tick; }

	void reset();

	void push(uint16_t value);
	uint16_t pop();

	template<CPU_Width T> void modify_flag_carry(T old_val, T val, bool added);
	void modify_flag_parity(uint8_t val);
	void modify_flag_half_carry(uint8_t old_val, uint8_t new_val);
	template<CPU_Width T> void modify_flag_zero(T val);
	template<CPU_Width T> void modify_flag_sign(T val);
	template<CPU_Width T> void modify_flag_overflow(T op1, T op2, T result);

	uint16_t& get_reg(Registers reg);
	uint8_t& get_reg(Registers_8 reg);

	uint16_t& get_sreg(Segment_Registers reg);

	void check_irq();

	void logic();
	
	template<CPU_Width T, bool sreg = false> std::pair<Op_Regmem<T>, Op_Reg<T>> fetch_modrm(bool default_seg, uint16_t data_seg);
	template<Op_Normal_Type O> O fetch(uint16_t data_seg);
};

template<CPU_Width T> class Op_Mem : public Operand
{
	int _addr;
public:
	CPU& _cpu;
	Op_Mem(CPU& cpu, int addr) : _cpu{cpu}, _addr{addr} {}
	operator T() { return get_val(); }
	Op_Mem& operator=(T value) { _cpu._bus.write<T>(_addr, value, false); return *this; }
	T get_val() { return _cpu._bus.read<T>(_addr, false); }
	int get_op() const { return _addr; }
};

template<CPU_Width T> class Op_Imm : public Operand
{
	T _imm;
public:
	CPU& _cpu;
	operator T() { return get_val(); }
	Op_Imm(CPU& cpu, T imm) : _cpu{cpu}, _imm{imm} {}
	Op_Imm& operator=(T value) { _imm = value; return *this; }
	T get_val() const { return _imm; }
};

template<CPU_Width T> class Op_Reg : public Operand
{
	T* _reg;
public:
	CPU& _cpu;
	operator T() { return get_val(); }
	Op_Reg(CPU& cpu, T* reg) : _cpu{cpu}, _reg{reg} {}
	Op_Reg& operator=(T value) { *_reg = value; return *this; }
	T get_val() { return *_reg; }
	T* get_op() const { return _reg; }
};

template<CPU_Width T> class Op_Regmem : public Operand
{
	std::variant<int, T*> _regmem;
public:
	CPU& _cpu;
	operator T() { return get_val(); }
	Op_Regmem(CPU& cpu, std::variant<int, T*> regmem) : _cpu{cpu}, _regmem{regmem} {}
	Op_Regmem& operator=(T value)
	{
		if (std::holds_alternative<T*>(_regmem))
		{
			*std::get<T*>(_regmem) = value;
		}

		else
		{
			_cpu._bus.write<T>(std::get<int>(_regmem), value, false);
		}

		return *this;
	}

	T get_val()
	{
		if(std::holds_alternative<T*>(_regmem))
		{
			return *std::get<T*>(_regmem);
		}

		else
		{
			return _cpu._bus.read<T>(std::get<int>(_regmem), false);
		}
	}
	std::variant<int, T*> get_op() const { return _regmem; }
};

#include "operand.inl"
#include "fetch.inl"
#include "flags.inl"
