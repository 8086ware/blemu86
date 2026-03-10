#pragma once

#include "device.h"
#include <SDL3/SDL.h>
#include "chipset/ppi.h"
#include <vector>

class Keyboard : public Device
{
	std::vector<uint8_t> _scancodes{};

	PPI& _ppi;
	PIC& _pic;
public:
	void cycle() override;

	Keyboard(PPI& ppi, PIC& pic);
};