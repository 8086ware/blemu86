#include "bus.h"
#include <print>
#include "device.h"
#include <memory>

Bus::Bus(std::span<uint8_t> ram) : _ram{ ram }
{
	std::println("[Bus] Init...");
}

void Bus::new_io_device(IO_Device* device)
{
	_io_devices.push_back(device);
}
