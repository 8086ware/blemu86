#pragma once

#include <memory>
#include "cpu/cpu.h"
#include "chipset/bus.h"
#include "cga.h"
#include "chipset/pic.h"
#include "chipset/pit.h"
#include "chipset/ppi.h"
#include "keyboard.h"
#include "bios_rom.h"
#include "floppy.h"
#include "device.h"
#include <cstdint>
#include <SDL3/SDL.h>

class Blemu86 // The emulator... Or physically the motherboard lol
{
public:
	static constexpr int ram_size{ 0x100000 };
	Blemu86(std::string_view bios_rom_file_name);
	void new_clock_device(Clock_Device* device, double hz);
	bool loop();
	~Blemu86();
private:
	float sync_hz{ 200 };
	std::vector<std::pair<Clock_Device*, double>> _clocked_devices{}; // Devices to cycle through (e.g: PIT) and the HZ
	std::unique_ptr<uint8_t[]> _ram{ std::make_unique<uint8_t[]>(ram_size) };
	Bus _bus{ {_ram.get(), ram_size} };
	CGA _cga{};
	PIC _pic{};
	PPI _ppi{ _pic };
	PIT _pit{ _pic };
	CPU _cpu{ _pic, _bus };
	DMA _dma{ _bus };
	std::array<FDD, 4> _fdds;
	FDC _fdc{ _pic, _dma, _fdds };
	Bios_ROM _bios_rom;
	Keyboard _keyboard{ _ppi, _pic };
};