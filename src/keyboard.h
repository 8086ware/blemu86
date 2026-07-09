#pragma once

#include "device.h"
#include <SFML/Graphics.hpp>
#include "chipset/ppi.h"
#include <vector>

class Keyboard : public Clock_Device
{
	std::vector<uint8_t> _scancodes{};

	PPI& _ppi;
	PIC& _pic;

	sf::Window& _win;
public:
	void cycle() override;

	Keyboard(sf::Window& win, PPI& ppi, PIC& pic);
};