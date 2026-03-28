#pragma once

#include <cstdint>
#include "device.h"

class PIC : public IO_Device
{
	uint8_t _command{};
	// -1 if no interrupt, > -1 contains interrupt number
	uint8_t _isr{}; // in service register, x bit is currently 
	uint8_t _irr{}; // interrupt request register, x bit is raised irq
	uint8_t _imr{}; // interrupt mask register, will prevent any irq at x bit set
	uint16_t _vector_offset{}; // offset from 0000:0000, or the ivt (multiples of 4, 2 ip 2 cs)

	uint8_t _icw_step{};

	uint8_t _icw[4]{};
	uint8_t _ocw[3]{};
public:
	enum class Port : uint16_t
	{
		Command = 0x20,
		Data = 0x21,
	};

	enum class IRQ
	{
		Pit = 0,
		Keyboard = 1,
		Slave_Controller = 2, // Unused on IBM PC XT, only on AT+
		Com2_Or_Com4 = 3,
		Com1_Or_Com3 = 4,
		LPT2 = 5,
		Floppy_Ctrl = 6,
		LPT1 = 7,
	};

	static constexpr uint8_t icw1_icw4{ 0x01 };
	static constexpr uint8_t icw1_single{ 0x02 };
	static constexpr uint8_t icw1_init{ 0x10 }; // imr cleared, irq 7 assigned priority 7, slave mode address is set to 7, status read set to irr

	// ICW2 is the vector offsets
	// ICW3 for setting the slave pic irq

	static constexpr uint8_t icw4_8086{ 0x01 };
	static constexpr uint8_t icw4_auto{ 0x02 };

	static constexpr uint8_t ocw2_servicing{ 0x7 };
	static constexpr uint8_t ocw2_eoi{ 0x20 };
	static constexpr uint8_t ocw2_sl{ 0x40 };
	static constexpr uint8_t ocw2_r{ 0x80 };

	static constexpr uint8_t ocw3_rr{ 0x3 };
	static constexpr uint8_t ocw3_poll{ 0x4 };
	static constexpr uint8_t ocw3_enable{ 0x8 };
	static constexpr uint8_t ocw3_smm{ 0x60 };

	bool write(int address, uint8_t data, bool io) override;
	std::optional<uint8_t> read(int address, bool io) override;

	void raise(IRQ line);

	uint8_t& get_irr() { return _irr; }
	uint8_t& get_isr() { return _isr; }
	uint8_t get_imr() { return _imr; }

	uint16_t get_vector_offset() { return _vector_offset; }

	PIC();
};