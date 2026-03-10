#include "cpu.h"
#include <utility>
CPU::Instruction::Operand::Operand(CPU& cpu, bool width) : _cpu{ cpu }, _width{ width }
{
	if (width)
	{
		_operand.emplace<1>(nullptr);
	}

	else
	{
		_operand.emplace<0>(nullptr);
	}
}

void CPU::Instruction::Operand::operator++(int val)
{
	if (_width)
	{
		uint16_t old{ get_val16() };

		write_val16(get_val16() + 1);

		_cpu.modify_flag_overflow(old, -1, get_val16(), _width);
		_cpu.modify_flag_half_carry(static_cast<uint8_t>(old), get_val16());
		_cpu.modify_flag_zero(get_val16());
		_cpu.modify_flag_parity(get_val16());
		_cpu.modify_flag_sign(get_val16(), _width);
	}

	else 
	{
		uint8_t old{ get_val8() };

		write_val8(get_val8() + 1);

		_cpu.modify_flag_overflow(static_cast<int16_t>(old), -1, get_val8(), _width);
		_cpu.modify_flag_half_carry(old, get_val8());
		_cpu.modify_flag_zero(get_val8());
		_cpu.modify_flag_parity(get_val8());
		_cpu.modify_flag_sign(get_val8(), _width);
	}
}

void CPU::Instruction::Operand::operator--(int val)
{
	if (_width)
	{
		uint16_t old{ get_val16() };

		write_val16(get_val16() - 1);

		_cpu.modify_flag_overflow(old, 1, get_val16(), _width);
		_cpu.modify_flag_half_carry(static_cast<uint8_t>(old), get_val16());
		_cpu.modify_flag_zero(get_val16());
		_cpu.modify_flag_parity(get_val16());
		_cpu.modify_flag_sign(get_val16(), _width);
	}

	else
	{
		uint8_t old{ get_val8() };

		write_val8(get_val8() - 1);

		_cpu.modify_flag_overflow(static_cast<int16_t>(old), 1, get_val8(), _width);
		_cpu.modify_flag_half_carry(old, get_val8());
		_cpu.modify_flag_zero(get_val8());
		_cpu.modify_flag_parity(get_val8());
		_cpu.modify_flag_sign(get_val8(), _width);
	}
}


void CPU::Instruction::Operand::operator=(uint16_t value)
{
	if (_width) { write_val16(value); }
	else { write_val8(static_cast<uint8_t>(value)); }
}


void CPU::Instruction::Operand::operator=(Data value)
{
	if (_width)
	{
		operator=(value.data16);
	}

	else
	{
		operator=(value.data8[0]);
	}
}

void CPU::Instruction::Operand::operator=(Operand value)
{
	if (_width)
	{
		operator=(value.get_val16());
	}

	else
	{
		operator=(value.get_val8());
	}
}

void CPU::Instruction::Operand::operator+=(uint16_t value)
{
	if (_width)
	{
		uint16_t old{ get_val16() };

		write_val16(get_val16() + value);

		_cpu.modify_flag_carry(old, value, true, _width);
		_cpu.modify_flag_overflow(old, -value, get_val16(), _width);
		_cpu.modify_flag_half_carry(static_cast<uint8_t>(old), get_val16());
		_cpu.modify_flag_zero(get_val16());
		_cpu.modify_flag_parity(get_val16());
		_cpu.modify_flag_sign(get_val16(), _width);
	}
	else 
	{
		uint8_t old{ get_val8() };

		write_val8(get_val8() + value);

		_cpu.modify_flag_carry(old, value, true, _width);
		_cpu.modify_flag_overflow(old, -value, get_val8(), _width);
		_cpu.modify_flag_half_carry(static_cast<uint8_t>(old), get_val8());
		_cpu.modify_flag_zero(get_val8());
		_cpu.modify_flag_parity(get_val8());
		_cpu.modify_flag_sign(get_val8(), _width);
	}
}

void CPU::Instruction::Operand::operator+=(Data value)
{
	if (_width)
	{
		operator+=(value.data16);
	}

	else
	{
		operator+=(value.data8[0]);
	}
}

void CPU::Instruction::Operand::operator+=(Operand value)
{
	if (_width)
	{
		operator+=(value.get_val16());
	}

	else
	{
		operator+=(value.get_val8());
	}
}

