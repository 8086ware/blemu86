#include "dma.h"
#include <utility>
#include <print>

DMA::DMA(Bus& bus) : Device(true, 0), _bus{ bus }
{
	std::println("[DMA] Init...");
}

bool DMA::write(int address, uint8_t data, bool io)
{
	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Channel_1_high_order_8bits:
		{
			_channels[1].base_address |= ((uint32_t)data << 16);
			break;
		}
		case Port::Channel_2_high_order_8bits:
		{
			_channels[2].base_address |= ((uint32_t)data << 16);
			break;
		}
		case Port::Channel_3_high_order_8bits:
		{
			_channels[3].base_address |= ((uint32_t)data << 16);
			break;
		}
		case Port::Channel_0_address:
		case Port::Channel_1_address:
		case Port::Channel_2_address:
		case Port::Channel_3_address:
		{
			int reg{ (address - std::to_underlying<Port>(Port::Channel_0_address)) / 2 };

			if (!_flip_flop) // low byte
			{
				_channels[reg].base_address &= ~0x00ff;
				_channels[reg].current_address &= ~0x00ff;
				_channels[reg].base_address |= static_cast<uint16_t>(data);
				_channels[reg].current_address |= static_cast<uint16_t>(data);
				_flip_flop = 1;
			}

			else // high byte
			{
				_channels[reg].base_address &= ~0xff00;
				_channels[reg].current_address &= ~0xff00;
				_channels[reg].base_address |= (static_cast<uint16_t>(data) << 8);
				_channels[reg].current_address |= (static_cast<uint16_t>(data) << 8);
				_flip_flop = 0;
			}

			break;
		}
		case Port::Channel_0_word_count:
		case Port::Channel_1_word_count:
		case Port::Channel_2_word_count:
		case Port::Channel_3_word_count:
		{
			int reg{ ((address - std::to_underlying<Port>(Port::Channel_0_word_count)) / 2) };

			if (!_flip_flop) // low byte
			{
				_channels[reg].base_word &= ~0x00ff;
				_channels[reg].current_word &= ~0x00ff;
				_channels[reg].base_word |= static_cast<uint16_t>(data);
				_channels[reg].current_word |= static_cast<uint16_t>(data);
				_flip_flop = 1;
			}

			else // high byte
			{
				_channels[reg].base_word &= ~0xff00;
				_channels[reg].current_word &= ~0xff00;
				_channels[reg].base_word |= (static_cast<uint16_t>(data) << 8);
				_channels[reg].current_word |= (static_cast<uint16_t>(data) << 8);
				_flip_flop = 0;
			}

			break;
		}
		case Port::Request:
		{
			int channel{ data & request_flag_select_channel };

			if (data & request_flag_set_request)
			{
				_dreq |= (1 << (channel));
			}

			else
			{
				_dreq &= ~(1 << (channel));
			}

			break;
		}
		case Port::Mask:
		{
			int channel{ data & single_mask_flag_select_channel };

			if (data & single_mask_set_mask)
			{
				_dmask |= (1 << (channel));
			}

			else
			{
				_dmask &= ~(1 << (channel));
			}

			break;
		}
		case Port::Mode:
		{
			int channel{ data & single_mask_flag_select_channel };
			_channels[channel].mode = data;
			break;
		}
		case Port::Master_clear_temp:
		{
			_flip_flop = 0;
			_status = 0;
			_dmask = 0xF;
			break;
		}
		case Port::Clear_Flip_Flop:
		{
			_flip_flop = 0;
			break;
		}
		case Port::Clear_mask:
		{
			_dmask = 0;
			break;
		}
		case Port::Multiple_mask:
		{
			_dmask = data & 0xf;
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

std::optional<uint8_t> DMA::read(int address, bool io)
{
	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Status_command:
		{
			_status |= (_dreq << 4);
			return _status;
			break;
		}
		case Port::Channel_0_address:
		case Port::Channel_1_address:
		case Port::Channel_2_address:
		case Port::Channel_3_address:
		{
			int reg{ (address - std::to_underlying<Port>(Port::Channel_0_address)) / 2 };

			if (!_flip_flop) // low byte
			{
				_flip_flop = true;
				return static_cast<uint8_t>(_channels[reg].current_address & 0x00ff);
			}

			else // high byte
			{
				_flip_flop = false;
				return static_cast<uint8_t>(_channels[reg].current_address >> 8);
			}
			break;
		}
		case Port::Channel_0_word_count:
		case Port::Channel_1_word_count:
		case Port::Channel_2_word_count:
		case Port::Channel_3_word_count:
		{
			int reg{ (address - std::to_underlying<Port>(Port::Channel_0_word_count)) / 2 };

			if (!_flip_flop) // low byte
			{
				_flip_flop = true;
				return static_cast<uint8_t>(_channels[reg].current_word & 0x00ff);
			}

			else // high byte
			{
				_flip_flop = false;
				return static_cast<uint8_t>(_channels[reg].current_word >> 8);
			}

			break;
		}
		}
	}

	return std::nullopt;
}

void DMA::operation(int channel, std::unique_ptr<uint8_t[]>& data) // A dreq...
{
	for (int i{}; i < (_channels[channel].base_word + 1); i++)
	{
		_bus.write8(_channels[channel].current_address, data[i], false);

		if (_channels[channel].mode & DMA::mode_flag_decrement_select)
		{
			_channels[channel].current_address--;
		}

		else
		{
			_channels[channel].current_address++;
		}
	} // Terminal count/EOP

	_channels[channel].current_word = 0xffff;

	if (_channels[channel].mode & DMA::mode_flag_autoinitialization)
	{
		_channels[channel].current_address = _channels[channel].base_address;
		_channels[channel].current_word = _channels[channel].base_word;
	}
}