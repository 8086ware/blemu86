#pragma once
#include "cpu.h"
#include <limits>

template<CPU_Width T> void CPU::modify_flag_carry(T old_val, T val, bool added)
{
	if ((static_cast<int>(old_val + val) > std::numeric_limits<T>::max() && added) || (static_cast<int>(old_val - val) < 0 && !added))
	{
		get_reg(Registers::Flags) |= flag_carry;
	}

	else
	{
		get_reg(Registers::Flags) &= ~flag_carry;
	}
}

template<CPU_Width T> void CPU::modify_flag_sign(T val)
{
	if ((val & ((std::numeric_limits<T>::max() / 2 + 1))))
	{
		get_reg(Registers::Flags) |= flag_sign;
	}

	else
	{
		get_reg(Registers::Flags) &= ~flag_sign;
	}
}

// assume subtraction (like cmp) between op1 and op2, for addition change the operands as needed
template<CPU_Width T> void CPU::modify_flag_overflow(T op1, T op2, T result)
{
	bool op1_neg{};
	bool op2_neg{};
	bool result_neg{};

	op1_neg = op1 & ((std::numeric_limits<T>::max() / 2 + 1));
	op2_neg = op2 & ((std::numeric_limits<T>::max() / 2 + 1));
	result_neg = result & ((std::numeric_limits<T>::max() / 2 + 1));
	// -127 to 127 (example)
	// -127 - 127 = 2
	// op1_neg - op2_neg

	if (op1_neg && !op2_neg && !result_neg)
	{
		get_reg(Registers::Flags) |= flag_overflow;
	}

	else if (!op1_neg && op2_neg && result_neg)
	{
		get_reg(Registers::Flags) |= flag_overflow;
	}

	else
	{
		get_reg(Registers::Flags) &= ~flag_overflow;
	}
}

template<CPU_Width T> void CPU::modify_flag_zero(T val)
{
	if (val == 0)
	{
		get_reg(Registers::Flags) |= flag_zero;
	}

	else
	{
		get_reg(Registers::Flags) &= ~flag_zero;
	}
}
