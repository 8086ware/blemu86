#pragma once
#include "device.h"
#include "pic.h"

class PPI : public Device
{
	uint8_t _ppi_dip_switch{ 0x2C };

	// PPI modes:
	// 0 - Simple input or output mode
	// In this mode, A and B are 2 simple io ports but C is split into 2 4 bit ports.
	// Outpu ts are latched, inputs are not latched. Ports can't interrupt or handshake
	// can only operate in mode 0 for now

	// 1 - Input or output with handshake
	// handshake signals between peripherals. Ports are simple 2 8 bit ports.
	// port C is used for hand shake signals. Specifically 2 (A and B) 3 bit lines. Remaining 2 bit is for i/o operations (simple)
	// Input and Output are latched

	// 2 - Bidirectional data transfer mode
	// Usually used in data transfer between computers.
	// Port A is configured as bidirectional, and Port B in mode 1 or 0.
	// Port A uses five signals from Port C as handshake signals for data transfer
	// Remaining three signals from Port C is simple IO or handshake for port B.

	uint8_t _regs[3]{};
	uint8_t _control_byte{};

	PIC& _pic;
public:
	uint8_t& port_a{ _regs[0] };

	enum class Port
	{
		A = 0x60,
		B = 0x61,
		C = 0x62,
		Ctrl = 0x63,
	};

	static constexpr uint8_t b_timer2_gate{ 0x1 };
	static constexpr uint8_t b_timer2_data{ 0x2 };
	static constexpr uint8_t b_jumper_e5{ 0x4 };
	static constexpr uint8_t b_read_switches{ 0x8 };// 1 = dip switch 5-8 is in PC0-3, 0 = dip switch 1-4 in PC0-3
	static constexpr uint8_t b_keyboard_clock{ 0x40 };
	static constexpr uint8_t b_keyboard_disable{ 0x80 };

	static constexpr uint8_t c_dip_switch_1{ 0xf };

	static constexpr uint8_t ctrl_lower_c_input{ 0x1 };
	static constexpr uint8_t ctrl_b_input{ 0x2 };

	static constexpr uint8_t ctrl_higher_c_input{ 0x8 };
	static constexpr uint8_t ctrl_a_input{ 0x16 };

	void write(int address, uint8_t data, bool io) override;
	uint8_t read(int address, bool io) override;

	PPI(PIC& pic);
};