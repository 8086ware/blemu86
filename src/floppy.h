#pragma once
#include "device.h"
#include <fstream>
#include <array>
#include "chipset/pic.h"
#include "chipset/dma.h"
#include <utility>
#include <string_view>

class FDD
{
	struct _Fd_info_struct
	{
		int cylinders{};
		int heads{};
		int spt{}; // sectors per track
	};

	enum class Type : int // IBM PC xt only has 360k floppy drives but why not add all this
	{
		Size_160K,
		Size_180K,
		Size_320K,
		Size_360K,
		Size_720K,
		Size_1_2M,
		Size_1_44M,
		Count,
	};

	Type _type{};

	static constexpr int _sector_size{ 512 }; // come on, every floppy should have 512 byte sectors...

	std::fstream _file{};
public:
	static constexpr std::array<_Fd_info_struct, std::to_underlying<Type>(Type::Count)> fd_info{ { // This can be public read-only
		{.cylinders = 40, .heads = 1, .spt = 8}, // 160k
		{.cylinders = 40, .heads = 1, .spt = 9}, // 180k
		{.cylinders = 40, .heads = 2, .spt = 8}, // 320k
		{.cylinders = 40, .heads = 2, .spt = 9}, // 360k
		{.cylinders = 80, .heads = 2, .spt = 9}, // 720k
		{.cylinders = 80, .heads = 2, .spt = 15}, // 1.2M
		{.cylinders = 80, .heads = 2, .spt = 18}, // 1.44M
	} };

	FDD();
	FDD(std::string_view file_name);

	bool seek(int cylinder, int head, int sector);

	int cylinder() { return (_file.tellg() / _sector_size) / (fd_info[std::to_underlying<Type>(_type)].spt * fd_info[std::to_underlying<Type>(_type)].heads); }
	int head() { return ((_file.tellg() / _sector_size) % (fd_info[std::to_underlying<Type>(_type)].spt * fd_info[std::to_underlying<Type>(_type)].heads)) / fd_info[std::to_underlying<Type>(_type)].spt; }
	int sector() { return (_file.tellg() / _sector_size) % fd_info[std::to_underlying<Type>(_type)].spt + 1; }

	int get_type_id() { return std::to_underlying<Type>(_type); }

	bool write(uint8_t* data, size_t size);
	bool read(uint8_t* data, size_t size);

	bool insert(std::string_view file_name);
};

class FDC : public IO_Device
{
	// intel 8272 or nec u765 compatible

	PIC& _pic;
	DMA& _dma;
	std::array<FDD, 4>& _fdds;

	FDD* _selected_fdd{ nullptr };

	std::array<uint8_t, 4> _st{};
	std::array<uint8_t, 9> _data{};

	int _data_bytes{};
	int _data_current{};

	uint8_t _msr{};

	bool _result_phase{};
	// The FDD are in seek mode
public:
	enum class Port
	{
		Msr = 0x3F4,
		Data_IO = 0x3F5,// R/W
		Dor = 0x3F2,
	};

	static constexpr uint8_t msr_fdd_0_seek{ 0x1 };
	static constexpr uint8_t msr_fdd_1_seek{ 0x2 };
	static constexpr uint8_t msr_fdd_2_seek{ 0x4 };
	static constexpr uint8_t msr_fdd_3_seek{ 0x8 };
	static constexpr uint8_t msr_busy{ 0x10 };// Transitioning from 1 to 0 indicates execution phase ended
	static constexpr uint8_t msr_non_dma_mode{ 0x20 };
	static constexpr uint8_t msr_data_io{ 0x40 }; // Indications direction of data transfer. 1 = Data I/O -> CPU (read), 0 = CPU -> Data I/O (write)
	static constexpr uint8_t msr_rqm{ 0x80 }; // Indicates data i/o is ready to send/receive data. This and Data IO port should be used to perform the (command phase) hadnshasking functions.

	static constexpr uint8_t dor_drive{ 0x3 };
	static constexpr uint8_t dor_reset_off{ 0x4 };
	static constexpr uint8_t dor_dma_irq_on{ 0x8 };
	static constexpr uint8_t dor_motor_a_on{ 0x10 };
	static constexpr uint8_t dor_motor_b_on{ 0x20 };
	static constexpr uint8_t dor_motor_c_on{ 0x40 };
	static constexpr uint8_t dor_motor_d_on{ 0x80 };

	static constexpr uint8_t data_command_specify{ 0x3 };
	static constexpr uint8_t data_command_read{ 0x6 };
	static constexpr uint8_t data_command_recalibrate{ 0x7 };
	static constexpr uint8_t data_command_sense_interrupt{ 0x8 };
	static constexpr uint8_t data_command_read_id{ 0xA };
	static constexpr uint8_t data_command_seek{ 0xF };

	static constexpr uint8_t st0_int_code{ 0xC0 };
	static constexpr uint8_t st0_sk_end{ 0x20 };
	static constexpr uint8_t st0_equipment_check{ 0x10 };
	static constexpr uint8_t st0_not_ready{ 0x8 };
	static constexpr uint8_t st0_head_address{ 0x4 };
	static constexpr uint8_t st0_drive_select{ 0x3 };

	static constexpr uint8_t st1_en_cylinder{ 0x80 };
	static constexpr uint8_t st1_data_error{ 0x20 };
	static constexpr uint8_t st1_over_run{ 0x10 };
	static constexpr uint8_t st1_no_data{ 0x4 };
	static constexpr uint8_t st1_not_writeable{ 0x2 };
	static constexpr uint8_t st1_ma_mark{ 0x1 };

	static constexpr uint8_t st2_control_mark{ 0x40 };
	static constexpr uint8_t st2_data_error_dd{ 0x20 };
	static constexpr uint8_t st2_wrong_cylinder{ 0x10 };
	static constexpr uint8_t st2_sh_hit{ 0x8 };
	static constexpr uint8_t st2_sn_satisfied{ 0x4 };
	static constexpr uint8_t st2_bad_cylinder{ 0x2 };
	static constexpr uint8_t st2_md_mark_data_field{ 0x1 };

	bool write(int address, uint8_t data, bool io) override;
	std::optional<uint8_t> read(int address, bool io) override;

	FDC(PIC& pic, DMA& dma, std::array<FDD, 4>& fdds);
};
