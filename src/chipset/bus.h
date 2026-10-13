#pragma once 
#include <memory>
#include <vector>
#include "device.h"
#include <array>
#include <span>
#include "cpu/cpu_concepts.h"
#include <print>

class Bus
{
	std::vector<IO_Device*> _io_devices{};
	std::span<uint8_t> _ram{};
public:
	Bus(std::span<uint8_t> ram);

	template<CPU_Width T> T read(int address, bool io)
	{
		if constexpr(std::is_same_v<T, uint8_t>)
		{
			std::optional<uint8_t> read{};

			if (address >= 0x100000 || address < 0)
			{
				std::println("[Bus] Invalid address {:x}", address);
				return 0xff;
			}

			for (int i{}; i < this->_io_devices.size(); i++) // Go through all IO devices...
			{
				read = _io_devices[i]->read(address, io); // Pass address and io option and write

				if (read)
				{
					break;
				}
			}

			if (!read)
			{
				if (io)
				{
					read = 0xff;
				}

				else
				{
					read = _ram[address];
				}
			}

			return read.value();
		}

		else if constexpr(std::is_same_v<T, uint16_t>)
		{
			return static_cast<uint16_t>(read<uint8_t>(address, io) & 0x00ff) | (static_cast<uint16_t>(read<uint8_t>(address + 1, io)) << 8);
		}
	}

	template<CPU_Width T> void write(int address, T data, bool io)
	{
		if constexpr(std::is_same_v<T, uint8_t>)
		{
			if (address >= 0x100000 || address < 0)
			{
				std::println("[Bus] Invalid write {:x} to {:x}", data, address);
				return;
			}

			bool device_written{};

			for (int i{}; i < this->_io_devices.size(); i++) // Go through all IO devices...
			{
				device_written = _io_devices[i]->write(address, data, io);

				if (device_written) // Pass address and io option and write
				{
					break;
				}
			}

			if (!device_written && !io)
			{
				_ram[address] = data;
			}
		}

		else if constexpr(std::is_same_v<T, uint16_t>)
		{
			write<uint8_t>(address, static_cast<uint8_t>(data & 0x00ff), io);
			write<uint8_t>(address + 1, static_cast<uint8_t>((data & 0xff00) >> 8), io);
		}
	}

	void new_io_device(IO_Device* device);
};
