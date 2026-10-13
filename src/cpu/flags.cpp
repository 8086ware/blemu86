#include "cpu.h"
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
		get_reg(Registers::Flags) |= flag_parity;
	}

	else
	{
		get_reg(Registers::Flags) &= ~flag_parity;
	}
}

// Sees if there is a new bit set in the 4th bit of a value (lsb on 16 bit values, so it works)

void CPU::modify_flag_half_carry(uint8_t old_val, uint8_t new_val)
{
	if (old_val < 0x10 && new_val >= 0x10)
	{
		get_reg(Registers::Flags) |= flag_half_carry;
	}

	else
	{
		get_reg(Registers::Flags) &= ~flag_half_carry;
	}
}
