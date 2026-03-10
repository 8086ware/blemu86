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

class Blemu86 // The emulator... Or physically the motherboard lol
{
public:
	static constexpr int ram_size{ 0x100000 };
	Blemu86(std::string_view bios_rom_file_name);
	~Blemu86();
	void new_cycle_device(Device& device);
	bool loop();
private:
	SDL_Window* _win{ SDL_CreateWindow("86Blemu", 320 * 3, 200 * 3, SDL_WINDOW_RESIZABLE) };
	SDL_Renderer* _win_render{ SDL_CreateRenderer(_win, NULL) };
	std::vector<Device*> _cycle_devices{}; // Devices to cycle through (e.g: PIT)
	std::shared_ptr<uint8_t[]> _ram{ std::make_unique<uint8_t[]>(ram_size) };
	Bus _bus{ _ram };
	CGA _cga{ _win_render };
	PIC _pic{};
	PPI _ppi{ _pic };
	PIT _pit{ _pic };
	CPU _cpu{ _pic, _bus };
	DMA _dma{ _bus };
	std::array<FDD, 4> fdds{ { {"osall.img"}, {""}, {""}, {""} } };
	FDC _fdc{ _pic, _dma, fdds };
	Bios_ROM _bios_rom;
	Keyboard _keyboard{ _ppi, _pic };
};