#pragma once
#include "cpu.h"

auto&& Operand::operator++(this auto&& self, int val)
{
	auto old{ self.get_val() };

	self = (self.get_val() + 1);

	self._cpu.modify_flag_overflow(old, static_cast<decltype(self.get_val())>(-1), self.get_val());
	self._cpu.modify_flag_half_carry(static_cast<uint8_t>(old), self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

auto&& Operand::operator--(this auto&& self, int val)
{
	auto old{ self.get_val() };

	self = (self.get_val() - 1);

	self._cpu.modify_flag_overflow(old, static_cast<decltype(self.get_val())>(1), self.get_val());
	self._cpu.modify_flag_half_carry(old, self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

auto&& Operand::operator+=(this auto&& self, auto value)
{
	auto old{ self.get_val() };

	self = (self.get_val() + value);

	self._cpu.modify_flag_carry(old, static_cast<decltype(self.get_val())>(value), true);
	self._cpu.modify_flag_overflow(old, static_cast<decltype(self.get_val())>(-value), self.get_val());
	self._cpu.modify_flag_half_carry(old, self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

auto&& Operand::operator-=(this auto&& self, auto value)
{
	auto old{ self.get_val() };

	self = (self.get_val() - value);

	self._cpu.modify_flag_carry(old, static_cast<decltype(self.get_val())>(value), false);
	self._cpu.modify_flag_overflow(old, static_cast<decltype(self.get_val())>(value), self.get_val());
	self._cpu.modify_flag_half_carry(static_cast<uint8_t>(old), self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

auto&& Operand::operator&=(this auto&& self, auto value)
{
	self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_overflow | CPU::flag_carry);

	self = (self.get_val() & value);
	self._cpu.modify_flag_sign(self.get_val());	
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	return self;
}

auto&& Operand::operator|=(this auto&& self, auto value)
{
	self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_overflow | CPU::flag_carry);

	self = (self.get_val() | value);
	self._cpu.modify_flag_sign(self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	return self;
}

auto&& Operand::operator^=(this auto&& self, auto value)
{
	self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_overflow | CPU::flag_carry);

	self = (self.get_val() ^ value);
	self._cpu.modify_flag_sign(self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	return self;
}

auto&& Operand::operator<<=(this auto&& self, auto value)
{
	if ((self.get_val() << (value - 1)) & (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1))
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}
	
	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	self = (self.get_val() << value);
	
	if (value == 1)
	{
		if ((self.get_val() & (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1)) && (self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry))
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_overflow;
		}
		else if(!(self.get_val() & (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1)) && !(self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry))
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_overflow;
		}
		else
		{
			self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_overflow;
		}
	}

	self._cpu.modify_flag_sign(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	
	return self;
}


auto&& Operand::operator>>=(this auto&& self, auto value)
{
	if ((self.get_val() >> (value - 1)) & 0x1)
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	if (value == 1)
	{
		if (self.get_val() & (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1))
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_overflow;
		}
		
		else
		{
			self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_overflow;
		}
	}

	self = (self.get_val() >> value);

	self._cpu.modify_flag_sign(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	
	return self;
}

auto&& Operand::sar(this auto&& self, auto value)
{
	if ((self.get_val() >> (value - 1)) & 0x1)
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	if (value == 1)
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_overflow;
	}

	bool msb{ static_cast<bool>(self.get_val() & (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1)) };

	self = (self.get_val() >> value);

	for (int i{ (sizeof(decltype(self.get_val())) * 8) - value }; i < sizeof(decltype(self.get_val())) * 8; i++)
	{
		if (msb)
		{
			self = (self.get_val() | (1 << i));
		}

		else
		{
			self = (self.get_val() & ~(1 << i));
		}
	}

	self._cpu.modify_flag_sign(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_zero(self.get_val());

	return self;
}

auto&& Operand::neg(this auto&& self)
{
	if (self.get_val() != 0)
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	self = (-self.get_val());
	
	return self;
}

auto&& Operand::div(this auto&& self)
{
	if constexpr(std::is_same_v<decltype(self.get_val()), uint16_t>)
	{
		uint32_t val_to_divide{ static_cast<uint32_t>(self._cpu.get_reg(CPU::Registers::DX) << 16 | self._cpu.get_reg(CPU::Registers::AX)) }; // dx:ax

		self._cpu.get_reg(CPU::Registers::AX) = static_cast<uint16_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers::DX) = static_cast<uint16_t>(val_to_divide % self.get_val());
	}

	else if constexpr(std::is_same_v<decltype(self.get_val()), uint8_t>)
	{
		uint16_t val_to_divide{ self._cpu.get_reg(CPU::Registers::AX) }; // dx:ax

		self._cpu.get_reg(CPU::Registers_8::AL) = static_cast<uint8_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers_8::AH) = static_cast<uint8_t>(val_to_divide % self.get_val());
	}

	return self;
}

auto&& Operand::mul(this auto&& self)
{
	if constexpr(std::is_same_v<decltype(self.get_val()), uint16_t>)
	{
		uint32_t product{ static_cast<uint32_t>(self._cpu.get_reg(CPU::Registers::AX) * self.get_val()) };

		self._cpu.get_reg(CPU::Registers::DX) = static_cast<uint16_t>(product >> 16);
		self._cpu.get_reg(CPU::Registers::AX) = static_cast<uint16_t>(product & 0x0000ffff);

		if (self._cpu.get_reg(CPU::Registers::DX) == 0)
		{
			self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_carry | CPU::flag_overflow);
		}

		else
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= (CPU::flag_carry | CPU::flag_overflow);
		}
	}
	
	else if constexpr(std::is_same_v<decltype(self.get_val()), uint8_t>)
	{
		self._cpu.get_reg(CPU::Registers::AX) = self._cpu.get_reg(CPU::Registers_8::AL) * self.get_val();

		if (self._cpu.get_reg(CPU::Registers_8::AH) == 0)
		{
			self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_carry | CPU::flag_overflow);
		}

		else
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= (CPU::flag_carry | CPU::flag_overflow);
		}
	}

	return self;
}

