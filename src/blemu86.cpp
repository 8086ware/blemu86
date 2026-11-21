#include "blemu86.h"
#include <string>
#include <thread>
#include <sstream>
#include <iostream>

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

	return running;
}

void Blemu86::debug_prompt(Blemu86& emu)
{
	while(emu.running)
	{
		std::string args{};
		
		std::print("Blemu> ");
		std::getline(std::cin, args);
		std::stringstream args_stream{args};
	
		std::string current_arg{};
		
		args_stream >> current_arg;

		if(current_arg == "exit")
		{
			emu.running = false;
		}

		else if(current_arg == "view")
		{
			int address{};

			args_stream >> std::hex >> address;

			int rows{16};
			int columns{32};

			for(int y{}; y < rows; y++)
			{
				std::print("{:5x} | ", address + (y * columns));

				for(int x{}; x < columns ; x++)
				{
					std::print("{:2x} ", emu._bus.read<uint8_t>(y * columns + address + x, false));
				}
				
				std::print("\t");

				for(int x{}; x < columns ; x++)
				{
					std::print("{}", std::isprint(static_cast<unsigned char>(emu._bus.read<uint8_t>(y * columns + address + x, false))) ? static_cast<char>(emu._bus.read<uint8_t>(y * columns + address + x, false)) : '.');
				}

				std::print("\n");
			}
		}

		else if(current_arg == "regs")
		{
			std::println("AX={}, CX={}, DX={}, BX={}, SP={}, BP={}, SI={}, DI={}", emu._cpu.get_reg(CPU::Registers::AX), emu._cpu.get_reg(CPU::Registers::CX), emu._cpu.get_reg(CPU::Registers::DX), emu._cpu.get_reg(CPU::Registers::BX), emu._cpu.get_reg(CPU::Registers::SP), emu._cpu.get_reg(CPU::Registers::BP), emu._cpu.get_reg(CPU::Registers::SI), emu._cpu.get_reg(CPU::Registers::DI));
		}

		else if(current_arg == "sregs")
		{
			std::println("ES={:x}, CS={:x}, SS={:x}, DS={:x}", emu._cpu.get_reg(CPU::Segment_Registers::ES), emu._cpu.get_reg(CPU::Segment_Registers::CS), emu._cpu.get_reg(CPU::Segment_Registers::SS), emu._cpu.get_reg(CPU::Segment_Registers::DS));
		}

		else if(current_arg == "stack")
		{
			for(int i{emu._cpu.get_reg(CPU::Registers::SP)}; i <= 0xffff; i+=2)
			{
				std::println("{} {:x}", i, emu._bus.read<uint16_t>(to_linear(emu._cpu.get_reg(CPU::Segment_Registers::SS), i), false));
			}
		}

		else if(current_arg == "reset")
		{
			emu._cpu.reset();
		}

		else if(current_arg == "fd")
		{
			args_stream >> current_arg;
			
			if(current_arg == "insert")
			{
				int fd{};
				args_stream >> fd;

				if(fd >= 0 && fd <= 3)
				{
					std::string file{};
					args_stream >> file;

					if(!emu._fdds[fd].insert(file))
					{
						std::println("Failed to insert {}", file);
					}
				}

				else
				{
					std::println("Floppy drive number must be 0-3");
				}
			}
		}
	}
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
	debug_prompt_thread.join();
	SDL_DestroyRenderer(_win_render);
	SDL_DestroyWindow(_win);
	SDL_Quit();
}

void Blemu86::new_clock_device(Clock_Device* device, double hz)
{
	_clocked_devices.push_back({device, hz});
}
