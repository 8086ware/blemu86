#pragma once
#include <cstdint>
#include <optional>

// This interafce is used for devices that provide input and output through ports/address space and cycling
class Device
{
	size_t _hz{};
	bool _io{};
public:
	virtual bool write(int address, uint8_t data, bool io) { return false;  }

	virtual std::optional<uint8_t> read(int address, bool io) 
	{
		return 0xff; 
	}

	virtual void cycle() {}

	size_t get_hz() { return _hz; }
	void set_hz(size_t hz) { _hz = hz; }

	Device(bool io, size_t hz) : _hz { hz }, _io{ io }
	{

	}
};