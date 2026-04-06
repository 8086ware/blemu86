#include "floppy.h"
#include <print>
#include <filesystem>
#include <string_view>

FDD::FDD()
{
	std::println("[FDD] Init...");
}

FDD::FDD(std::string_view file_name)
{
	std::println("[FDD] Init...");
	insert(file_name);
}

bool FDD::insert(std::string_view file_name)
{
	if (file_name.empty())
	{
		return false;
	}

	_file.open(std::filesystem::path(file_name), std::ios_base::binary | std::ios_base::out | std::ios_base::in);

	if (!_file.is_open())
	{
		std::println("[FDD] Inserted with {} but failed (file not found?)", file_name);
		return false;
	}

	std::println("[FDD] Inserted {}", file_name);

	size_t floppy_total_bytes{ std::filesystem::file_size(file_name) };

	_type = Type::Size_360K; // Assume default 360k in ibm pc xt

	for (int i{}; i < fd_info.size(); i++)
	{
		if (floppy_total_bytes == fd_info[i].cylinders * fd_info[i].heads * fd_info[i].spt * _sector_size)
		{
			_type = static_cast<Type>(i);
			break;
		}
	}
}

bool FDD::seek(int cylinder, int head, int sector)
{
	std::println("[FDD] Seek to C:{}, H:{}, S:{}", cylinder, head, sector);

	int lba{ ((cylinder * fd_info[std::to_underlying<Type>(_type)].heads + head) * fd_info[std::to_underlying<Type>(_type)].spt + (sector - 1)) * _sector_size};

	_file.seekp(lba, std::ios_base::beg);
	_file.seekp(lba, std::ios_base::beg);

	return _file.bad() || _file.eof();
}

bool FDD::write(uint8_t* data, size_t size)
{
	_file.write(reinterpret_cast<char*>(data), size);
	return _file.bad() || _file.eof();
}

bool FDD::read(uint8_t* data, size_t size)
{
	std::println("[FDD] Read {} bytes, or {} sectors at C:{}, H:{}, S:{}", size, size / _sector_size, cylinder(), head(), sector());
	_file.read(reinterpret_cast<char*>(data), size);
	return _file.bad() || _file.eof();
}

FDC::FDC(PIC& pic, DMA& dma, std::array<FDD, 4>& fdds) : _pic{ pic }, _dma{ dma }, _fdds{ fdds }
{
	std::println("[FDC] Init...");
}

