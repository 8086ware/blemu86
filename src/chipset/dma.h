#pragma once
#include "device.h"
#include "chipset/bus.h"

class DMA : public IO_Device
{
	uint8_t _status{};

	bool _flip_flop{};

	uint8_t _dreq{};
	uint8_t _dmask{};
	uint8_t _dack{};
public:
	Bus& _bus;

	struct
	{
		uint32_t current_address{}; // Is incremented each transfer
		uint16_t current_word{}; // Is decremented each transfer
		uint32_t base_address{}; // Original current_address
		uint16_t base_word{}; // Original current_word

		uint8_t mode{};
	} _channels[4];

	static constexpr uint8_t command_flag_mem2mem{ 0x1 };
	static constexpr uint8_t command_flag_channel_0_address_hold{ 0x2 };
	static constexpr uint8_t command_flag_controller_disable{ 0x4 };
	static constexpr uint8_t command_flag_priority_rotate{ 0x10 };

	static constexpr uint8_t mode_flag_select_channel{ 0x3 };
	static constexpr uint8_t mode_flag_transfer_mode{ 0xC };
	static constexpr uint8_t mode_flag_autoinitialization{ 0x10 };
	static constexpr uint8_t mode_flag_decrement_select{ 0x20 };
	static constexpr uint8_t mode_flag_select_mode{ 0xC0 };

	static constexpr uint8_t request_flag_select_channel{ 0x3 };
	static constexpr uint8_t request_flag_set_request{ 0x4 };

	static constexpr uint8_t status_flag_tc_channel_0{ 0x1 };
	static constexpr uint8_t status_flag_tc_channel_1{ 0x2 };
	static constexpr uint8_t status_flag_tc_channel_2{ 0x4 };
	static constexpr uint8_t status_flag_tc_channel_3{ 0x8 };

	static constexpr uint8_t status_flag_request_channel_0{ 0x10 };
	static constexpr uint8_t status_flag_request_channel_1{ 0x20 };
	static constexpr uint8_t status_flag_request_channel_2{ 0x40 };
	static constexpr uint8_t status_flag_request_channel_3{ 0x80 };

	static constexpr uint8_t single_mask_flag_select_channel{ 0x3 };
	static constexpr uint8_t single_mask_set_mask{ 0x4 };

	static constexpr uint8_t multi_mask_set_channel_0{ 0x1 };
	static constexpr uint8_t multi_mask_set_channel_1{ 0x2 };
	static constexpr uint8_t multi_mask_set_channel_2{ 0x4 };
	static constexpr uint8_t multi_mask_set_channel_3{ 0x8 };

	enum class Port : uint8_t
	{
		Channel_0_address = 0x0,
		Channel_0_word_count = 0x1,
		Channel_1_address = 0x2,
		Channel_1_word_count = 0x3,
		Channel_2_address = 0x4,
		Channel_2_word_count = 0x5,
		Channel_3_address = 0x6,
		Channel_3_word_count = 0x7,
		Status_command = 0x8, // R = Status, W = Command
		Request = 0x9,
		Mask = 0xA,
		Mode = 0xB,
		Clear_Flip_Flop = 0xC,
		Master_clear_temp = 0xD,
		Clear_mask = 0xE,
		Multiple_mask = 0xF,

		Channel_1_high_order_8bits = 0x83,
		Channel_2_high_order_8bits = 0x81,
		Channel_3_high_order_8bits = 0x82,
	};

	bool write(int address, uint8_t data, bool io) override;
	std::optional<uint8_t> read(int address, bool io) override;

	void operation(int channel, std::unique_ptr<uint8_t[]>& data);

	DMA(Bus& bus);
};