auto&& Operand::rol(this auto&& self, auto value) // IMPLEMENauto OF FLAG
{
	if (std::rotl(self.get_val(), value - 1) & (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1))
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	self = (std::rotl(self.get_val(), value));

	return self;
}

auto&& Operand::ror(this auto&& self, auto value) // IMPLEMENauto OF FLAG
{
	if (std::rotr(self.get_val(), value - 1) & 0x1)
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	self = (std::rotr(self.get_val(), value));

	return self;
}

auto&& Operand::rcl(this auto&& self, auto value)
{
	bool old_cf{ static_cast<bool>(self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry) };

	if (std::rotl(self.get_val(), value - 1) & (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1))
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	self = (std::rotl(self.get_val(), value));

	if (old_cf)
	{
		self = (self.get_val() | 0x1);
	}

	return self;
}

auto&& Operand::rcr(this auto&& self, auto value)
{
	bool old_cf{ static_cast<bool>(self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry) };

	if (std::rotr(self.get_val(), value - 1) & 0x1)
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	else
	{
		self._cpu.get_reg(CPU::Registers::Flags) &= ~CPU::flag_carry;
	}

	self = (std::rotr(self.get_val(), value));

	if (old_cf)
	{
		self = (self.get_val() | (std::numeric_limits<decltype(self.get_val())>::max() / 2 + 1));
	}

	return self;
}


auto&& Operand::imul(this auto&& self)
{
	if constexpr(std::is_same_v<decltype(self.get_val()), uint16_t>)
	{
		int32_t product{ static_cast<int16_t>(self._cpu.get_reg(CPU::Registers::AX)) * static_cast<int16_t>(self.get_val()) };

		self._cpu.get_reg(CPU::Registers::DX) = static_cast<int16_t>(product >> 16);
		self._cpu.get_reg(CPU::Registers::AX) = static_cast<int16_t>(product & 0x0000ffff);

		if (self._cpu.get_reg(CPU::Registers::DX) == 0 || self._cpu.get_reg(CPU::Registers::DX) == 0xffff)
		{
			self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_carry | CPU::flag_overflow);
		}

		else
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= (CPU::flag_carry | CPU::flag_overflow);
		}
	}

	else if constexpr(std::is_same_v<decltype(self.get_val()), uint8_t>)
	{
		self._cpu.get_reg(CPU::Registers::AX) = static_cast<int8_t>(self._cpu.get_reg(CPU::Registers_8::AL)) * static_cast<int8_t>(self.get_val());

		if (self._cpu.get_reg(CPU::Registers_8::AH) == 0 || self._cpu.get_reg(CPU::Registers_8::AH) == 0xff)
		{
			self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_carry | CPU::flag_overflow);
		}

		else
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= (CPU::flag_carry | CPU::flag_overflow);
		}
	}

	return self;
}

auto&& Operand::imul(this auto&& self, auto value1, auto value2) // imul, r16, rm16, imm8
{
	self = (value1 * value2);
	return self;
}

auto&& Operand::idiv(this auto&& self)
{
	if constexpr(std::is_same_v<decltype(self.get_val()), uint16_t>)
	{
		int32_t val_to_divide{ static_cast<int32_t>(self._cpu.get_reg(CPU::Registers::DX) << 16 | self._cpu.get_reg(CPU::Registers::AX)) }; // dx:ax

		self._cpu.get_reg(CPU::Registers::AX) = static_cast<int16_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers::DX) = static_cast<int16_t>(val_to_divide % self.get_val());
	}

	else if constexpr(std::is_same_v<decltype(self.get_val()), uint8_t>)
	{
		int32_t val_to_divide{ self._cpu.get_reg(CPU::Registers::AX) }; // dx:ax

		self._cpu.get_reg(CPU::Registers_8::AL) = static_cast<int8_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers_8::AH) = static_cast<int8_t>(val_to_divide % self.get_val());
	}

	return self;
}
