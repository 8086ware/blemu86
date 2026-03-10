#include "cpu.h"
#include "to_linear.h"

void CPU::Instruction::calc_modrm_byte(int modrm_address, bool sreg)
{
	_modrm = true;

	uint8_t modrm{ _cpu._bus.read8(modrm_address, false) };

	// modrm == 0b10'000'000

	uint8_t mod_val{ static_cast<uint8_t>((modrm & 0b11000000) >> 6) };
	uint8_t reg_val{ static_cast<uint8_t>((modrm & 0b00111000) >> 3) };
	uint8_t rm_val{ static_cast<uint8_t>(modrm & 0b00000111) };

	int16_t displacement16{};
	int8_t displacement8{};

	if (mod_val == 0b01)
	{
		displacement8 = _cpu._bus.read8(modrm_address + 1, false);
		_length += 2;
	}

	else if (mod_val == 0b10 || (mod_val == 0b00 && rm_val == 6))
	{
		displacement16 = _cpu._bus.read16(modrm_address + 1, false);
		_length += 3;
	}

	else
	{
		_length++;
	}

	uint16_t temp_regmem{};

	// Using mem, not reg in regmem
	if (mod_val == 0b01 || mod_val == 0b10 || mod_val == 0b00) // No displacement for mod 0 (except rm_val == 6)
	{
		switch (rm_val)
		{
		case 0:
		{
			temp_regmem += _cpu.get_reg16(Registers::BX) + _cpu.get_reg16(Registers::SI);
			break;
		}
		case 1:
		{
			temp_regmem += _cpu.get_reg16(Registers::BX) + _cpu.get_reg16(Registers::DI);
			break;
		}
		case 2:
		{
			if (_default_seg)
			{
				_data_seg = _cpu.get_sreg(Segment_Registers::SS);
			}

			temp_regmem += _cpu.get_reg16(Registers::BP) + _cpu.get_reg16(Registers::SI);
			break;
		}
		case 3:
		{
			if (_default_seg)
			{
				_data_seg = _cpu.get_sreg(Segment_Registers::SS);
			}

			temp_regmem += _cpu.get_reg16(Registers::BP) + _cpu.get_reg16(Registers::DI);
			break;
		}
		case 4:
		{
			temp_regmem += _cpu.get_reg16(Registers::SI);
			break;
		}
		case 5:
		{
			temp_regmem += _cpu.get_reg16(Registers::DI);
			break;
		}
		case 6:
		{
			if (mod_val == 0)
			{
				temp_regmem += displacement16;
			}

			else
			{
				if (_default_seg)
				{
					_data_seg = _cpu.get_sreg(Segment_Registers::SS);
				}

				temp_regmem += _cpu.get_reg16(Registers::BP);
			}

			break;
		}
		case 7:
		{
			temp_regmem += _cpu.get_reg16(Registers::BX);
			break;
		}
		}

		// Add displacement to regmem first before adding (if displacement provided)

		if (mod_val == 0b01)
		{
			temp_regmem += displacement8;
		}

		else if (mod_val == 0b10)
		{
			temp_regmem += displacement16;
		}

		if (_width)
		{
			_regmem->get_addr16() = reinterpret_cast<uint16_t*>(to_linear(_data_seg, temp_regmem));
		}

		else
		{
			_regmem->get_addr8() = reinterpret_cast<uint8_t*>(to_linear(_data_seg, temp_regmem));
		}
	}

	// rm is same as reg, using rmreg

	else if (mod_val == 0b11)
	{
		if (_width)
		{
			_regmem->get_addr16() = &_cpu.get_reg16(static_cast<Registers>(rm_val));
		}

		else
		{
			_regmem->get_addr8() = &_cpu.get_reg8(static_cast<Registers_8>(rm_val));
		}
	}

	// Determine reg 

	if (sreg)
	{
		_reg->get_addr16() = &_cpu.get_sreg(static_cast<Segment_Registers>(reg_val));
	}

	else if (_width)
	{
		_reg->get_addr16() = &_cpu.get_reg16(static_cast<Registers>(reg_val));
	}

	else
	{
		_reg->get_addr8() = &_cpu.get_reg8(static_cast<Registers_8>(reg_val));
	}
}