#pragma once
#include "cpu.h"
#include "get_bytes.h"
#include <bit>
#include <cmath>

template<CPU_Width T> auto&& Operand<T>::operator++(this auto&& self, int val)
{
	bool old_cf{self._cpu.get_flags().test(CPU::flag_carry)};

	self += 1;

	self._cpu.get_flags().set(CPU::flag_carry, old_cf);

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator--(this auto&& self, int val)
{
	bool old_cf{self._cpu.get_flags().test(CPU::flag_carry)};

	self -= 1;

	self._cpu.get_flags().set(CPU::flag_carry, old_cf);

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator+=(this auto&& self, T value)
{
	auto old{ self.get_val() };

	self = (self.get_val() + value);

	self._cpu.get_flags().set(CPU::flag_carry, old > self.get_val());
	self._cpu.get_flags().set(CPU::flag_overflow, (!(get_hibyte(old) & 0x80) && !(get_hibyte(value) & 0x80) && (get_hibyte(self.get_val()) & 0x80))
	|| ((get_hibyte(old) & 0x80) && (get_hibyte(value) & 0x80) && !(get_hibyte(self.get_val()) & 0x80)));
	self._cpu.get_flags().set(CPU::flag_half_carry, (old & 0x0f) > (self.get_val() & 0x0f));
	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator-=(this auto&& self, T value)
{
	auto old{ self.get_val() };

	self = (self.get_val() - value);

	self._cpu.get_flags().set(CPU::flag_carry, old < self.get_val());
	self._cpu.get_flags().set(CPU::flag_overflow, ((get_hibyte(old) & 0x80) && !(get_hibyte(value) & 0x80) && !(get_hibyte(self.get_val()) & 0x80))
	|| (!(get_hibyte(old) & 0x80) && (get_hibyte(value) & 0x80) && (get_hibyte(self.get_val()) & 0x80)));
	self._cpu.get_flags().set(CPU::flag_half_carry, (old & 0x0f) < (self.get_val() & 0x0f));
	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);

	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator&=(this auto&& self, T value)
{
	self._cpu.get_flags().set(CPU::flag_carry, false);
	self._cpu.get_flags().set(CPU::flag_overflow, false);

	self = (self.get_val() & value);

	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);
	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator|=(this auto&& self, T value)
{
	self._cpu.get_flags().set(CPU::flag_carry, false);
	self._cpu.get_flags().set(CPU::flag_overflow, false);

	self = (self.get_val() | value);
	
	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);
	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator^=(this auto&& self, T value)
{
	self._cpu.get_flags().set(CPU::flag_carry, false);
	self._cpu.get_flags().set(CPU::flag_overflow, false);

	self = (self.get_val() ^ value);

	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);
	return self;
}

template<CPU_Width T> auto&& Operand<T>::operator<<=(this auto&& self, T value)
{
	if(value > 0)
	{
		if ((self.get_val() << (value - 1)) & (std::numeric_limits<T>::max() / 2 + 1))
		{
			self._cpu.get_flags().set(CPU::flag_carry, true);
		}
		
		else
		{
			self._cpu.get_flags().set(CPU::flag_carry, false);
		}
	}
	self = (self.get_val() << value);
	
	if (value == 1)
	{
		if ((self.get_val() & (std::numeric_limits<T>::max() / 2 + 1)) && (self._cpu.get_flags().test(CPU::flag_carry)))
		{
			self._cpu.get_flags().set(CPU::flag_overflow, true);
		}
		else if(!(self.get_val() & (std::numeric_limits<T>::max() / 2 + 1)) && !(self._cpu.get_flags().test(CPU::flag_carry)))
		{
			self._cpu.get_flags().set(CPU::flag_overflow, true);
		}
		else
		{
			self._cpu.get_flags().set(CPU::flag_overflow, false);
		}
	}

	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);

	return self;
}


template<CPU_Width T> auto&& Operand<T>::operator>>=(this auto&& self, T value)
{
	if(value > 0)
	{
		if ((self.get_val() >> (value - 1)) & 0x1)
		{
			self._cpu.get_flags().set(CPU::flag_carry, true);
		}

		else
		{
			self._cpu.get_flags().set(CPU::flag_carry, false);
		}
	}

	if (value == 1)
	{
		if (self.get_val() & (std::numeric_limits<T>::max() / 2 + 1))
		{
			self._cpu.get_flags().set(CPU::flag_overflow, true);
		}
		
		else
		{
			self._cpu.get_flags().set(CPU::flag_overflow, false);
		}
	}

	self = (self.get_val() >> value);

	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);
	
	return self;
}

