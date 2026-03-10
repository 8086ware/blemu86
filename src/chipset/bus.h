#pragma once

#include <memory>
#include <vector>
#include "device.h"
#include <array>

class Bus
{
	std::vector<Device*> _io_devices{};
	std::shared_ptr<uint8_t[]> _ram{};
public:
	Bus(std::shared_ptr<uint8_t[]>& ram);

	uint8_t read8(int address, bool io);
	uint16_t read16(int address, bool io);
	void write8(int address, uint8_t data, bool io);
	void write16(int address, uint16_t data, bool io);

	void new_iodevice(Device& device);
};