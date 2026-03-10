#include "bus.h"
#include <print>
#include "device.h"
#include <memory>
#include "86blemu.h"

Bus::Bus(std::shared_ptr<uint8_t[]>& ram) : _ram{ ram }
{
	std::println("[Bus] Init...");
}

uint8_t Bus::read8(int address, bool io)
{
	uint8_t read{ 0xff };

	if (address >= 0x100000 || address < 0)
	{
		std::println("[Bus] Invalid address {:x}", address);
		return 0xff;
	}

	bool device_read{};

	for (int i{}; i < this->_io_devices.size(); i++) // Go through all IO devices...
	{
		read = _io_devices[i]->read(address, io); // Pass address and io option and write

		if (!_io_devices[i]->get_last_access())
		{
			device_read = true;
			break;
		}
	}

	if (!device_read && !io)
	{
		read = _ram[address];
	}

	return read;
}

uint16_t Bus::read16(int address, bool io)
{
	return static_cast<uint16_t>(read8(address, io) & 0x00ff) | (static_cast<uint16_t>(read8(address + 1, io)) << 8);
}

void Bus::write8(int address, uint8_t data, bool io)
{
	if (address >= 0x100000 || address < 0)
	{
		std::println("[Bus] Invalid write {:x} to {:x}", data, address);
		return;
	}

	bool device_written{};

	for (int i{}; i < this->_io_devices.size(); i++) // Go through all IO devices...
	{
		_io_devices[i]->write(address, data, io); // Pass address and io option and write

		if (!_io_devices[i]->get_last_access())
		{
			device_written = true;
			break;
		}
	}

	if (!device_written && !io)
	{
		_ram[address] = data;
	}
}

void Bus::write16(int address, uint16_t data, bool io)
{
	write8(address, static_cast<uint8_t>(data & 0x00ff), io);
	write8(address + 1, static_cast<uint8_t>((data & 0xff00) >> 8), io);
}

void Bus::new_iodevice(Device& device)
{
	_io_devices.push_back(&device);
}