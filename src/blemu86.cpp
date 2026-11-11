#include "blemu86.h"
#include <string>

bool Blemu86::loop()
{
	const auto sync_start = SDL_GetTicksNS();

	for(int i{}; i < _clocked_devices.size(); i++)
	{
		for(int k{}; k < _clocked_devices[i].second / sync_hz; k++)
		{
			_clocked_devices[i].first->cycle();
		}
	}

	for(int i{}; i < _cpu.get_hz() / sync_hz; i++)
	{
		_cpu.logic();
	}

	while(SDL_GetTicksNS() - sync_start <= (1/sync_hz) * 1'000'000'000);

	return true;
}


Blemu86::Blemu86(std::string_view bios_rom_file_name) : _bios_rom(bios_rom_file_name)
{
	SDL_Init(SDL_INIT_VIDEO);

	srand(time(NULL)); // needed for some things

	new_clock_device(&_keyboard, 15000);
	new_clock_device(&_cga, 60);
	new_clock_device(&_pit, 1193182);

	_bus.new_io_device(&_cga);
	_bus.new_io_device(&_cga.crtc);
	_bus.new_io_device(&_pit);
	_bus.new_io_device(&_pic);
	_bus.new_io_device(&_ppi);
	_bus.new_io_device(&_bios_rom);
	_bus.new_io_device(&_dma);
	_bus.new_io_device(&_fdc);
	
	_fdds[0].insert("osall.img");
	_cpu.set_last_tick(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

Blemu86::~Blemu86()
{
	SDL_Quit();
}

void Blemu86::new_clock_device(Clock_Device* device, double hz)
{
	_clocked_devices.push_back({device, hz});
}