void CPU::Instruction::Operand::operator-=(uint16_t value)
{
	if (_width)
	{
		uint16_t old{ get_val16() };

		write_val16(get_val16() - value);

		_cpu.modify_flag_carry(old, value, false, _width);
		_cpu.modify_flag_overflow(old, value, get_val16(), _width);
		_cpu.modify_flag_half_carry(static_cast<uint8_t>(old), get_val16());
		_cpu.modify_flag_zero(get_val16());
		_cpu.modify_flag_parity(get_val16());
		_cpu.modify_flag_sign(get_val16(), _width);
	}

	else
	{
		uint8_t old{ get_val8() };

		write_val8(get_val8() - value);

		_cpu.modify_flag_carry(old, value, false, _width);
		_cpu.modify_flag_overflow(old, value, get_val8(), _width);
		_cpu.modify_flag_half_carry(static_cast<uint8_t>(old), get_val8());
		_cpu.modify_flag_zero(get_val8());
		_cpu.modify_flag_parity(get_val8());
		_cpu.modify_flag_sign(get_val8(), _width);
	}
}

void CPU::Instruction::Operand::operator-=(Data value)
{
	if (_width)
	{
		operator-=(value.data16);
	}

	else
	{
		operator-=(value.data8[0]);
	}
}

void CPU::Instruction::Operand::operator-=(Operand value)
{
	if (_width)
	{
		operator-=(value.get_val16());
	}

	else
	{
		operator-=(value.get_val8());
	}
}

void CPU::Instruction::Operand::operator&=(uint16_t value)
{
	_cpu.get_reg16(Registers::Flags) &= ~(flag_overflow | flag_carry);

	if (_width)
	{
		write_val16(get_val16() & value);
		_cpu.modify_flag_sign(get_val16(), _width);
		_cpu.modify_flag_zero(get_val16());
		_cpu.modify_flag_parity(get_val16());
	}

	else
	{
		write_val8(get_val8() & value);
		_cpu.modify_flag_sign(get_val8(), _width);
		_cpu.modify_flag_zero(get_val8());
		_cpu.modify_flag_parity(get_val8());
	}
}

void CPU::Instruction::Operand::operator&=(Data value)
{
	if (_width)
	{
		operator&=(value.data16);
	}

	else
	{
		operator&=(value.data8[0]);
	}
}

void CPU::Instruction::Operand::operator&=(Operand value)
{
	if (_width)
	{
		operator&=(value.get_val16());
	}

	else
	{
		operator&=(value.get_val8());
	}
}

void CPU::Instruction::Operand::operator|=(uint16_t value)
{
	_cpu.get_reg16(Registers::Flags) &= ~(flag_overflow | flag_carry);

	if (_width)
	{
		write_val16(get_val16() | value);
		_cpu.modify_flag_sign(get_val16(), _width);
		_cpu.modify_flag_zero(get_val16());
		_cpu.modify_flag_parity(get_val16());
	}

	else
	{
		write_val8(get_val8() | value);
		_cpu.modify_flag_sign(get_val8(), _width);
		_cpu.modify_flag_zero(get_val8());
		_cpu.modify_flag_parity(get_val8());
	}
}

void CPU::Instruction::Operand::operator|=(Data value)
{
	if (_width)
	{
		operator|=(value.data16);
	}

	else
	{
		operator|=(value.data8[0]);
	}
}

void CPU::Instruction::Operand::operator|=(Operand value)
{
	if (_width)
	{
		operator|=(value.get_val16());
	}

	else
	{
		operator|=(value.get_val8());
	}
}


void CPU::Instruction::Operand::operator^=(uint16_t value)
{
	_cpu.get_reg16(Registers::Flags) &= ~(flag_overflow | flag_carry);

	if (_width)
	{
		write_val16(get_val16() ^ value);
		_cpu.modify_flag_sign(get_val16(), _width);
		_cpu.modify_flag_zero(get_val16());
		_cpu.modify_flag_parity(get_val16());
	}

	else
	{
		write_val8(get_val8() ^ value);
		_cpu.modify_flag_sign(get_val8(), _width);
		_cpu.modify_flag_zero(get_val8());
		_cpu.modify_flag_parity(get_val8());
	}
}

void CPU::Instruction::Operand::operator^=(Data value)
{
	if (_width)
	{
		operator^=(value.data16);
	}

	else
	{
		operator^=(value.data8[0]);
	}
}

