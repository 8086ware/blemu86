#pragma once
#include <cstdint>
#include "device.h"
#include "pic.h"

// 8253 Programmable Interval Timer

class PIT : public Device
{
	PIC& _pic; // PIC the PIT is connected to

	enum class Port : uint16_t
	{
		Channel_0 = 0x40,
		Channel_1 = 0x41,
		Channel_2 = 0x42,

		Command_Reg = 0x43,
	};

	enum class Operating_Mode
	{
		Interrupt_On_Terminal_Count = 0,
		Hardware_Retriggerable_Oneshot = 1,
		Rate_Generator = 2,
		Square_Wave_Generator = 3,
		Software_Triggered_Strobe = 4,
		Hardware_Triggered_Strobe = 5,
	};

	enum class Access_Mode
	{
		Latch_Count = 0,
		Lobyte_Only = 1,
		Hibyte_Only = 2,
		Lobyte_Hibyte = 3,
	};

	struct 
	{
		bool latch_on{};
		uint16_t latch{};

		bool received_reload_low{};
		bool sent_current_low{};

		Operating_Mode operating_mode{};
		Access_Mode access_mode{};

		uint16_t current_count{};
		uint16_t reload_value{};

		bool output{true};

		bool flip_flop{};
	} _timers[3]{};
public:
	// IO device base class stuff
	void write(int address, uint8_t data, bool io) override;
	uint8_t read(int address, bool io) override;

	void cycle() override;

	PIT(PIC& pic);
};