template<CPU_Width T> auto&& Operand<T>::sar(this auto&& self, T value)
{
	if(value > 0)
	{
		if ((self.get_val() >> (value - 1)) & 0x1)
		{
			self._cpu.get_flags().set(CPU::flag_carry, true);
		}

		else
		{
			self._cpu.get_flags().set(CPU::flag_carry, false);
		}
	}

	if (value == 1)
	{
		self._cpu.get_flags().set(CPU::flag_overflow, false);
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

	self._cpu.get_flags().set(CPU::flag_zero, self.get_val() == 0);
	self._cpu.get_flags().set(CPU::flag_parity, std::popcount(get_lobyte(self.get_val())) % 2 == 0);
	self._cpu.get_flags().set(CPU::flag_sign, get_hibyte(self.get_val()) & 0x80);
	return self;
}

template<CPU_Width T> auto&& Operand<T>::neg(this auto&& self)
{
	T old{self.get_val()};
	
	self = 0;

	self -= old;
	
	if (old != 0)
	{
		self._cpu.get_flags().set(CPU::flag_carry, true);
	}

	else
	{
		self._cpu.get_flags().set(CPU::flag_carry, false);
	}


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
			self._cpu.get_flags().set(CPU::flag_carry, false);
			self._cpu.get_flags().set(CPU::flag_overflow, false);
		}

		else
		{
			self._cpu.get_flags().set(CPU::flag_carry, true);
			self._cpu.get_flags().set(CPU::flag_overflow, true);
		}
	}
	
	else if constexpr(std::is_same_v<T, uint8_t>)
	{
		self._cpu.get_reg(CPU::Registers::AX) = self._cpu.get_reg(CPU::Registers_8::AL) * self.get_val();

		if (self._cpu.get_reg(CPU::Registers_8::AH) == 0)
		{
			self._cpu.get_flags().set(CPU::flag_carry, false);
			self._cpu.get_flags().set(CPU::flag_overflow, false);
		}

		else
		{
			self._cpu.get_flags().set(CPU::flag_carry, true);
			self._cpu.get_flags().set(CPU::flag_overflow, true);
		}
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::rol(this auto&& self, T value) // IMPLEMENauto OF FLAG
{
	T og_val{value};

	self = std::rotl(self.get_val(), value);

	if(og_val != 0)
	{
		self._cpu.get_flags().set(CPU::flag_carry, get_lobyte(self.get_val()) & 0x1);
	}

	if(og_val == 1)
	{
		self._cpu.get_flags().set(CPU::flag_overflow, !(get_hibyte(self.get_val()) & 0x80) != !(self._cpu.get_flags().test(CPU::flag_carry)));
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::ror(this auto&& self, T value) // IMPLEMENauto OF FLAG
{
	T og_val{value};

	self = std::rotr(self.get_val(), value);

	if(og_val != 0)
	{
		self._cpu.get_flags().set(CPU::flag_carry, get_hibyte(self.get_val()) & 0x80);
	}

	if(og_val == 1)
	{
		self._cpu.get_flags().set(CPU::flag_overflow, !(get_hibyte(self.get_val()) & 0x40) != !(get_hibyte(self.get_val()) & 0x80));
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::rcl(this auto&& self, T value)
{
	T og_val{value};

	while(value != 0)
	{
		bool temp_cf{get_hibyte(self.get_val()) & 0x80};
		self = (self.get_val() * 2) + self._cpu.get_flags().test(CPU::flag_carry);
		self._cpu.get_flags().set(CPU::flag_carry, temp_cf);
		value--;
	}

	if(og_val == 1)
	{
		self._cpu.get_flags().set(CPU::flag_overflow, !(get_hibyte(self.get_val()) & 0x80) != !(self._cpu.get_flags().test(CPU::flag_carry)));
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::rcr(this auto&& self, T value)
{
	T og_val{value};

	if(og_val == 1)
	{
		self._cpu.get_flags().set(CPU::flag_overflow, !(get_hibyte(self.get_val()) & 0x80) != !(self._cpu.get_flags().test(CPU::flag_carry)));
	}
	
	while(value != 0)
	{
		bool temp_cf{get_lobyte(self.get_val()) & 0x1};
		self = (self.get_val() / 2) + (self._cpu.get_flags().test(CPU::flag_carry) * pow(2,sizeof(T) * 8));
		self._cpu.get_flags().set(CPU::flag_carry, temp_cf);
		value--;
	}

	return self;
}


template<CPU_Width T> auto&& Operand<T>::imul(this auto&& self)
{
	if constexpr(std::is_same_v<T, uint16_t>)
	{
		int32_t product{ static_cast<int32_t>(static_cast<int16_t>(self._cpu.get_reg(CPU::Registers::AX)) * static_cast<int16_t>(self.get_val())) };

		self._cpu.get_reg(CPU::Registers::DX) = static_cast<int16_t>(product >> 16);
		self._cpu.get_reg(CPU::Registers::AX) = static_cast<int16_t>(product & 0x0000ffff);

		if (self._cpu.get_reg(CPU::Registers::DX) == 0 || self._cpu.get_reg(CPU::Registers::DX) == 0xffff)
		{
			self._cpu.get_flags().set(CPU::flag_carry, false);
			self._cpu.get_flags().set(CPU::flag_overflow, false);
		}

		else
		{
			self._cpu.get_flags().set(CPU::flag_carry, true);
			self._cpu.get_flags().set(CPU::flag_overflow, true);
		}
	}

	else if constexpr(std::is_same_v<T, uint8_t>)
	{
		self._cpu.get_reg(CPU::Registers::AX) = static_cast<int16_t>(static_cast<int8_t>(self._cpu.get_reg(CPU::Registers_8::AL)) * static_cast<int8_t>(self.get_val()));

		if (self._cpu.get_reg(CPU::Registers_8::AH) == 0 || self._cpu.get_reg(CPU::Registers_8::AH) == 0xff)
		{
			self._cpu.get_flags().set(CPU::flag_carry, false);
			self._cpu.get_flags().set(CPU::flag_overflow, false);
		}

		else
		{
			self._cpu.get_flags().set(CPU::flag_carry, true);
			self._cpu.get_flags().set(CPU::flag_overflow, true);
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
	T carry{ static_cast<T>(self._cpu.get_flags().test(CPU::flag_carry)) };
	self += (value + carry);

	if(static_cast<T>(value + carry) == 0 && value == 0xff)
	{
		self._cpu.get_flags().set(CPU::flag_carry, true);
	}

	return self;
}

template<CPU_Width T> auto&& Operand<T>::sbb(this auto&& self, T value)
{
	T carry{ static_cast<T>(self._cpu.get_flags().test(CPU::flag_carry)) };
	self -= (value + carry);

	if(static_cast<T>(value + carry) == 0 && value == 0xff)
	{
		self._cpu.get_flags().set(CPU::flag_carry, true);
	}

	return self;
}