void CPU::Instruction::Operand::operator^=(Operand value)
{
	if (_width)
	{
		operator^=(value.get_val16());
	}

	else
	{
		operator^=(value.get_val8());
	}
}

void CPU::Instruction::Operand::operator<<=(uint8_t value)
{
	if (_width)
	{
		if ((get_val16() << (value - 1)) & 0x8000)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}
		
		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val16(get_val16() << value);

		if (value == 1)
		{
			if ((get_val16() & 0x8000) && (_cpu.get_reg16(Registers::Flags) & flag_carry))
			{
				_cpu.get_reg16(Registers::Flags) |= flag_overflow;
			}

			else if(!(get_val16() & 0x8000) && !(_cpu.get_reg16(Registers::Flags) & flag_carry))
			{
				_cpu.get_reg16(Registers::Flags) |= flag_overflow;
			}

			else
			{
				_cpu.get_reg16(Registers::Flags) &= ~flag_overflow;
			}
		}

		_cpu.modify_flag_sign(get_val16(), _width);
		_cpu.modify_flag_parity(get_val16());
		_cpu.modify_flag_zero(get_val16());
	}

	else
	{
		if ((get_val8() << (value - 1)) & 0x80)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val8(get_val8() << value);

		if (value == 1)
		{
			if (get_val8() & 0x80)
			{
				_cpu.get_reg16(Registers::Flags) |= flag_overflow;
			}

			else
			{
				_cpu.get_reg16(Registers::Flags) &= ~flag_overflow;
			}
		}

		_cpu.modify_flag_sign(get_val8(), _width);
		_cpu.modify_flag_parity(get_val8());
		_cpu.modify_flag_zero(get_val8());
	}
}

void CPU::Instruction::Operand::operator<<=(Data value)
{
	operator<<=(value.data8[0]);
}

void CPU::Instruction::Operand::operator<<=(Operand value)
{
	if (_width)
	{
		operator<<=(value.get_val16());
	}

	else
	{
		operator<<=(value.get_val8());
	}
}

