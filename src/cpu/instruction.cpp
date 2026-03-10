#include "cpu.h"
#include "to_linear.h"

CPU::Instruction::Instruction(CPU& cpu) : _cpu{ cpu }
{
	_segment = _cpu.get_sreg(Segment_Registers::CS);
	_offset = _cpu.get_reg16(Registers::IP);
	_data_seg = _cpu.get_sreg(Segment_Registers::DS);

	bool prefix_instruction_done{};

	while (!prefix_instruction_done)
	{
		CPU_Opcode cur_byte = static_cast<CPU_Opcode>(_cpu._bus.read8(to_linear(_segment, _offset + _length), false));

		switch (cur_byte)
		{
		case CPU_Opcode::PREFIX_ES:
		case CPU_Opcode::PREFIX_CS:
		case CPU_Opcode::PREFIX_SS:
		case CPU_Opcode::PREFIX_DS:
		{
			_length++;
			_data_seg = _cpu.get_sreg(static_cast<Segment_Registers>((std::to_underlying<CPU_Opcode>(cur_byte) >> 3) & 0x3));
			_default_seg = false;
			break;
		}
		case CPU_Opcode::PREFIX_REP_OR_REPE:
		case CPU_Opcode::PREFIX_REPNE:
		{
			_rep = cur_byte; // repetition
			_length++;
			break;
		}
		case static_cast<CPU_Opcode>(0xf0):
		{
			_length++;
			break;
		}
		default:
		{
			prefix_instruction_done = 1;
		}
		}
	}

	//http://forthworks.com:8800/temp/opcodes.html

	CPU_Opcode opcode = static_cast<CPU_Opcode>(_cpu._bus.read8(to_linear(_segment, _offset + _length), false));

	_operation = opcode;

	_length++;

	_width = opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x1;

	_data1_width = opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x100;
	_data2_width = opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x200;

	_reg = std::make_unique<Operand>(_cpu, _width);
	_regmem = std::make_unique<Operand>(_cpu, _width);

	if (_width) // width
	{
		_reg->get_addr16() = &_cpu.get_reg16(Registers::AX);
	}

	else
	{
		_reg->get_addr8() = &_cpu.get_reg8(Registers_8::AL);
	}

	if (opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x4) // modrm byte?
	{
		calc_modrm_byte(to_linear(_segment, _offset + _length), opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x80);
	}

	if (opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x8 || (opcode == CPU_Opcode::GROUP_OPCODE_F7 && (_reg->get_addr16() == &_cpu.get_reg16(Registers::AX))) || (opcode == CPU_Opcode::GROUP_OPCODE_F6 && _reg->get_addr8() == &_cpu.get_reg8(Registers_8::AL))) // data 1.
	{
		if (_data1_width == 0)
		{
			_data1.data8[0] = _cpu._bus.read8(to_linear(_segment, _offset + _length), false);
			_length++;
		}

		else
		{
			_data1.data16 = _cpu._bus.read16(to_linear(_segment, _offset + _length), false);
			_length += 2;
		}
	}

	if (opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x10) // data 2.
	{
		if (_data2_width == 0)
		{
			_data2.data8[0] = _cpu._bus.read8(to_linear(_segment, _offset + _length), false);
			_length++;
		}

		else
		{
			_data2.data16 = _cpu._bus.read16(to_linear(_segment, _offset + _length), false);
			_length += 2;
		}
	}

	if (opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x20) // is segment in the opcode?
	{
		_reg->get_addr16() = &_cpu.get_sreg(static_cast<Segment_Registers>((std::to_underlying<CPU_Opcode>(opcode) >> 3) & 0x3));
	}

	else if (opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x40) // is register in opcode last 3 bits?
	{
		if (_width)
		{
			_reg->get_addr16() = &_cpu.get_reg16(static_cast<Registers>(std::to_underlying<CPU_Opcode>(opcode) & 0x7));
		}

		else
		{
			_reg->get_addr8() = &_cpu.get_reg8(static_cast<Registers_8>(std::to_underlying<CPU_Opcode>(opcode) & 0x7));
		}
	}

	if (opcode_desc[std::to_underlying<CPU_Opcode>(opcode)] & 0x2) // reg/regmem direction
	{
		_regmem_to_reg = 1;
	}
}