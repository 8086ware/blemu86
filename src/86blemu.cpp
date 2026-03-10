#include "86blemu.h"
#include <SDL3/SDL.h>
#include <string>

bool Blemu86::loop()
{
	double cpu_ns_per_tick{ (1 / static_cast<double>(_cpu.get_hz())) * 1'000'000'000 };
	size_t time_now{ static_cast<size_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) };

	for (int i{}; i < (time_now - _cpu.get_last_tick()) / cpu_ns_per_tick; i++)
	{
		CPU::Instruction instruction{ _cpu };
		instruction.exec();

		for (size_t k{}; k < _cycle_devices.size(); k++)
		{
			double device_ns_per_tick{ (1 / static_cast<double>(_cycle_devices[k]->get_hz())) * 1'000'000'000 };

			if (_cpu.ticks_total % (size_t)(device_ns_per_tick / cpu_ns_per_tick) == 0) // Sync time for things slower than CPU
			{
				_cycle_devices[k]->cycle();
			}
		}
	}

	_cpu.set_last_tick(time_now);

	return true;
}

bool exit_callback(void* userdata, SDL_Event* event)
{
	if (event->type == SDL_EVENT_QUIT)
	{
		reinterpret_cast<Blemu86*>(userdata)->~Blemu86();
		exit(0);
	}

	return true;
}

Blemu86::Blemu86(std::string_view bios_rom_file_name) : _bios_rom(bios_rom_file_name)
{
	srand(time(NULL)); // needed for some things

	new_cycle_device(_keyboard);
	new_cycle_device(_cga);
	new_cycle_device(_pit);

	_bus.new_iodevice(_cga);
	_bus.new_iodevice(_cga.crtc);
	_bus.new_iodevice(_pit);
	_bus.new_iodevice(_pic);
	_bus.new_iodevice(_ppi);
	_bus.new_iodevice(_bios_rom);
	_bus.new_iodevice(_dma);
	_bus.new_iodevice(_fdc);

	if (!SDL_WasInit(SDL_INIT_VIDEO))
	{
		SDL_Init(SDL_INIT_VIDEO);
	}

	SDL_AddEventWatch(exit_callback, this);

	_cpu.set_last_tick(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

Blemu86::~Blemu86()
{
	SDL_DestroyRenderer(_win_render);
	SDL_DestroyWindow(_win);
	SDL_Quit();
}

void Blemu86::new_cycle_device(Device& device)
{
	_cycle_devices.push_back(&device);
}