bool FDC::write(int address, uint8_t data, bool io)
{
	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Data_IO:
		{
			_data[_data_current] = data;

			if (_data_current == 0)
			{
				_msr |= msr_busy;

				uint8_t command{ static_cast<uint8_t>(_data[_data_current] & 0x1F) }; // need f and 0

				if (command == data_command_sense_interrupt)
				{
					_data_bytes = 1;
				}

				else if (command == data_command_specify)
				{
					_data_bytes = 3;
				}

				else if (command == data_command_recalibrate)
				{
					_data_bytes = 2;
				}

				else if (command == data_command_seek)
				{
					_data_bytes = 3;
				}

				else if (command == data_command_read)
				{
					_data_bytes = 9;
				}

				else if (command == data_command_read_id)
				{
					_data_bytes = 2;
				}

				else
				{
					std::println("[FDC] Unknown Command {:x}\n", command);
				}

				_msr &= ~msr_data_io;
			}

			if (_data_bytes - 1 == _data_current) // execution phase
			{
				if (_data_bytes > 1)
				{
					_selected_fdd = &_fdds[_data[1] & 0x3];
				}

				_data_current = 0;
				// setting data bytes here is the amount read from the data stack
				switch (_data[0] & 0x1F)
				{
				case data_command_sense_interrupt:
				{
					_data[0] = _st[0]; // Send st0 with sense interrupt
					_data[1] = _selected_fdd->cylinder(); // PCN, cylinder number at last interrupt
					_data_bytes = 2;
					_result_phase = true;
					break;
				}
				case data_command_recalibrate:
				{
					_selected_fdd->seek(0, 0, 1);
					// finished (no result phase)
					_data_bytes = 0;
					_st[0] = st0_sk_end | (static_cast<uint8_t>(_selected_fdd->head()) << 2) | (_data[1] & 0x3); // for sense interrupt
					_pic.raise(PIC::IRQ::Floppy_Ctrl);
					break;
				}
				case data_command_seek:
				{
					_data_bytes = 0;
					_selected_fdd->seek(_data[2], _data[1] & 0x4, _selected_fdd->sector());
					_st[0] = st0_sk_end | (static_cast<uint8_t>(_selected_fdd->head()) << 2) | (_data[1] & 0x3); // for sense interrupt
					_pic.raise(PIC::IRQ::Floppy_Ctrl);
					break;
				}
				case data_command_read_id:
				{
					_data[6] = 2;
					_data_bytes = 7;
					_result_phase = true;
					break;
				}
				case data_command_read:
				{
					uint8_t cylinder_number = _data[2];
					uint8_t head_number = _data[3];
					uint8_t sector_number = _data[4];
					uint8_t bytes_per_sector = _data[5];
					uint8_t end_of_track = _data[6];
					uint8_t data_length = _data[8];
					
					int bytes_read{};

					if (bytes_per_sector == 2)
					{
						bytes_read = 512;
					}

					else
					{
						bytes_read = data_length;
					}

					_data[6] = bytes_per_sector;

					int mt_read{1};

					if (_data[0] & 0x80) // multi track read
					{
						mt_read = 2;
					}

					auto fdd_buffer{ std::make_unique<uint8_t[]>(((end_of_track - (sector_number - 1)) * bytes_read) * mt_read) };

					_selected_fdd->seek(cylinder_number, head_number, sector_number);
					_selected_fdd->read(fdd_buffer.get(), (end_of_track - (sector_number - 1)) * bytes_read);

					if (_data[0] & 0x80) // multi track read
					{
						if (_selected_fdd->head() == 0)
						{
							_selected_fdd->seek(cylinder_number, 1, sector_number);
						}

						else
						{
							_selected_fdd->seek(cylinder_number, 0, sector_number);
						}

						_selected_fdd->read(&fdd_buffer.get()[(end_of_track - (sector_number - 1)) * bytes_read - 1], (end_of_track - (sector_number - 1)) * bytes_read);
					}

					_dma.operation(2, fdd_buffer);

					_data_bytes = 7;

					_st[0] = (head_number << 2) | _data[1] & 0x3;
					_st[1] = 0;
					_st[2] = 0;

					_pic.raise(PIC::IRQ::Floppy_Ctrl);
					_result_phase = true;
					break;
				}
				}

				// result phase
				if (_result_phase)
				{
					if (_data_bytes == 7)
					{
						_data[0] = _st[0];
						_data[1] = _st[1];
						_data[2] = _st[2];

						_data[3] = _selected_fdd->cylinder();
						_data[4] = _selected_fdd->head();
						_data[5] = _selected_fdd->sector();
					}

					_msr |= msr_busy;
					_msr |= msr_data_io;
				}
			}

			else
			{
				_data_current++;
			}
			break;
		}
		case Port::Dor:
		{
			_selected_fdd = &_fdds[data & dor_drive];

			if ((data & dor_reset_off) == 0) // reset
			{
				_msr &= ~msr_data_io;
				_st[0] = (3 << 6) | (static_cast<uint8_t>(_selected_fdd->head()) << 2) | (data & dor_drive);
				_pic.raise(PIC::IRQ::Floppy_Ctrl);
				_msr |= msr_rqm;
				_data_bytes = 0;
				_data_current = 0;
			}

			if (data & dor_motor_a_on || data & dor_motor_b_on || data & dor_motor_c_on || data & dor_motor_d_on) // one of the motors turned on
			{
				_st[0] = (3 << 6) | (static_cast<uint8_t>(_selected_fdd->head()) << 2) | (data & dor_drive);
				_msr &= ~msr_data_io;
				_pic.raise(PIC::IRQ::Floppy_Ctrl);
				_msr |= msr_rqm;
			}

			if (data & dor_dma_irq_on)
			{
				_msr &= ~msr_non_dma_mode;
			}

			else
			{
				_msr |= msr_non_dma_mode;
			}

			break;
		}
		default:
		{
			return false;
			break;
		}
		}
	}

	else
	{
		return false;
	}

	return true;
}

std::optional<uint8_t> FDC::read(int address, bool io)
{
	if (io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Msr:
		{
			return _msr;
			break;
		}
		case Port::Data_IO:
		{
			uint8_t data{ _data[_data_current] };

			_data_current++;

			if (_data_current >= _data_bytes)
			{
				_msr &= ~msr_data_io;
				_msr &= ~msr_busy;
				_data_current = 0;
				_data_bytes = 0;
				_result_phase = false;
				return data;
			}

			else
			{
				_msr |= msr_busy;
				_msr |= msr_data_io; // Make sure we still in read mode
				return data;
			}

			break;
		}
		}
	}

	return std::nullopt;
}
