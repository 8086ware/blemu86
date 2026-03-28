#include "ppi.h"
#include <print>

PPI::PPI(PIC& pic) : _pic{ pic }
{
	std::println("[PPI] Init...");
}

// value is something to be written, using read 
std::optional<uint8_t> PPI::read(int address, bool io)
{
	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::A: // input on IBM XT
		{
			uint8_t ret{ _regs[0] };

			if ((_regs[1] & b_keyboard_disable))
			{
				ret = 0;
			}

			_regs[0] = 0;

			return ret;
			break;
		}
		case Port::B:
		{
			return _regs[1];
			break;
		}
		case Port::C:
		{
			return _regs[2];
			break;
		}
		}
	}

	return std::nullopt;
}

bool PPI::write(int address, uint8_t data, bool io)
{
	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::B: // input on IBM XT
		{
			_regs[1] = data;

			_regs[2] &= ~c_dip_switch_1;

			if (_regs[1] & b_read_switches)
			{
				_regs[2] |= (_ppi_dip_switch >> 4);
			}

			else
			{
				_regs[2] |= (_ppi_dip_switch & 0x0f);
			}

			if ((_regs[1] & b_keyboard_clock) == 0) // clock line low, respond with test pass
			{
				_regs[0] = 0xAA;
				_pic.raise(PIC::IRQ::Keyboard);
			}

			break;
		}
		case Port::Ctrl:
		{
			_control_byte = data;
			break;
		}
		default:
		{
			return false;
		}
		}
	}

	else
	{
		return false;
	}

	return true;
}