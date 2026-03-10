#include "pic.h"
#include <utility>
#include <print>

void PIC::raise(IRQ line)
{
	_irr |= (1 << std::to_underlying<IRQ>(line));
}

void PIC::write(int address, uint8_t data, bool io) 
{
	_last_access = false;

	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Command:
		{
			if (data & icw1_init)
			{
				_icw[0] = data;
				_icw_step = 2;
				_imr = 0;
			}

			else
			{
				if (data & ocw3_enable) // OCW3 selected
				{
					_ocw[2] = data;
				}

				else // OCW2 selected
				{
					_ocw[1] = data;

					uint8_t ir_acted_upon = _ocw[1] & ocw2_servicing;

					if (_ocw[1] & ocw2_eoi)
					{
						if (_ocw[1] & ocw2_sl)
						{
							_isr &= ~(1 << ir_acted_upon);
						}

						for (int i = 0; i < 8; i++)
						{
							if (_isr & (1 << i))
							{
								_isr &= ~(1 << i);
								break;
							}
						}
					}
				}
			}
			break;
		}

		case Port::Data:
		{
			if (_command == 0 && _icw_step == 0) // OCW1 
			{
				_imr = data;
			}

			if (_icw_step == 2)
			{
				_vector_offset = data & 0xF8; // in 8086 mode the 5 most significant data bits are the vector offset
				_icw_step = 0;

				if ((_icw[0] & icw1_single) == 0)
				{
					_icw_step = 3;
				}

				else if (_icw[0] & icw1_icw4)
				{
					_icw_step = 4;
				}

				else
				{
					_icw[3] = 0;
				}
			}

			else if (_icw_step == 3) // icw3 only read when 2 pics
			{
				_icw[2] = data;

				if (_icw[0] & icw1_icw4)
				{
					_icw_step = 4;
				}

				else
				{
					_icw[3] = 0;
					_icw_step = 0;
				}
			}

			else if (_icw_step == 4)
			{
				_icw[3] = data;
				_icw_step = 0;
			}
			break;
		}
		default:
		{
			_last_access = true;
			break;
		}
		}
	}

	else
	{
		_last_access = true;
	}
}

uint8_t PIC::read(int address, bool io)
{
	_last_access = false;

	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Command:
		{
			if ((_ocw[2] & ocw3_rr) == 0x2) // Read IR reg
			{
				return _irr;
			}

			else if ((_ocw[2] & ocw3_rr) == 0x3) // Read IS reg
			{
				return _isr;
			}

			_ocw[2] &= ~ocw3_rr;
			break;
		}
		case Port::Data:
		{
			if (_command == 0) // OCW1
			{
				return _imr;
			}
			break;
		}
		}
	}

	_last_access = true;
	return 0xff;
}

PIC::PIC() : Device(true, 0)
{
	std::println("[PIC] Init...");
}