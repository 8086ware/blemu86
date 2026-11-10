#pragma once
#include "cpu.h"

template<CPU_Width T> auto&& Operand<T>::operator++(this auto&& self, int val)
{
	auto old{ self.get_val() };

	self = (self.get_val() + 1);

	self._cpu.modify_flag_overflow(old, static_cast<T>(-1), self.get_val());
	self._cpu.modify_flag_half_carry(static_cast<uint8_t>(old), self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator--(this auto&& self, int val)
{
	auto old{ self.get_val() };

	self = (self.get_val() - 1);

	self._cpu.modify_flag_overflow(old, static_cast<T>(1), self.get_val());
	self._cpu.modify_flag_half_carry(old, self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator+=(this auto&& self, T value)
{
	auto old{ self.get_val() };

	self = (self.get_val() + value);

	self._cpu.modify_flag_carry(old, self.get_val(), true);
	self._cpu.modify_flag_overflow(old, static_cast<T>(-value), self.get_val());
	self._cpu.modify_flag_half_carry(old, self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator-=(this auto&& self, T value)
{
	auto old{ self.get_val() };

	self = (self.get_val() - value);

	self._cpu.modify_flag_carry(old, self.get_val(), false);
	self._cpu.modify_flag_overflow(old, static_cast<T>(value), self.get_val());
	self._cpu.modify_flag_half_carry(static_cast<uint8_t>(old), self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	self._cpu.modify_flag_sign(self.get_val());

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator&=(this auto&& self, T value)
{
	self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_overflow | CPU::flag_carry);

	self = (self.get_val() & value);
	self._cpu.modify_flag_sign(self.get_val());	
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator|=(this auto&& self, T value)
{
	self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_overflow | CPU::flag_carry);

	self = (self.get_val() | value);
	self._cpu.modify_flag_sign(self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator^=(this auto&& self, T value)
{
	self._cpu.get_reg(CPU::Registers::Flags) &= ~(CPU::flag_overflow | CPU::flag_carry);

	self = (self.get_val() ^ value);
	self._cpu.modify_flag_sign(self.get_val());
	self._cpu.modify_flag_zero(self.get_val());
	self._cpu.modify_flag_parity(self.get_val());
	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator<<=(this auto&& self, T value)
{
	if ((self.get_val() << (value - 1)) & (std::numeric_limits<T>::max() / 2 + 1))
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
		if ((self.get_val() & (std::numeric_limits<T>::max() / 2 + 1)) && (self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry))
		{
			self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_overflow;
		}
		else if(!(self.get_val() & (std::numeric_limits<T>::max() / 2 + 1)) && !(self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry))
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


template<CPU_Width T> auto&& Operand<T>::operator>>=(this auto&& self, T value)
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
		if (self.get_val() & (std::numeric_limits<T>::max() / 2 + 1))
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

template<CPU_Width T> auto&& Operand<T>::sar(this auto&& self, T value)
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

	bool msb{ static_cast<bool>(self.get_val() & (std::numeric_limits<T>::max() / 2 + 1)) };

	self = (self.get_val() >> value);

	for (int i{ (sizeof(T) * 8) - value }; i < sizeof(T) * 8; i++)
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

template<CPU_Width T> auto&& Operand<T>::neg(this auto&& self)
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

template<CPU_Width T> auto&& Operand<T>::div(this auto&& self)
{
	if constexpr(std::is_same_v<T, uint16_t>)
	{
		uint32_t val_to_divide{ static_cast<uint32_t>(self._cpu.get_reg(CPU::Registers::DX) << 16 | self._cpu.get_reg(CPU::Registers::AX)) }; // dx:ax

		self._cpu.get_reg(CPU::Registers::AX) = static_cast<uint16_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers::DX) = static_cast<uint16_t>(val_to_divide % self.get_val());
	}

	else if constexpr(std::is_same_v<T, uint8_t>)
	{
		uint16_t val_to_divide{ self._cpu.get_reg(CPU::Registers::AX) }; // dx:ax

		self._cpu.get_reg(CPU::Registers_8::AL) = static_cast<uint8_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers_8::AH) = static_cast<uint8_t>(val_to_divide % self.get_val());
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::mul(this auto&& self)
{
	if constexpr(std::is_same_v<T, uint16_t>)
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
	
	else if constexpr(std::is_same_v<T, uint8_t>)
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

template<CPU_Width T> auto&& Operand<T>::rol(this auto&& self, T value) // IMPLEMENauto OF FLAG
{
	if (std::rotl(self.get_val(), value - 1) & (std::numeric_limits<T>::max() / 2 + 1))
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

template<CPU_Width T> auto&& Operand<T>::ror(this auto&& self, T value) // IMPLEMENauto OF FLAG
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

template<CPU_Width T> auto&& Operand<T>::rcl(this auto&& self, T value)
{
	bool old_cf{ static_cast<bool>(self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry) };

	if (std::rotl(self.get_val(), value - 1) & (std::numeric_limits<T>::max() / 2 + 1))
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

template<CPU_Width T> auto&& Operand<T>::rcr(this auto&& self, T value)
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
		self = (self.get_val() | (std::numeric_limits<T>::max() / 2 + 1));
	}

	return self;
}


template<CPU_Width T> auto&& Operand<T>::imul(this auto&& self)
{
	if constexpr(std::is_same_v<T, uint16_t>)
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

	else if constexpr(std::is_same_v<T, uint8_t>)
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

template<CPU_Width T> auto&& Operand<T>::imul(this auto&& self, T value1, T value2) // imul, r16, rm16, imm8
{
	self = (value1 * value2);
	return self;
}

template<CPU_Width T> auto&& Operand<T>::idiv(this auto&& self)
{
	if constexpr(std::is_same_v<T, uint16_t>)
	{
		int32_t val_to_divide{ static_cast<int32_t>(self._cpu.get_reg(CPU::Registers::DX) << 16 | self._cpu.get_reg(CPU::Registers::AX)) }; // dx:ax

		self._cpu.get_reg(CPU::Registers::AX) = static_cast<int16_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers::DX) = static_cast<int16_t>(val_to_divide % self.get_val());
	}

	else if constexpr(std::is_same_v<T, uint8_t>)
	{
		int32_t val_to_divide{ self._cpu.get_reg(CPU::Registers::AX) }; // dx:ax

		self._cpu.get_reg(CPU::Registers_8::AL) = static_cast<int8_t>(val_to_divide / self.get_val());
		self._cpu.get_reg(CPU::Registers_8::AH) = static_cast<int8_t>(val_to_divide % self.get_val());
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::cmp(this auto&& self, T value)
{
	T temp{self.get_val()};
	self -= value;
	self = temp;
	return self;
}

template<CPU_Width T> auto&& Operand<T>::test(this auto&& self, T value)
{
	T temp{self.get_val()};
	self &= value;
	self = temp;
	return self;
}

template<CPU_Width T> auto&& Operand<T>::adc(this auto&& self, T value)
{
	T carry{static_cast<T>(self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry)};
	self += (value + carry);

	if(static_cast<T>(value + carry) == 0)
	{
		self._cpu.get_reg(CPU::Registers::Flags) |= CPU::flag_carry;
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::sbb(this auto&& self, T value)
{
	T carry{static_cast<T>(self._cpu.get_reg(CPU::Registers::Flags) & CPU::flag_carry)};
	self -= (value + carry);

	return self;
}
