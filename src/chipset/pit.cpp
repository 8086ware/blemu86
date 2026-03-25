#include "pit.h"
#include <utility>
#include <print>
#include <chrono>

PIT::PIT(PIC& pic) : _pic{ pic }, Device(true, 1193182)
{
	std::println("[PIT] Init...");
}

std::optional<uint8_t> PIT::read(int address, bool io)
{
	int channel{ address - std::to_underlying<Port>(Port::Channel_0) };

	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Channel_0:
		case Port::Channel_1:
		case Port::Channel_2:
		{
			if (!_timers[channel].sent_current_low) // low
			{
				_timers[channel].sent_current_low = 1;

				if (_timers[channel].access_mode != Access_Mode::Hibyte_Only) // Lobyte or both?
				{
					if (_timers[channel].latch_on)
					{
						_timers[channel].latch_on = 0;
						return _timers[channel].latch & 0x00ff;
					}

					else
					{
						return _timers[channel].current_count & 0x00ff;
					}
				}

				else
				{
					return 0;
				}
			}

			else // high
			{
				_timers[channel].sent_current_low = 0;

				if (_timers[channel].access_mode != Access_Mode::Lobyte_Only)  // Hibyte or both?
				{
					if (_timers[channel].latch_on)
					{
						_timers[channel].latch_on = 0;
						return _timers[channel].latch >> 8;
					}

					else
					{
						return _timers[channel].current_count >> 8;
					}
				}

				else
				{
					return 0;
				}
			}
		}
		}
	}

	return std::nullopt;
}

bool PIT::write(int address, uint8_t data, bool io)
{
	int channel{ address - std::to_underlying<Port>(Port::Channel_0) };

	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Channel_0:
		case Port::Channel_1:
		case Port::Channel_2:
		{
			if (_timers[channel].received_reload_low) // low
			{
				_timers[channel].received_reload_low = true;

				if (_timers[channel].access_mode != Access_Mode::Hibyte_Only)
				{
					_timers[channel].reload_value &= ~0x00ff;
					_timers[channel].reload_value |= data;
				}

				else if (_timers[channel].access_mode == Access_Mode::Lobyte_Only)
				{
					if (_timers[channel].operating_mode == Operating_Mode::Interrupt_On_Terminal_Count)
					{
						_timers[channel].output = 1;
						_timers[channel].current_count = _timers[channel].reload_value;
					}
				}
			}

			else // high
			{
				_timers[channel].received_reload_low = 0;

				if (_timers[channel].access_mode != Access_Mode::Lobyte_Only)
				{
					_timers[channel].reload_value &= ~0xff00;
					_timers[channel].reload_value |= (data << 8);

					if (_timers[channel].operating_mode == Operating_Mode::Interrupt_On_Terminal_Count)
					{
						_timers[channel].output = 1;
						_timers[channel].current_count = _timers[channel].reload_value;
					}
				}
			}
			break;
		}
		case Port::Command_Reg:
		{
			int channel{ (data & 0xC0) >> 6 };

			if (((data & 0x30) >> 4) == 0 && _timers[channel].access_mode == Access_Mode::Lobyte_Hibyte)
			{
				_timers[channel].latch_on = 1;
				_timers[channel].latch = _timers[channel].current_count;
				_timers[channel].sent_current_low = 0;
			}

			else
			{
				_timers[channel].access_mode = static_cast<Access_Mode>((data & 0x30) >> 4);
				_timers[channel].operating_mode = static_cast<Operating_Mode>((data & 0xE) >> 1);

				if (_timers[channel].operating_mode == Operating_Mode::Interrupt_On_Terminal_Count)
				{
					_timers[channel].output = 0;
				}
			}
			break;
		}
		default:
		{
			return false;
			break;
		}
		}
	}

	else
	{
		return false;
	}

	return true;
}

void PIT::cycle()
{
	for (int channel{}; channel < 3; channel++)
	{
		switch (_timers[channel].operating_mode)
		{
		case Operating_Mode::Interrupt_On_Terminal_Count:
		{
			if (_timers[channel].current_count != 0 && _timers[channel].output)
			{
				_timers[channel].current_count--;
				_timers[channel].output = 1;
			}

			else
			{
				_timers[channel].output = 0;
			}

			break;
		}
		case Operating_Mode::Rate_Generator:
		{
			if (_timers[channel].current_count != 1)
			{
				_timers[channel].current_count--;
				_timers[channel].output = 1;
			}

			else
			{
				_timers[channel].output = 0;
			}

			break;
		}
		case Operating_Mode::Square_Wave_Generator:
		{
			if (_timers[channel].current_count % 2 == 1)
			{
				_timers[channel].current_count -= 1;
			}

			else
			{
				_timers[channel].current_count -= 2;
			}


			if (_timers[channel].current_count != 0)
			{
				_timers[channel].output = 1;
			}

			else
			{
				if (_timers[channel].flip_flop == 1)
				{
					_timers[channel].flip_flop = 0;
				}

				else
				{
					_timers[channel].output = 0;
					_timers[channel].flip_flop = 1;
				}
			}

			break;
		}
		}

		if (!_timers[channel].output)
		{
			switch (_timers[channel].operating_mode)
			{
			case Operating_Mode::Square_Wave_Generator:
			case Operating_Mode::Rate_Generator:
			case Operating_Mode::Interrupt_On_Terminal_Count:
			{
				_timers[channel].current_count = _timers[channel].reload_value;
				break;
			}
			}

			if (channel == 0)
			{
				_pic.raise(PIC::IRQ::Pit);
			}
		}
	}


}