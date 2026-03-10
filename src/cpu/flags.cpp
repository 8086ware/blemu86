#include "cpu.h"

void CPU::modify_flag_carry(int old_val, int val, bool added, bool word)
{
	if (word)
	{
		if ((old_val + val > USHRT_MAX && added) || (old_val - val < 0 && !added))
		{
			get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			get_reg16(Registers::Flags) &= ~flag_carry;
		}
	}

	else
	{
		if ((old_val + val > UCHAR_MAX && added) || (old_val - val < 0 && !added))
		{
			get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			get_reg16(Registers::Flags) &= ~flag_carry;
		}
	}
}

// Sees if amount of bits set is even (on) or odd (off)
// only least significant byte so its fine passing 16 bit value

void CPU::modify_flag_parity(uint8_t val)
{
	int bit_amount{};

	for (int i = 0; i < 8; i++)
	{
		if (val & (1 << i))
		{
			bit_amount++;
		}
	}

	if (bit_amount % 2 == 0)
	{
		get_reg16(Registers::Flags) |= flag_parity;
	}

	else
	{
		get_reg16(Registers::Flags) &= ~flag_parity;
	}
}

// Sees if there is a new bit set in the 4th bit of a value (lsb on 16 bit values, so it works)

void CPU::modify_flag_half_carry(uint8_t old_val, uint8_t new_val)
{
	if (old_val < 0x10 && new_val >= 0x10)
	{
		get_reg16(Registers::Flags) |= flag_half_carry;
	}

	else
	{
		get_reg16(Registers::Flags) &= ~flag_half_carry;
	}
}

// Sees if the new value is 0 (works on 8 bit and 16 bit)

void CPU::modify_flag_zero(uint16_t val)
{
	if (val == 0)
	{
		get_reg16(Registers::Flags) |= flag_zero;
	}

	else
	{
		get_reg16(Registers::Flags) &= ~flag_zero;
	}
}

void CPU::modify_flag_sign(int16_t val, bool word)
{
	int8_t val8{ static_cast<int8_t>(val) };

	if (word)
	{
		if ((val & 0x8000))
		{
			get_reg16(Registers::Flags) |= flag_sign;
		}

		else
		{
			get_reg16(Registers::Flags) &= ~flag_sign;
		}
	}

	else
	{
		if ((val8 & 0x80))
		{
			get_reg16(Registers::Flags) |= flag_sign;
		}

		else
		{
			get_reg16(Registers::Flags) &= ~flag_sign;
		}
	}
}

// assume subtraction (like cmp) between op1 and op2, for addition change the operands as needed
void CPU::modify_flag_overflow(int16_t op1, int16_t op2, int16_t result, bool word)
{
	bool op1_neg{};
	bool op2_neg{};
	bool result_neg{};

	if (word)
	{
		op1_neg = op1 & 0x8000;
		op2_neg = op2 & 0x8000;
		result_neg = result & 0x8000;
	}

	else
	{
		int8_t op1_8{ static_cast<int8_t>(op1) };
		int8_t op2_8{ static_cast<int8_t>(op2) };
		int8_t result_8{ static_cast<int8_t>(result) };

		op1_neg = op1_8 & 0x80;
		op2_neg = op2_8 & 0x80;
		result_neg = result_8 & 0x80;
	}

	// -127 to 127 (example)
	// -127 - 127 = 2
	// op1_neg - op2_neg

	if (op1_neg && !op2_neg && !result_neg)
	{
		get_reg16(Registers::Flags) |= flag_overflow;
	}

	else if (!op1_neg && op2_neg && result_neg)
	{
		get_reg16(Registers::Flags) |= flag_overflow;
	}

	else
	{
		get_reg16(Registers::Flags) &= ~flag_overflow;
	}
}