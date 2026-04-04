#pragma once
#include "device.h"
#include <string>
#include <fstream>
#include <print>
#include <filesystem>

class Bios_ROM : public IO_Device
{
private:
	std::ifstream _bios_file{};
	size_t _file_size{};
	std::unique_ptr<uint8_t[]> _bios_rom{ nullptr };
public:
	Bios_ROM(std::string_view file_name)
	{
		_bios_file.open(std::filesystem::path(file_name), std::ios_base::binary | std::ios_base::in);

		if (_bios_file.is_open())
		{
			_file_size = std::filesystem::file_size(file_name);

			_bios_rom = std::make_unique<uint8_t[]>(_file_size);

			_bios_file.seekg(0, std::ios_base::beg);

			_bios_file.read(reinterpret_cast<char*>(_bios_rom.get()), _file_size);

			std::println("[BIOS Rom] Successfully Loaded to 0x{:x}-0x{:x}", 0xFFFFF - _file_size, 0xFFFFF);
		}

		else
		{
			std::println("[BIOS Rom] Couldn't open file {}.", file_name);
		}

		_bios_file.close();
	}

	std::optional<uint8_t> read(int address, bool io) override // Read only device
	{
		if (!io)
		{
			if (address >= 0x100000 - _file_size && address < 0x100000)
			{
				return _bios_rom[address - (0x100000 - _file_size)];
			}
		}

		return std::nullopt;
	}

	bool write(int address, uint8_t data, bool io) override // Read only device
	{
		return false;
	}
};