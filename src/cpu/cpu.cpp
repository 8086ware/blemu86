#include "cpu.h"
#include <print>
#include "to_linear.h"
#include "chipset/bus.h"

CPU::CPU(PIC& pic, Bus& bus) : _pic{ pic }, _bus{ bus }
{
	std::println("[CPU] Init...at {} hz", get_hz());
	reset();
}

void CPU::reset()
{
	std::println("[CPU] Reset to 0xfff0:xf000");

	get_reg(Registers::IP) = 0xfff0;
	get_reg(Segment_Registers::CS) = 0xf000;
}

void CPU::push(uint16_t value)
{
	get_reg(Registers::SP) -= 2;
	_bus.write<uint16_t>(to_linear(get_reg(Segment_Registers::SS), get_reg(Registers::SP)), value, false);
}

uint16_t CPU::pop()
{
	uint16_t value{ _bus.read<uint16_t>(to_linear(get_reg(Segment_Registers::SS), get_reg(Registers::SP)), false) };
	get_reg(Registers::SP) += 2;
	return value;
}

uint16_t& CPU::get_reg(Registers reg)
{
	return _regs[std::to_underlying(reg)].word;
}

uint8_t& CPU::get_reg(Registers_8 reg)
{
	if (std::to_underlying(reg) <= 3)
	{
		return _regs[std::to_underlying(reg)].byte[0];
	}

	else
	{
		return _regs[std::to_underlying(reg) - 4].byte[1];
	}
}

uint16_t& CPU::get_reg(Segment_Registers reg)
{
	return _sregs[std::to_underlying(reg)].word;
}

void CPU::check_irq()
{
	bool interrupt_go{};

	uint16_t vector_offset{};

	if (get_flags().test(CPU::flag_interrupt))
	{
		if (_pic.get_irr() != 0)
		{
			for (int i{}; i < 8; i++)
			{
				if (_pic.get_isr() & (1 << i))
				{
					break;
				}

				if (_pic.get_irr() & (1 << i) && (_pic.get_imr() & (1 << i)) == 0)
				{
					_pic.get_isr() |= (1 << i);
					_pic.get_irr() &= ~(1 << i);

					vector_offset = _pic.get_vector_offset() * 4 + i * 4;
					interrupt_go = true;
					break;
				}
			}

			if (interrupt_go)
			{
				push(get_flags().to_ulong());
				push(get_reg(Segment_Registers::CS));
				push(get_reg(Registers::IP));

				get_reg(Segment_Registers::CS) = _bus.read<uint16_t>(to_linear(0, vector_offset + 2), false);
				get_reg(Registers::IP) = _bus.read<uint16_t>(to_linear(0, vector_offset), false);

				get_flags().set(CPU::flag_interrupt, false);
				_halted = 0;
			}
		}
	}
}