void CPU::Instruction::Operand::operator>>=(uint8_t value)
{
	if (_width)
	{
		if ((get_val16() >> (value - 1)) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		if (value == 1)
		{
			if (get_val16() & 0x8000)
			{
				_cpu.get_reg16(Registers::Flags) |= flag_overflow;
			}

			else
			{
				_cpu.get_reg16(Registers::Flags) &= ~flag_overflow;
			}
		}

		write_val16(get_val16() >> value);

		_cpu.modify_flag_sign(get_val16(), _width);
		_cpu.modify_flag_parity(get_val16());
		_cpu.modify_flag_zero(get_val16());
	}

	else
	{
		if ((get_val8() >> (value - 1)) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		if (value == 1)
		{
			if (get_val8() & 0x80)
			{
				_cpu.get_reg16(Registers::Flags) |= flag_overflow;
			}

			else
			{
				_cpu.get_reg16(Registers::Flags) &= ~flag_overflow;
			}
		}

		write_val8(get_val8() >> value);

		_cpu.modify_flag_sign(get_val8(), _width);
		_cpu.modify_flag_parity(get_val8());
		_cpu.modify_flag_zero(get_val8());
	}
}

void CPU::Instruction::Operand::operator>>=(Data value)
{
	operator>>=(value.data8[0]);
}

void CPU::Instruction::Operand::operator>>=(Operand value)
{
	if (_width)
	{
		operator>>=(value.get_val16());
	}

	else
	{
		operator>>=(value.get_val8());
	}
}

void CPU::Instruction::Operand::sar(uint8_t value)
{
	if (_width)
	{
		if ((get_val16() >> (value - 1)) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		if (value == 1)
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_overflow;
		}

		bool msb{ static_cast<bool>(get_val16() & 0x8000) };

		write_val16(get_val16() >> value);

		for (int i{ 16 - value }; i < 16; i++)
		{
			if (msb)
			{
				write_val16(get_val16() | (1 << i));
			}

			else
			{
				write_val16(get_val16() & ~(1 << i));
			}
		}

		_cpu.modify_flag_sign(get_val16(), _width);
		_cpu.modify_flag_parity(get_val16());
		_cpu.modify_flag_zero(get_val16());
	}

	else
	{
		if ((get_val8() >> (value - 1)) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		if (value == 1)
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_overflow;
		}

		bool msb{ static_cast<bool>(get_val8() & 0x80)};

		write_val8(get_val8() >> value);

		for (int i{ 8 - value }; i < 8; i++)
		{
			if (msb)
			{
				write_val8(get_val8() | (1 << i));
			}

			else
			{
				write_val8(get_val8() & ~(1 << i));
			}
		}

		_cpu.modify_flag_sign(get_val8(), _width);
		_cpu.modify_flag_parity(get_val8());
		_cpu.modify_flag_zero(get_val8());
	}
}

void CPU::Instruction::Operand::sar(Data value)
{
	sar(value.data8[0]);
}

void CPU::Instruction::Operand::sar(Operand value)
{
	if (_width)
	{
		sar(value.get_val16());
	}

	else
	{
		sar(value.get_val8());
	}
}

void CPU::Instruction::Operand::neg()
{
	if (_width)
	{
		if (get_val16() != 0)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val16(-get_val16());
	}

	else
	{
		if (get_val8() != 0)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val8(-get_val8());
	}
}

void CPU::Instruction::Operand::div()
{
	if (_width)
	{
		uint32_t val_to_divide{ static_cast<uint32_t>(_cpu.get_reg16(Registers::DX) << 16 | _cpu.get_reg16(Registers::AX)) }; // dx:ax

		_cpu.get_reg16(Registers::AX) = static_cast<uint16_t>(val_to_divide / get_val16());
		_cpu.get_reg16(Registers::DX) = static_cast<uint16_t>(val_to_divide % get_val16());
	}

	else
	{
		uint16_t val_to_divide{ _cpu.get_reg16(Registers::AX) }; // dx:ax

		_cpu.get_reg8(Registers_8::AL) = static_cast<uint8_t>(val_to_divide / get_val8());
		_cpu.get_reg8(Registers_8::AH) = static_cast<uint8_t>(val_to_divide % get_val8());
	}
}

void CPU::Instruction::Operand::mul()
{
	if (_width)
	{
		uint32_t product{ static_cast<uint32_t>(_cpu.get_reg16(Registers::AX) * get_val16()) };

		_cpu.get_reg16(Registers::DX) = static_cast<uint16_t>(product >> 16);
		_cpu.get_reg16(Registers::AX) = static_cast<uint16_t>(product & 0x0000ffff);

		if (_cpu.get_reg16(Registers::DX) == 0)
		{
			_cpu.get_reg16(Registers::Flags) &= ~(flag_carry | flag_overflow);
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) |= (flag_carry | flag_overflow);
		}
	}
	
	else
	{
		_cpu.get_reg16(Registers::AX) = _cpu.get_reg8(Registers_8::AL) * get_val8();

		if (_cpu.get_reg8(Registers_8::AH) == 0)
		{
			_cpu.get_reg16(Registers::Flags) &= ~(flag_carry | flag_overflow);
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) |= (flag_carry | flag_overflow);
		}
	}
}

void CPU::Instruction::Operand::rol(uint8_t value) // IMPLEMENT OF FLAG
{
	if (_width)
	{
		if (std::rotl(get_val16(), value - 1) & 0x8000)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val16(std::rotl(get_val16(), value));
	}

	else
	{
		if (std::rotl(get_val8(), value - 1) & 0x80)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val8(std::rotl(get_val8(), value));
	}
}

void CPU::Instruction::Operand::rol(Data value)
{
	if (_width) { rol(value.data16); }
	else { rol(value.data8[0]); }
}

void CPU::Instruction::Operand::rol(Operand value)
{
	if (_width) { rol(value.get_val16()); }
	else { rol(value.get_val8()); }
}

