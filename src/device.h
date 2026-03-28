#pragma once
#include <cstdint>
#include <optional>

// This interface is used for devices that provide input and output through ports/address space
class IO_Device
{
public:
	virtual bool write(int address, uint8_t data, bool io) = 0;
	virtual std::optional<uint8_t> read(int address, bool io) = 0;
};

class Clock_Device
{
public:
	virtual void cycle() = 0;
};