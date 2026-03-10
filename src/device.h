#pragma once
#include <cstdint>

// This interafce is used for devices that provide input and output through ports/address space and cycling
class Device
{
	size_t _hz{};
	bool _io{};
protected:
	bool _last_access{};
public:
	virtual void write(int address, uint8_t data, bool io) 
	{ 
		_last_access = true; 
	}

	virtual uint8_t read(int address, bool io) 
	{ 
		_last_access = true;
		return 0xff; 
	}

	virtual void cycle() {}

	bool get_last_access() { return _last_access; }

	size_t get_hz() { return _hz; }
	void set_hz(size_t hz) { _hz = hz; }

	Device(bool io, size_t hz) : _hz { hz }, _io{ io }
	{

	}
};