void CPU::Instruction::Operand::ror(uint8_t value) // IMPLEMENT OF FLAG
{
	if (_width)
	{
		if (std::rotr(get_val16(), value - 1) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val16(std::rotr(get_val16(), value));
	}

	else
	{
		if (std::rotr(get_val8(), value - 1) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val8(std::rotr(get_val8(), value));
	}
}

void CPU::Instruction::Operand::ror(Data value)
{
	if (_width) { ror(value.data16); }
	else { ror(value.data8[0]); }
}

void CPU::Instruction::Operand::ror(Operand value)
{
	if (_width) { ror(value.get_val16()); }
	else { ror(value.get_val8()); }
}

void CPU::Instruction::Operand::rcl(uint8_t value)
{
	bool old_cf{ static_cast<bool>(_cpu.get_reg16(Registers::Flags) & flag_carry) };

	if (_width)
	{
		if (std::rotl(get_val16(), value - 1) & 0x8000)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val16(std::rotl(get_val16(), value));

		if (old_cf)
		{
			write_val16(get_val16() | 0x1);
		}
	}

	else
	{
		if (std::rotl(get_val8(), value - 1) & 0x80)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val8(std::rotl(get_val8(), value));

		if (old_cf)
		{
			write_val8(get_val8() | 0x1);
		}
	}
}

void CPU::Instruction::Operand::rcl(Data value)
{
	if (_width) { rcl(value.data16); }
	else { rcl(value.data8[0]); }
}

void CPU::Instruction::Operand::rcl(Operand value)
{
	if (_width) { rcl(value.get_val16()); }
	else { rcl(value.get_val8()); }
}

void CPU::Instruction::Operand::rcr(uint8_t value)
{
	bool old_cf{ static_cast<bool>(_cpu.get_reg16(Registers::Flags) & flag_carry) };

	if (_width)
	{
		if (std::rotr(get_val16(), value - 1) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val16(std::rotr(get_val16(), value));

		if (old_cf)
		{
			write_val16(get_val16() | 0x8000);
		}
	}

	else
	{
		if (std::rotr(get_val8(), value - 1) & 0x1)
		{
			_cpu.get_reg16(Registers::Flags) |= flag_carry;
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
		}

		write_val8(std::rotr(get_val8(), value));

		if (old_cf)
		{
			write_val8(get_val8() | 0x80);
		}
	}
}

void CPU::Instruction::Operand::rcr(Data value)
{
	if (_width) { rcr(value.data16); }
	else { rcr(value.data8[0]); }
}

void CPU::Instruction::Operand::rcr(Operand value)
{
	if (_width) { rcr(value.get_val16()); }
	else { rcr(value.get_val8()); }
}


void CPU::Instruction::Operand::imul()
{
	if (_width)
	{
		int32_t product{ static_cast<int16_t>(_cpu.get_reg16(Registers::AX)) * static_cast<int16_t>(get_val16()) };

		_cpu.get_reg16(Registers::DX) = static_cast<int16_t>(product >> 16);
		_cpu.get_reg16(Registers::AX) = static_cast<int16_t>(product & 0x0000ffff);

		if (_cpu.get_reg16(Registers::DX) == 0 || _cpu.get_reg16(Registers::DX) == 0xffff)
		{
			_cpu.get_reg16(Registers::Flags) &= ~(flag_carry | flag_overflow);
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) |= (flag_carry | flag_overflow);
		}
	}

	else
	{
		_cpu.get_reg16(Registers::AX) = static_cast<int8_t>(_cpu.get_reg8(Registers_8::AL)) * static_cast<int8_t>(get_val8());

		if (_cpu.get_reg8(Registers_8::AH) == 0 || _cpu.get_reg8(Registers_8::AH) == 0xff)
		{
			_cpu.get_reg16(Registers::Flags) &= ~(flag_carry | flag_overflow);
		}

		else
		{
			_cpu.get_reg16(Registers::Flags) |= (flag_carry | flag_overflow);
		}
	}
}

void CPU::Instruction::Operand::imul(uint16_t value1, uint16_t value2)
{
	
}

void CPU::Instruction::Operand::idiv()
{
	if (_width)
	{
		int32_t val_to_divide{ static_cast<int32_t>(_cpu.get_reg16(Registers::DX) << 16 | _cpu.get_reg16(Registers::AX)) }; // dx:ax

		_cpu.get_reg16(Registers::AX) = static_cast<int16_t>(val_to_divide / get_val16());
		_cpu.get_reg16(Registers::DX) = static_cast<int16_t>(val_to_divide % get_val16());
	}

	else
	{
		int32_t val_to_divide{ _cpu.get_reg16(Registers::AX) }; // dx:ax

		_cpu.get_reg8(Registers_8::AL) = static_cast<int8_t>(val_to_divide / get_val8());
		_cpu.get_reg8(Registers_8::AH) = static_cast<int8_t>(val_to_divide % get_val8());
	}
}