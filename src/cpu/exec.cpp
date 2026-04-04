#include "cpu.h"
#include <print>
#include "to_linear.h"

void CPU::Instruction::exec()
{
	_cpu.ticks_total++;

	bool exec{ true };
	bool increase_ip{ true };
	bool dec_cx{};

	if (_cpu._sti_enable)
	{
		_cpu.get_reg16(Registers::Flags) |= flag_interrupt;
		_cpu._sti_enable = false;
	}

	if (!_cpu._halted)
	{
		switch (_rep)
		{
		case CPU_Opcode::PREFIX_REP_OR_REPE:
		{
			if (_cpu._zero_flag_check) // this is for instructions that check if the zero flag is on, but it uses the same rep prefix byte.
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_zero) && _cpu.get_reg16(Registers::CX) != 0)
				{
					increase_ip = false;
					dec_cx = true;
				}

				else
				{
					exec = false;
				}

				_cpu._zero_flag_check = false;
			}

			else
			{
				if (_cpu.get_reg16(Registers::CX) != 0)
				{
					dec_cx = true;
					increase_ip = false;
				}

				else
				{
					exec = false;
				}
			}
			break;
		}

		case CPU_Opcode::PREFIX_REPNE:
		{
			if ((_cpu.get_reg16(Registers::Flags) & flag_zero) == 0 && _cpu.get_reg16(Registers::CX) != 0)
			{
				increase_ip = false;
				dec_cx = true;
			}

			else
			{
				exec = false;
			}

			break;
		}
		}

		if (exec)
		{
			uint8_t group_opcode_instruction{};

			for (int i{}; i < 8; i++)
			{
				if (_width)
				{
					if (_reg->get_addr16() == &_cpu.get_reg16(static_cast<Registers>(i)))
					{
						group_opcode_instruction = i;
						break;
					}
				}

				else
				{
					if (_reg->get_addr8() == &_cpu.get_reg8(static_cast<Registers_8>(i)))
					{
						group_opcode_instruction = i;
						break;
					}
				}
			}

			switch (_operation) // actual opcode
			{
			case CPU_Opcode::GROUP_OPCODE_80:
			{
				switch (static_cast<CPU_Group_Opcode_80>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_80::ADD_RM8_IMM8:
				{
					*_regmem += _data1;
					break;
				}
				case CPU_Group_Opcode_80::ADC_RM8_IMM8:
				{
					uint8_t temp{ static_cast<uint8_t>(_data1.data8[0] + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
					*_regmem += temp;
					break;
				}
				case CPU_Group_Opcode_80::SBB_RM8_IMM8:
				{
					uint8_t temp{ static_cast<uint8_t>(_data1.data8[0] + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
					*_regmem -= temp;
					break;
				}
				case CPU_Group_Opcode_80::AND_RM8_IMM8:
				{
					*_regmem &= _data1;
					break;
				}
				case CPU_Group_Opcode_80::CMP_RM8_IMM8: // 80 mm ii
				{
					uint8_t temp_var{ _regmem->get_val8() };
					*_regmem -= _data1;
					*_regmem = temp_var;
					break;
				}
				case CPU_Group_Opcode_80::SUB_RM8_IMM8:
				{
					*_regmem -= _data1;
					break;
				}
				case CPU_Group_Opcode_80::OR_RM8_IMM8:
				{
					*_regmem |= _data1;
					break;
				}
				case CPU_Group_Opcode_80::XOR_RM8_IMM8:
				{
					*_regmem ^= _data1;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}
				break;
			}
			case CPU_Opcode::GROUP_OPCODE_81:
			{
				switch (static_cast<CPU_Group_Opcode_81>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_81::ADD_RM16_IMM16:
				{
					*_regmem += _data1;
					break;
				}
				case CPU_Group_Opcode_81::ADC_RM16_IMM16:
				{
					uint16_t temp{ static_cast<uint16_t>(_data1.data16 + (_cpu.get_reg16(Registers::Flags) & flag_carry)) };
					*_regmem += temp;
					break;
				}
				case CPU_Group_Opcode_81::SBB_RM16_IMM16:
				{
					uint16_t temp{ static_cast<uint16_t>(_data1.data16 + (_cpu.get_reg16(Registers::Flags) & flag_carry)) };
					*_regmem -= temp;
					break;
				}
				case CPU_Group_Opcode_81::AND_RM16_IMM16:
				{
					*_regmem &= _data1;
					break;
				}
				case CPU_Group_Opcode_81::CMP_RM16_IMM16: // 83 mm ii
				{
					uint16_t temp_var{ _regmem->get_val16() };
					*_regmem -= _data1;
					*_regmem = temp_var;
					break;
				}
				case CPU_Group_Opcode_81::SUB_RM16_IMM16:
				{
					*_regmem -= _data1;
					break;
				}
				case CPU_Group_Opcode_81::OR_RM16_IMM16:
				{
					*_regmem |= _data1;
					break;
				}
				case CPU_Group_Opcode_81::XOR_RM16_IMM16:
				{
					*_regmem ^= _data1;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}
				break;
			}
			case CPU_Opcode::GROUP_OPCODE_83:
			{
				switch (static_cast<CPU_Group_Opcode_83>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_83::ADD_RM16_IMM8:
				{
					*_regmem += _data1;
					break;
				}
				case CPU_Group_Opcode_83::ADC_RM16_IMM8:
				{
					uint8_t temp{ static_cast<uint8_t>(_data1.data8[0] + static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry)) };
					*_regmem += temp;
					break;
				}
				case CPU_Group_Opcode_83::SBB_RM16_IMM8:
				{
					uint8_t temp{ static_cast<uint8_t>(_data1.data8[0] + static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry)) };
					*_regmem -= temp;
					break;
				}
				case CPU_Group_Opcode_83::AND_RM16_IMM8:
				{
					*_regmem &= _data1;
					break;
				}
				case CPU_Group_Opcode_83::CMP_RM16_IMM8: // 83 mm ii
				{
					uint16_t temp_var{ _regmem->get_val16() };
					*_regmem -= _data1;
					*_regmem = temp_var;
					break;
				}
				case CPU_Group_Opcode_83::SUB_RM16_IMM8:
				{
					*_regmem -= _data1;
					break;
				}
				case CPU_Group_Opcode_83::OR_RM16_IMM8:
				{
					*_regmem |= _data1;
					break;
				}
				case CPU_Group_Opcode_83::XOR_RM16_IMM8:
				{
					*_regmem ^= _data1;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}
				break;
			}
			case CPU_Opcode::GROUP_OPCODE_C0:
			{
				switch (static_cast<CPU_Group_Opcode_C0>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_C0::ROL_RM8_IMM8:
				{
					_regmem->rol(_data1);
					break;
				}
				case CPU_Group_Opcode_C0::ROR_RM8_IMM8:
				{
					_regmem->ror(_data1);
					break;
				}
				case CPU_Group_Opcode_C0::RCL_RM8_IMM8:
				{
					_regmem->rcl(_data1);
					break;
				}
				case CPU_Group_Opcode_C0::RCR_RM8_IMM8:
				{
					_regmem->rcr(_data1);
					break;
				}
				case CPU_Group_Opcode_C0::SAL_RM8_IMM8: // D0 mm
				{
					*_regmem <<= _data1;
					break;
				}
				case CPU_Group_Opcode_C0::SAR_RM8_IMM8: // D0 mm
				{
					_regmem->sar(_data1);
					break;
				}
				case CPU_Group_Opcode_C0::SHR_RM8_IMM8: // D0 mm
				{
					*_regmem >>= _data1;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}

				break;
			}
			case CPU_Opcode::GROUP_OPCODE_C1:
			{
				switch (static_cast<CPU_Group_Opcode_C1>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_C1::ROL_RM16_IMM8:
				{
					_regmem->rol(_data1);
					break;
				}
				case CPU_Group_Opcode_C1::ROR_RM16_IMM8:
				{
					_regmem->ror(_data1);
					break;
				}
				case CPU_Group_Opcode_C1::RCL_RM16_IMM8:
				{
					_regmem->rcl(_data1);
					break;
				}
				case CPU_Group_Opcode_C1::RCR_RM16_IMM8:
				{
					_regmem->rcr(_data1);
					break;
				}
				case CPU_Group_Opcode_C1::SAL_RM16_IMM8: // D0 mm
				{
					*_regmem <<= _data1;
					break;
				}
				case CPU_Group_Opcode_C1::SAR_RM16_IMM8: // D0 mm
				{
					_regmem->sar(_data1);
					break;
				}
				case CPU_Group_Opcode_C1::SHR_RM16_IMM8: // D0 mm
				{
					*_regmem >>= _data1;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}

				break;
			}
			case CPU_Opcode::GROUP_OPCODE_D0:
			{
				switch (static_cast<CPU_Group_Opcode_D0>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_D0::ROL_RM8_1:
				{
					_regmem->rol(1);
					break;
				}
				case CPU_Group_Opcode_D0::ROR_RM8_1:
				{
					_regmem->ror(1);
					break;
				}
				case CPU_Group_Opcode_D0::RCL_RM8_1:
				{
					_regmem->rcl(1);
					break;
				}
				case CPU_Group_Opcode_D0::RCR_RM8_1:
				{
					_regmem->rcr(1);
					break;
				}
				case CPU_Group_Opcode_D0::SAL_RM8_1: // D0 mm
				{
					*_regmem <<= 1;
					break;
				}
				case CPU_Group_Opcode_D0::SAR_RM8_1: // D0 mm
				{
					_regmem->sar(1);
					break;
				}
				case CPU_Group_Opcode_D0::SHR_RM8_1: // D0 mm
				{
					*_regmem >>= 1;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}

				break;
			}
			case CPU_Opcode::GROUP_OPCODE_D1:
			{
				switch (static_cast<CPU_Group_Opcode_D1>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_D1::ROL_RM16_1:
				{
					_regmem->rol(1);
					break;
				}
				case CPU_Group_Opcode_D1::ROR_RM16_1:
				{
					_regmem->ror(1);
					break;
				}
				case CPU_Group_Opcode_D1::RCL_RM16_1:
				{
					_regmem->rcl(1);
					break;
				}
				case CPU_Group_Opcode_D1::RCR_RM16_1:
				{
					_regmem->rcr(1);
					break;
				}
				case CPU_Group_Opcode_D1::SAL_RM16_1: // D0 mm
				{
					*_regmem <<= 1;
					break;
				}
				case CPU_Group_Opcode_D1::SAR_RM16_1: // D0 mm
				{
					_regmem->sar(1);
					break;
				}
				case CPU_Group_Opcode_D1::SHR_RM16_1: // D0 mm
				{
					*_regmem >>= 1;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}

				break;
			}
			case CPU_Opcode::GROUP_OPCODE_D2:
			{
				switch (static_cast<CPU_Group_Opcode_D2>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_D2::ROL_RM8_CL:
				{
					_regmem->rol(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D2::ROR_RM8_CL:
				{
					_regmem->ror(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D2::RCL_RM8_CL:
				{
					_regmem->rcl(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D2::RCR_RM8_CL:
				{
					_regmem->rcr(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D2::SAL_RM8_CL: // D0 mm
				{
					*_regmem <<= _cpu.get_reg8(Registers_8::CL);
					break;
				}
				case CPU_Group_Opcode_D2::SAR_RM8_CL: // D0 mm
				{
					_regmem->sar(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D2::SHR_RM8_CL: // D0 mm
				{
					*_regmem >>= _cpu.get_reg8(Registers_8::CL);
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);

					break;
				}
				}
				break;
			}
			case CPU_Opcode::GROUP_OPCODE_D3:
			{
				switch (static_cast<CPU_Group_Opcode_D3>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_D3::ROL_RM16_CL:
				{
					_regmem->rol(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D3::ROR_RM16_CL:
				{
					_regmem->ror(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D3::RCL_RM16_CL:
				{
					_regmem->rcl(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D3::RCR_RM16_CL:
				{
					_regmem->rcr(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D3::SAL_RM16_CL: // D0 mm
				{
					*_regmem <<= _cpu.get_reg8(Registers_8::CL);
					break;
				}
				case CPU_Group_Opcode_D3::SAR_RM16_CL: // D0 mm
				{
					_regmem->sar(_cpu.get_reg8(Registers_8::CL));
					break;
				}
				case CPU_Group_Opcode_D3::SHR_RM16_CL: // D0 mm
				{
					*_regmem >>= _cpu.get_reg8(Registers_8::CL);
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);
					break;
				}
				}
				break;
			}
			case CPU_Opcode::GROUP_OPCODE_F6:
			{
				switch (static_cast<CPU_Group_Opcode_F6>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_F6::TEST_RM8_IMM8:
				{
					uint8_t temp_var{ _regmem->get_val8() };
					*_regmem &= _data1;
					*_regmem = temp_var;
					break;
				}
				case CPU_Group_Opcode_F6::MUL_RM8: // F6 mm
				{
					_regmem->mul();
					break;
				}
				case CPU_Group_Opcode_F6::IMUL_RM8:
				{
					_regmem->imul();
					break;
				}
				case CPU_Group_Opcode_F6::NEG_RM8:
				{
					_regmem->neg();
					break;
				}
				case CPU_Group_Opcode_F6::NOT_RM8:
				{
					*_regmem = ~(_regmem->get_val8());
					break;
				}
				case CPU_Group_Opcode_F6::DIV_RM8:
				{
					_regmem->div();
					break;
				}
				case CPU_Group_Opcode_F6::IDIV_RM8:
				{
					_regmem->idiv();
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);
					break;
				}
				}
				break;
			}
			case CPU_Opcode::GROUP_OPCODE_F7:
			{
				switch (static_cast<CPU_Group_Opcode_F7>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_F7::TEST_RM16_IMM16:
				{
					uint16_t temp_var{ _regmem->get_val16() };
					*_regmem &= _data1;
					*_regmem = temp_var;
					break;
				}
				case CPU_Group_Opcode_F7::MUL_RM16: // F7 mm
				{
					_regmem->mul();
					break;
				}
				case CPU_Group_Opcode_F7::NEG_RM16:
				{
					_regmem->neg();
					break;
				}
				case CPU_Group_Opcode_F7::NOT_RM16:
				{
					*_regmem = ~(_regmem->get_val16());
					break;
				}
				case CPU_Group_Opcode_F7::DIV_RM16:
				{
					_regmem->div();
					break;
				}
				case CPU_Group_Opcode_F7::IDIV_RM16:
				{
					_regmem->idiv();
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);
					break;
				}
				}
				break;
			}
			case CPU_Opcode::GROUP_OPCODE_FE:
			{
				switch (static_cast<CPU_Group_Opcode_FE>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_FE::DEC_RM8: // FE mm dd dd
				{
					(*_regmem)--;
					break;
				}
				case CPU_Group_Opcode_FE::INC_RM8: // FE mm dd dd
				{
					(*_regmem)++;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);
					break;
				}
				}

				break;
			}
			case CPU_Opcode::GROUP_OPCODE_FF:
			{
				switch (static_cast<CPU_Group_Opcode_FF>(group_opcode_instruction))
				{
				case CPU_Group_Opcode_FF::DEC_RM16: // FF mm dd dd
				{
					(*_regmem)--;
					break;
				}
				case CPU_Group_Opcode_FF::CALL_RM16:
				{
					_cpu.push(_cpu.get_reg16(Registers::IP) + _length);
					_cpu.get_reg16(Registers::IP) = _regmem->get_val16();
					increase_ip = false;
					break;
				}
				case CPU_Group_Opcode_FF::CALL_M16_16: // FF mm
				{
					_cpu.push(_cpu.get_sreg(Segment_Registers::CS));
					_cpu.push(_cpu.get_reg16(Registers::IP) + _length);

					uint16_t offset{ _regmem->get_val16() };
					_regmem->get_addr16()++;
					uint16_t segment{ _regmem->get_val16() };

					_cpu.get_reg16(Registers::IP) = offset;
					_cpu.get_sreg(Segment_Registers::CS) = segment;
					increase_ip = false;
					break;
				}
				case CPU_Group_Opcode_FF::JMP_RM16: // FF mm dd dd
				{
					_cpu.get_reg16(Registers::IP) = _regmem->get_val16();
					increase_ip = false;
					break;
				}
				case CPU_Group_Opcode_FF::JMP_M16_16:
				{
					uint16_t offset = _regmem->get_val16();
					_regmem->get_addr16()++;
					uint16_t segment = _regmem->get_val16();

					_cpu.get_reg16(Registers::IP) = offset;
					_cpu.get_sreg(Segment_Registers::CS) = segment;
					increase_ip = false;
					break;
				}
				case CPU_Group_Opcode_FF::PUSH_RM16: // FF mm dd dd
				{
					_cpu.push(_regmem->get_val16());
					break;
				}
				case CPU_Group_Opcode_FF::INC_RM16: // FF mm dd dd
				{
					(*_regmem)++;
					break;
				}
				default:
				{
					printf("Unknown Opcode %x /%d\n", _operation, group_opcode_instruction);
					break;
				}
				}

				break;
			}
			case CPU_Opcode::AAD:
			{
				uint8_t tempal{ _cpu.get_reg8(Registers_8::AL) };
				uint8_t tempah{ _cpu.get_reg8(Registers_8::AH) };

				_cpu.get_reg8(Registers_8::AL) = (tempal + (tempah * _data1.data8[0])) & 0xff;
				_cpu.get_reg8(Registers_8::AH) = 0;

				_cpu.modify_flag_sign(static_cast<int16_t>(_cpu.get_reg8(Registers_8::AL)), false);
				_cpu.modify_flag_zero(static_cast<uint16_t>(_cpu.get_reg8(Registers_8::AL)));
				_cpu.modify_flag_parity(_cpu.get_reg8(Registers_8::AL));
				break;
			}
			case CPU_Opcode::AAM:
			{
				uint8_t temp{ _cpu.get_reg8(Registers_8::AL) };

				_cpu.get_reg8(Registers_8::AH) = temp / _data1.data8[0];
				_cpu.get_reg8(Registers_8::AL) = temp % _data1.data8[0];

				_cpu.modify_flag_sign(static_cast<int16_t>(_cpu.get_reg8(Registers_8::AL)), false);
				_cpu.modify_flag_zero(static_cast<uint16_t>(_cpu.get_reg8(Registers_8::AL)));
				_cpu.modify_flag_parity(_cpu.get_reg8(Registers_8::AL));
				break;
			}
			case CPU_Opcode::AAS:
			{
				if ((_cpu.get_reg8(Registers_8::AL) & 0xf) > 9 || _cpu.get_reg16(Registers::Flags) & flag_half_carry)
				{
					_cpu.get_reg16(Registers::AX) -= 6;
					_cpu.get_reg8(Registers_8::AH)--;
					_cpu.get_reg16(Registers::Flags) |= flag_half_carry;
					_cpu.get_reg16(Registers::Flags) |= flag_carry;
					_cpu.get_reg8(Registers_8::AL) &= 0xf;
				}

				else
				{
					_cpu.get_reg16(Registers::Flags) &= ~flag_half_carry;
					_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
					_cpu.get_reg8(Registers_8::AL) &= 0xf;
				}
				break;
			}
			case CPU_Opcode::ADC_AL_IMM8: // 04 ii
			{
				uint8_t temp{ static_cast<uint8_t>(_data1.data8[0] + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
				*_reg += temp;
				break;
			}
			case CPU_Opcode::ADC_AX_IMM16: // 05 ii ii
			{
				uint16_t temp{ static_cast<uint16_t>(_data1.data16 + (_cpu.get_reg16(Registers::Flags) & flag_carry)) };
				*_reg += temp;
				break;
			}
			case CPU_Opcode::ADC_RM8_R8:
			case CPU_Opcode::ADC_RM16_R16:
			case CPU_Opcode::ADC_R8_RM8:
			case CPU_Opcode::ADC_R16_RM16:
			{
				if (_regmem_to_reg)
				{
					if (_width)
					{
						uint16_t temp{ static_cast<uint16_t>(_regmem->get_val16() + (_cpu.get_reg16(Registers::Flags) & flag_carry)) };
						*_reg += temp;
					}

					else
					{
						uint8_t temp{ static_cast<uint8_t>(_regmem->get_val8() + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
						*_reg += temp;
					}
				}

				else
				{
					if (_width)
					{
						uint16_t temp{ static_cast<uint16_t>(_reg->get_val16() + (_cpu.get_reg16(Registers::Flags) & flag_carry)) };
						*_regmem += temp;
					}

					else
					{
						uint8_t temp{ static_cast<uint8_t>(_reg->get_val8() + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
						*_regmem += temp;
					}
				}
				break;
			}
			case CPU_Opcode::ADD_AL_IMM8: // 04 ii
			case CPU_Opcode::ADD_AX_IMM16: // 05 ii ii
			{
				*_reg += _data1;
				break;
			}
			case CPU_Opcode::ADD_RM8_R8: // 00 mm
			case CPU_Opcode::ADD_RM16_R16: // 01 mm		
			case CPU_Opcode::ADD_R8_RM8: // 02 mm
			case CPU_Opcode::ADD_R16_RM16: // 03 mm
			{
				if (_regmem_to_reg)
				{
					*_reg += *_regmem;
				}

				else
				{
					*_regmem += *_reg;
				}
				break;
			}
			case CPU_Opcode::AND_AL_IMM8:
			case CPU_Opcode::AND_AX_IMM16:
			{
				*_reg &= _data1;
				break;
			}
			case CPU_Opcode::AND_RM8_R8:
			case CPU_Opcode::AND_RM16_R16:
			case CPU_Opcode::AND_R8_RM8:
			case CPU_Opcode::AND_R16_RM16:
			{
				if (_regmem_to_reg)
				{
					*_reg &= *_regmem;
				}

				else
				{
					*_regmem &= *_reg;
				}

				break;
			}
			case CPU_Opcode::CALL_PTR16_16: // 9A ii ii ii ii
			{
				_cpu.push(_cpu.get_sreg(Segment_Registers::CS));
				_cpu.push(_cpu.get_reg16(Registers::IP) + _length);

				_cpu.get_sreg(Segment_Registers::CS) = _data1.data16;
				_cpu.get_reg16(Registers::IP) = _data2.data16;

				increase_ip = false;
				break;
			}
			case CPU_Opcode::CALL_REL16: // E8 ii ii
			{
				_cpu.push(_cpu.get_reg16(Registers::IP) + _length);

				int16_t call_value{ static_cast<int16_t>(_data1.data16 + _length) };

				_cpu.get_reg16(Registers::IP) += call_value;

				increase_ip = false;
				break;
			}
			case CPU_Opcode::CBW:
			{
				if ((int8_t)_cpu.get_reg8(Registers_8::AL) < 0)
				{
					_cpu.get_reg8(Registers_8::AH) = 0xFF;
				}

				else
				{
					_cpu.get_reg8(Registers_8::AH) = 0;
				}
				break;
			}
			case CPU_Opcode::CLC:
			{
				_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
				break;
			}
			case CPU_Opcode::CLD:
			{
				_cpu.get_reg16(Registers::Flags) &= ~flag_direction;
				break;
			}
			case CPU_Opcode::CLI:
			{
				_cpu.get_reg16(Registers::Flags) &= ~flag_interrupt;
				break;
			}
			case CPU_Opcode::CMC:
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_carry)
				{
					_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
				}

				else
				{
					_cpu.get_reg16(Registers::Flags) |= flag_carry;
				}

				break;
			}
			case CPU_Opcode::CMP_AL_IMM8: // 3C ii
			{
				uint8_t temp_var{ _reg->get_val8() };
				*_reg -= _data1;
				*_reg = temp_var;
				break;
			}
			case CPU_Opcode::CMP_AX_IMM16: // 3D ii ii
			{
				uint16_t temp_var{ _reg->get_val16() };
				*_reg -= _data1;
				*_reg = temp_var;
				break;
			}
			case CPU_Opcode::CMP_RM8_R8: // 38 mm
			case CPU_Opcode::CMP_RM16_R16: // 39 mm
			case CPU_Opcode::CMP_R8_RM8: // 3A mm
			case CPU_Opcode::CMP_R16_RM16: // 3B mm
			{
				if (_regmem_to_reg)
				{
					if (_width)
					{
						uint16_t temp_var{ _reg->get_val16() };
						*_reg -= *_regmem;
						*_reg = temp_var;
					}

					else
					{
						uint8_t temp_var{ _reg->get_val8() };
						*_reg -= *_regmem;
						*_reg = temp_var;
					}
				}

				else
				{
					if (_width)
					{
						uint16_t temp_var{ _regmem->get_val16() };
						*_regmem -= *_reg;
						*_regmem = temp_var;
					}

					else
					{
						uint8_t temp_var{ _regmem->get_val8() };
						*_regmem -= *_reg;
						*_regmem = temp_var;
					}
				}
				break;
			}
			case CPU_Opcode::CMPSB:
			{
				Instruction::Operand temp{ _cpu, _width };
				temp.get_addr8() = reinterpret_cast<uint8_t*>(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)));
				uint8_t temp_var{ temp.get_val8() };
				temp -= _cpu._bus.read8(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), false);
				temp = temp_var;

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI)--;
					_cpu.get_reg16(Registers::DI)--;
				}

				else
				{
					_cpu.get_reg16(Registers::SI)++;
					_cpu.get_reg16(Registers::DI)++;
				}

				_cpu._zero_flag_check = true;
				break;
			}
			case CPU_Opcode::CMPSW:
			{
				Instruction::Operand temp{ _cpu, _width };
				temp.get_addr16() = reinterpret_cast<uint16_t*>(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)));
				uint16_t temp_var{ temp.get_val16() };
				temp -= _cpu._bus.read16(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), false);
				temp = temp_var;

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI) -= 2;
					_cpu.get_reg16(Registers::DI) -= 2;
				}

				else
				{
					_cpu.get_reg16(Registers::SI) += 2;
					_cpu.get_reg16(Registers::DI) += 2;
				}

				_cpu._zero_flag_check = true;
				break;
			}
			case CPU_Opcode::CWD:
			{
				if ((int16_t)_cpu.get_reg16(Registers::AX) < 0)
				{
					_cpu.get_reg16(Registers::DX) = 0xFFFF;
				}

				else
				{
					_cpu.get_reg16(Registers::DX) = 0;
				}

				break;
			}
			case CPU_Opcode::DAA:
			{
				uint8_t old_al{ _cpu.get_reg8(Registers_8::AL) };
				bool old_cf{ static_cast<bool>(_cpu.get_reg16(Registers::Flags) & flag_carry) };
				_cpu.get_reg16(Registers::Flags) &= ~flag_carry;

				if(((_cpu.get_reg8(Registers_8::AL) & 0xf) > 9) || _cpu.get_reg16(Registers::Flags) & flag_half_carry)
				{
					Operand temp_al{ _cpu, false };
					temp_al.get_addr8() = &_cpu.get_reg8(Registers_8::AL);
					temp_al += 6;
					_cpu.get_reg16(Registers::Flags) |= old_cf;
					_cpu.get_reg16(Registers::Flags) |= flag_half_carry;
				}
				else
				{
					_cpu.get_reg16(Registers::Flags) &= ~flag_half_carry;
				}
				if ((old_al > 0x99) || (old_cf == true)) 
				{
					_cpu.get_reg8(Registers_8::AL) += 0x60;
					_cpu.get_reg16(Registers::Flags) |= flag_carry;
				}
				else
				{
					_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
				}
				break;
			}
			case CPU_Opcode::DAS:
			{
				uint8_t old_al{ _cpu.get_reg8(Registers_8::AL) };
				bool old_cf{ static_cast<bool>(_cpu.get_reg16(Registers::Flags) & flag_carry) };

				_cpu.get_reg16(Registers::Flags) &= ~flag_carry;

				if ((_cpu.get_reg8(Registers_8::AL) & 0xf) > 9 || _cpu.get_reg16(Registers::Flags) & flag_half_carry)
				{
					_cpu.get_reg8(Registers_8::AL) -= 6;

					if (old_cf)
					{
						_cpu.get_reg16(Registers::Flags) |= flag_carry;
					}

					else
					{
						_cpu.get_reg16(Registers::Flags) &= ~flag_carry;
					}

					_cpu.get_reg16(Registers::Flags) |= flag_half_carry;
				}

				else
				{
					_cpu.get_reg16(Registers::Flags) &= ~flag_half_carry;
				}

				if ((old_al > 0x99) || old_cf)
				{
					_cpu.get_reg8(Registers_8::AL) -= 0x60;
					_cpu.get_reg16(Registers::Flags) |= flag_carry;
				}
				break;
			}
			// 0x48 + i
			case CPU_Opcode::DEC_AX:
			case CPU_Opcode::DEC_CX:
			case CPU_Opcode::DEC_DX:
			case CPU_Opcode::DEC_BX:
			case CPU_Opcode::DEC_SP:
			case CPU_Opcode::DEC_BP:
			case CPU_Opcode::DEC_SI:
			case CPU_Opcode::DEC_DI:
			{
				(*_reg)--;
				break;
			}
			case CPU_Opcode::HLT:
			{
				_cpu._halted = true;
				break;
			}
			case CPU_Opcode::IN_AL_IMM8:
			{
				_cpu.get_reg8(Registers_8::AL) = _cpu._bus.read8(_data1.data8[0], true);
				break;
			}
			case CPU_Opcode::IN_AX_IMM8:
			{
				_cpu.get_reg16(Registers::AX) = _cpu._bus.read16(_data1.data8[0], true);
				break;
			}
			case CPU_Opcode::IN_AL_DX:
			{
				_cpu.get_reg8(Registers_8::AL) = _cpu._bus.read8(_cpu.get_reg16(Registers::DX), true);
				break;
			}
			case CPU_Opcode::IN_AX_DX:
			{
				_cpu.get_reg16(Registers::AX) = _cpu._bus.read16(_cpu.get_reg16(Registers::DX), true);
				break;
			}
			case CPU_Opcode::INSB:
			{
				_cpu.get_reg8(Registers_8::AL) = _cpu._bus.read8(_cpu.get_reg16(Registers::DX), true);

				_cpu._bus.write8(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), _cpu.get_reg8(Registers_8::AL), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::DI)--;
				}

				else
				{
					_cpu.get_reg16(Registers::DI)++;
				}

				break;
			}
			case CPU_Opcode::INSW:
			{
				_cpu.get_reg16(Registers::AX) = _cpu._bus.read16(_cpu.get_reg16(Registers::DX), 1);

				_cpu._bus.write16(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), _cpu.get_reg16(Registers::AX), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::DI) -= 2;
				}

				else
				{
					_cpu.get_reg16(Registers::DI) += 2;
				}

				break;
			}
			// 40 + i
			case CPU_Opcode::INC_AX:
			case CPU_Opcode::INC_CX:
			case CPU_Opcode::INC_DX:
			case CPU_Opcode::INC_BX:
			case CPU_Opcode::INC_SP:
			case CPU_Opcode::INC_BP:
			case CPU_Opcode::INC_SI:
			case CPU_Opcode::INC_DI:
			{
				(*_reg)++;
				break;
			}
			case CPU_Opcode::INT_IMM8: // CD ii
			{
				uint8_t interrupt{ _data1.data8[0] };

				// Look up in interrupt vector table. Example: int 0x10, 0x10 * 4 = 0x40, get offset at 0x40 and segment at 0x42, jump there 

				uint16_t interrupt_offset{ _cpu._bus.read16(to_linear(0, interrupt * 4), 0) };
				uint16_t interrupt_segment{ _cpu._bus.read16(to_linear(0, interrupt * 4) + 2, 0) };

				_cpu.get_reg16(Registers::Flags) &= ~flag_interrupt;

				_cpu.push(_cpu.get_reg16(Registers::Flags));
				_cpu.push(_cpu.get_sreg(Segment_Registers::CS));
				_cpu.push(_cpu.get_reg16(Registers::IP) + _length);

				_cpu.get_sreg(Segment_Registers::CS) = interrupt_segment;
				_cpu.get_reg16(Registers::IP) = interrupt_offset;

				increase_ip = false;
				break;
			}
			case CPU_Opcode::IRET: // CF
			{
				// pop ip, cs, and flags after interrupt

				_cpu.get_reg16(Registers::IP) = _cpu.pop();
				_cpu.get_sreg(Segment_Registers::CS) = _cpu.pop();
				_cpu.get_reg16(Registers::Flags) = _cpu.pop();

				increase_ip = false;
				break;
			}
			case CPU_Opcode::JA_REL8: // 77 ii
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_carry) == 0 && (_cpu.get_reg16(Registers::Flags) & flag_zero) == 0)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JAE_REL8: // 73 ii // Same as JNB_REL8, JNC_REL8
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_carry) == 0)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JB_REL8: // 72 ii // Same as JC_REL8, JNAE_REL8
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_carry)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JBE_REL8: // 76 ii // Same as JNA_REL8
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_carry || _cpu.get_reg16(Registers::Flags) & flag_zero)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JCXZ_REL8: // E3 ii
			{
				if (_cpu.get_reg16(Registers::CX) == 0)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case static_cast<CPU_Opcode>(0x64):
			case CPU_Opcode::JE_REL8: // 74 ii // Same as JZ_REL8
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_zero)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JG_REL8: // 7f ii // Same as JNLE_REL8
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_zero) == 0 && (((_cpu.get_reg16(Registers::Flags) & flag_sign) == 0 && (_cpu.get_reg16(Registers::Flags) & flag_overflow) == 0) || ((_cpu.get_reg16(Registers::Flags) & flag_sign) && (_cpu.get_reg16(Registers::Flags) & flag_overflow))))
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JGE_REL8: // 7D ii // Same as JNL_REL8
			{
				if (((_cpu.get_reg16(Registers::Flags) & flag_sign) == 0 && (_cpu.get_reg16(Registers::Flags) & flag_overflow) == 0) || ((_cpu.get_reg16(Registers::Flags) & flag_sign) && (_cpu.get_reg16(Registers::Flags) & flag_overflow)))
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JL_REL8: // 7c ii // Same as JNGE_REL8
			{
				if (((_cpu.get_reg16(Registers::Flags) & flag_sign) == 0 && _cpu.get_reg16(Registers::Flags) & flag_overflow) || (_cpu.get_reg16(Registers::Flags) & flag_sign && (_cpu.get_reg16(Registers::Flags) & flag_overflow) == 0))
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JLE_REL8: // 7E ii // Same as JNG_REL8
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_zero || (((_cpu.get_reg16(Registers::Flags) & flag_sign) == 0 && _cpu.get_reg16(Registers::Flags) & flag_overflow) || (_cpu.get_reg16(Registers::Flags) & flag_sign && (_cpu.get_reg16(Registers::Flags) & flag_overflow) == 0)))
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JNE_REL8: // 75 ii // Same as JNZ_REL8
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_zero) == 0)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JNO_REL8: // 71 ii
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_overflow) == 0)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JNP_REL8: // 7b ii // Same as JPO_REL8
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_parity) == 0)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case static_cast<CPU_Opcode>(0x69):
			case CPU_Opcode::JNS_REL8: // 79 ii
			{
				if ((_cpu.get_reg16(Registers::Flags) & flag_sign) == 0)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JO_REL8: // 77 ii
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_overflow)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JP_REL8: // 7A ii // Same as JPE_REL8
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_parity)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JS_REL8: // 78 ii
			{
				if (_cpu.get_reg16(Registers::Flags) & flag_sign)
				{
					_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
					increase_ip = false;
				}
				break;
			}
			case CPU_Opcode::JMP_REL8: // EB ii
			{
				_cpu.get_reg16(Registers::IP) += static_cast<int8_t>(_data1.data8[0]) + _length;
				increase_ip = false;
				break;
			}
			case CPU_Opcode::JMP_REL16: // E9 ii ii
			{
				_cpu.get_reg16(Registers::IP) += static_cast<int16_t>(_data1.data16) + _length;
				increase_ip = false;
				break;
			}
			case CPU_Opcode::JMP_PTR16_16: // EA ii ii ii ii
			{
				_cpu.get_reg16(Registers::IP) = _data1.data16;
				_cpu.get_sreg(Segment_Registers::CS) = _data2.data16;
				increase_ip = false;
				break;
			}
			case CPU_Opcode::LAHF:
			{
				_cpu.get_reg8(Registers_8::AH) = _cpu.get_reg16(Registers::Flags) & 0x00ff;
				break;
			}
			case CPU_Opcode::LDS:
			{
				*_reg = *_regmem;
				_regmem->get_addr16()++;
				_cpu.get_sreg(Segment_Registers::DS) = _regmem->get_val16();
				break;
			}
			case CPU_Opcode::LEA:
			{
				uint16_t addr{ static_cast<uint16_t>(reinterpret_cast<long long>(_regmem->get_addr16()) - (static_cast<long long>(_data_seg * 0x10))) };
				*_reg = addr;
				break;
			}
			case CPU_Opcode::LES:
			{
				*_reg = *_regmem;
				_regmem->get_addr16()++;
				_cpu.get_sreg(Segment_Registers::ES) = _regmem->get_val16();
				break;
			}
			case CPU_Opcode::LODSB:
			{
				_cpu.get_reg8(Registers_8::AL) = _cpu._bus.read8(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI)--;
				}

				else
				{
					_cpu.get_reg16(Registers::SI)++;
				}

				break;
			}
			case CPU_Opcode::LODSW:
			{
				_cpu.get_reg16(Registers::AX) = _cpu._bus.read16(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI) -= 2;
				}

				else
				{
					_cpu.get_reg16(Registers::SI) += 2;
				}

				break;
			}
			case CPU_Opcode::LOOP_REL8:
			{
				_cpu.get_reg16(Registers::CX)--;

				if (_cpu.get_reg16(Registers::CX) != 0)
				{
					_cpu.get_reg16(Registers::IP) += (int8_t)_data1.data8[0] + _length;
					increase_ip = false;
				}

				break;
			}
			case CPU_Opcode::LOOPE_REL8:
			{
				_cpu.get_reg16(Registers::CX)--;

				if (_cpu.get_reg16(Registers::CX) != 0 && _cpu.get_reg16(Registers::Flags) & flag_zero)
				{
					_cpu.get_reg16(Registers::IP) += (int8_t)_data1.data8[0] + _length;
					increase_ip = false;
				}

				break;
			}
			case CPU_Opcode::LOOPNE_REL8:
			{
				_cpu.get_reg16(Registers::CX)--;

				if (_cpu.get_reg16(Registers::CX) != 0 && (_cpu.get_reg16(Registers::Flags) & flag_zero) == 0)
				{
					_cpu.get_reg16(Registers::IP) += (int8_t)_data1.data8[0] + _length;
					increase_ip = false;
				}

				break;
			}
			case CPU_Opcode::MOV_RM8_IMM8: // C6 mm dd dd ii
			case CPU_Opcode::MOV_RM16_IMM16: // C7 mm dd dd ii ii
			{
				*_regmem = _data1;
				break;
			}
			case CPU_Opcode::MOV_RM8_R8: // 88 mm dd dd
			case CPU_Opcode::MOV_RM16_R16: // 89 mm dd dd
			case CPU_Opcode::MOV_R8_RM8: // 8A mm dd dd
			case CPU_Opcode::MOV_R16_RM16: // 8B mm dd dd
			case CPU_Opcode::MOV_RM16_SREG: // 8C mm dd dd
			case CPU_Opcode::MOV_SREG_RM16: // 8E mm dd dd
			{
				if (_regmem_to_reg)
				{
					*_reg = *_regmem;
				}

				else
				{
					*_regmem = *_reg;
				}
				break;
			}
			// B0 ii
			case CPU_Opcode::MOV_AL_IMM8:
			case CPU_Opcode::MOV_CL_IMM8:
			case CPU_Opcode::MOV_DL_IMM8:
			case CPU_Opcode::MOV_BL_IMM8:
			case CPU_Opcode::MOV_AH_IMM8:
			case CPU_Opcode::MOV_CH_IMM8:
			case CPU_Opcode::MOV_DH_IMM8:
			case CPU_Opcode::MOV_BH_IMM8:
			case CPU_Opcode::MOV_AX_IMM16: // B8+x ii ii
			case CPU_Opcode::MOV_CX_IMM16:
			case CPU_Opcode::MOV_DX_IMM16:
			case CPU_Opcode::MOV_BX_IMM16:
			case CPU_Opcode::MOV_SP_IMM16:
			case CPU_Opcode::MOV_BP_IMM16:
			case CPU_Opcode::MOV_SI_IMM16:
			case CPU_Opcode::MOV_DI_IMM16:
			{
				*_reg = _data1;
				break;
			}
			case CPU_Opcode::MOV_AL_MOFFS8: // A0 dd dd
			{
				_cpu.get_reg8(Registers_8::AL) = _cpu._bus.read8(to_linear(_data_seg, _data1.data16), false);
				break;
			}
			case CPU_Opcode::MOV_AX_MOFFS16: // A1 dd dd
			{
				_cpu.get_reg16(Registers::AX) = _cpu._bus.read16(to_linear(_data_seg, _data1.data16), false);
				break;
			}
			case CPU_Opcode::MOV_MOFFS8_AL: // A2 dd dd	
			{
				_cpu._bus.write8(to_linear(_data_seg, _data1.data16), _cpu.get_reg8(Registers_8::AL), false);
				break;
			}
			case CPU_Opcode::MOV_MOFFS16_AX: // A2 dd dd
			{
				_cpu._bus.write16(to_linear(_data_seg, _data1.data16), _cpu.get_reg16(Registers::AX), false);
				break;
			}
			case CPU_Opcode::MOVSB:
			{
				_cpu._bus.write8(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), _cpu._bus.read8(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)), false), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI)--;
					_cpu.get_reg16(Registers::DI)--;
				}

				else
				{
					_cpu.get_reg16(Registers::SI)++;
					_cpu.get_reg16(Registers::DI)++;
				}

				break;
			}
			case CPU_Opcode::MOVSW:
			{
				_cpu._bus.write16(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), _cpu._bus.read16(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)), false), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI) -= 2;
					_cpu.get_reg16(Registers::DI) -= 2;
				}

				else
				{
					_cpu.get_reg16(Registers::SI) += 2;
					_cpu.get_reg16(Registers::DI) += 2;
				}

				break;
			}
			case CPU_Opcode::OR_AX_IMM16:
			case CPU_Opcode::OR_AL_IMM8:
			{
				*_reg |= _data1;
				break;
			}
			case CPU_Opcode::OR_RM8_R8:
			case CPU_Opcode::OR_RM16_R16:
			case CPU_Opcode::OR_R8_RM8:
			case CPU_Opcode::OR_R16_RM16:
			{
				if (_regmem_to_reg)
				{
					*_reg |= *_regmem;
				}

				else
				{
					*_regmem |= *_reg;
				}
				break;
			}
			case CPU_Opcode::OUT_IMM8_AL:
			{
				_cpu._bus.write8(_data1.data8[0], _reg->get_val8(), true);
				break;
			}
			case CPU_Opcode::OUT_IMM8_AX:
			{
				_cpu._bus.write16(_data1.data8[0], _reg->get_val16(), true);
				break;
			}
			case CPU_Opcode::OUT_DX_AL:
			{
				_cpu._bus.write8(_cpu.get_reg16(Registers::DX), _reg->get_val8(), true);
				break;
			}
			case CPU_Opcode::OUT_DX_AX:
			{
				_cpu._bus.write16(_cpu.get_reg16(Registers::DX), _reg->get_val16(), true);
				break;
			}
			case CPU_Opcode::OUTSB:
			{
				_cpu.get_reg8(Registers_8::AL) = _cpu._bus.read8(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)), true);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI)--;
				}

				else
				{
					_cpu.get_reg16(Registers::SI)++;
				}

				_cpu._bus.write8(_cpu.get_reg16(Registers::DX), _cpu.get_reg8(Registers_8::AL), true);
				break;
			}
			case CPU_Opcode::OUTSW:
			{
				_cpu.get_reg16(Registers::AX) = _cpu._bus.read16(to_linear(_data_seg, _cpu.get_reg16(Registers::SI)), true);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::SI) -= 2;
				}

				else
				{
					_cpu.get_reg16(Registers::SI) += 2;
				}

				_cpu._bus.write16(_cpu.get_reg16(Registers::DX), _cpu.get_reg16(Registers::AX), true);
				break;
			}
			case CPU_Opcode::PUSHA:
			{
				for (int i{}; i < 8; i++)
				{
					_cpu.push(_cpu.get_reg16(static_cast<Registers>(i)));
				}

				break;
			}
			case CPU_Opcode::PUSH_AX: // 50 + i
			case CPU_Opcode::PUSH_CX:
			case CPU_Opcode::PUSH_DX:
			case CPU_Opcode::PUSH_BX:
			case CPU_Opcode::PUSH_SP:
			case CPU_Opcode::PUSH_BP:
			case CPU_Opcode::PUSH_SI:
			case CPU_Opcode::PUSH_DI:
			case CPU_Opcode::PUSH_ES: // es
			case CPU_Opcode::PUSH_CS: // cs
			case CPU_Opcode::PUSH_SS: // ss
			case CPU_Opcode::PUSH_DS: // ds
			{
				_cpu.push(_reg->get_val16());
				break;
			}
			case CPU_Opcode::PUSHF:
			{
				_cpu.push(_cpu.get_reg16(Registers::Flags));
				break;
			}
			case CPU_Opcode::PUSH_IMM8:
			{
				_cpu.push(_data1.data8[0]);
				break;
			}
			case CPU_Opcode::PUSH_IMM16:
			{
				_cpu.push(_data1.data16);
				break;
			}
			case CPU_Opcode::POPA:
			{
				for (int i{ 7 }; i >= 0; i--)
				{
					if (i == 4) // popping sp (into nothing)
					{
						_cpu.pop();
					}

					else
					{
						_cpu.get_reg16(static_cast<Registers>(i)) = _cpu.pop();
					}
				}
				break;
			}
			case CPU_Opcode::POP_RM16: // 8F mm dd dd
			{
				*_regmem = _cpu.pop();
				break;
			}
			case CPU_Opcode::POP_AX: // 58 + i
			case CPU_Opcode::POP_CX:
			case CPU_Opcode::POP_DX:
			case CPU_Opcode::POP_BX:
			case CPU_Opcode::POP_SP:
			case CPU_Opcode::POP_BP:
			case CPU_Opcode::POP_SI:
			case CPU_Opcode::POP_DI:
			case CPU_Opcode::POP_ES: // es
			case CPU_Opcode::POP_CS: // cs
			case CPU_Opcode::POP_SS: // ss
			case CPU_Opcode::POP_DS: // ds
			{
				*_reg = _cpu.pop();
				break;
			}
			case CPU_Opcode::POPF:
			{
				_cpu.get_reg16(Registers::Flags) = _cpu.pop();
				break;
			}
			case CPU_Opcode::RET_FAR:
			{
				_cpu.get_reg16(Registers::IP) = _cpu.pop();
				_cpu.get_sreg(Segment_Registers::CS) = _cpu.pop();
				increase_ip = false;
				break;
			}
			case CPU_Opcode::RET_FAR_IMM16:
			{
				_cpu.get_reg16(Registers::IP) = _cpu.pop();
				_cpu.get_sreg(Segment_Registers::CS) = _cpu.pop();
				_cpu.get_reg16(Registers::SP) += _data1.data16;
				increase_ip = false;
				break;
			}
			case CPU_Opcode::RET_NEAR:
			{
				_cpu.get_reg16(Registers::IP) = _cpu.pop();
				increase_ip = false;
				break;
			}
			case CPU_Opcode::RET_NEAR_IMM16:
			{
				_cpu.get_reg16(Registers::IP) = _cpu.pop();
				_cpu.get_reg16(Registers::SP) += _data1.data16;
				increase_ip = false;
				break;
			}
			case CPU_Opcode::SAHF:
			{
				_cpu.get_reg16(Registers::Flags) &= ~0x00ff;
				_cpu.get_reg16(Registers::Flags) |= _cpu.get_reg8(Registers_8::AH);
				break;
			}
			case CPU_Opcode::SBB_AL_IMM8:
			{
				uint8_t temp{ static_cast<uint8_t>(_data1.data8[0] + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
				*_reg -= temp;
				break;
			}
			case CPU_Opcode::SBB_AX_IMM16:
			{
				uint16_t temp{ static_cast<uint16_t>(_data1.data16 + (_cpu.get_reg16(Registers::Flags) & flag_carry)) };
				*_reg -= temp;
				break;
			}
			case CPU_Opcode::SBB_RM8_R8:
			case CPU_Opcode::SBB_RM16_R16:
			case CPU_Opcode::SBB_R8_RM8:
			case CPU_Opcode::SBB_R16_RM16:
			{
				if (_regmem_to_reg)
				{
					if (_width)
					{
						uint16_t temp{ static_cast<uint16_t>(_regmem->get_val16() + (_cpu.get_reg16(Registers::Flags) & flag_carry))};
						*_reg -= temp;
					}

					else
					{
						uint8_t temp{ static_cast<uint8_t>(_regmem->get_val8() + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
						*_reg -= temp;
					}
				}

				else
				{
					if (_width)
					{
						uint16_t temp{ static_cast<uint16_t>(_reg->get_val16() + (_cpu.get_reg16(Registers::Flags) & flag_carry)) };
						*_regmem -= temp;
					}

					else
					{
						uint8_t temp{ static_cast<uint8_t>(_reg->get_val8() + (static_cast<uint8_t>(_cpu.get_reg16(Registers::Flags) & flag_carry))) };
						*_regmem -= temp;
					}
				}
				break;
			}
			case CPU_Opcode::SCASB:
			{
				uint8_t temp_var{ _cpu.get_reg8(Registers_8::AL) };
				Instruction::Operand temp{ _cpu, _width };
				temp.get_addr8() = &_cpu.get_reg8(Registers_8::AL);
				temp -= _cpu._bus.read8(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), false);
				temp = temp_var;

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::DI)--;
				}

				else
				{
					_cpu.get_reg16(Registers::DI)++;
				}

				_cpu._zero_flag_check = true;

				break;
			}
			case CPU_Opcode::SCASW:
			{
				uint16_t temp_var{ _cpu.get_reg16(Registers::AX) };
				Instruction::Operand temp{ _cpu, _width };
				temp.get_addr16() = &_cpu.get_reg16(Registers::AX);
				temp -= _cpu._bus.read16(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), false);
				temp = temp_var;

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::DI) -= 2;
				}

				else
				{
					_cpu.get_reg16(Registers::DI) += 2;
				}

				_cpu._zero_flag_check = true;

				break;
			}
			case CPU_Opcode::STC:
			{
				_cpu.get_reg16(Registers::Flags) |= flag_carry;
				break;
			}
			case CPU_Opcode::STD:
			{
				_cpu.get_reg16(Registers::Flags) |= flag_direction;
				break;
			}
			case CPU_Opcode::STI:
			{
				_cpu._sti_enable = 1;
				break;
			}
			case CPU_Opcode::STOSB:
			{
				_cpu._bus.write8(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), _cpu.get_reg8(Registers_8::AL), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::DI)--;
				}

				else
				{
					_cpu.get_reg16(Registers::DI)++;
				}

				break;
			}
			case CPU_Opcode::STOSW:
			{
				_cpu._bus.write16(to_linear(_cpu.get_sreg(Segment_Registers::ES), _cpu.get_reg16(Registers::DI)), _cpu.get_reg16(Registers::AX), false);

				if (_cpu.get_reg16(Registers::Flags) & flag_direction)
				{
					_cpu.get_reg16(Registers::DI) -= 2;
				}

				else
				{
					_cpu.get_reg16(Registers::DI) += 2;
				}

				break;
			}
			case CPU_Opcode::SUB_AL_IMM8:
			case CPU_Opcode::SUB_AX_IMM16:
			{
				*_reg -= _data1;
				break;
			}
			case CPU_Opcode::SUB_RM8_R8:
			case CPU_Opcode::SUB_RM16_R16:
			case CPU_Opcode::SUB_R8_RM8:
			case CPU_Opcode::SUB_R16_RM16:
			{
				if (_regmem_to_reg)
				{
					*_reg -= *_regmem;
				}

				else
				{
					*_regmem -= *_reg;
				}
				break;
			}
			case CPU_Opcode::TEST_AL_IMM8:
			{
				uint8_t temp_var{ _reg->get_val8() };
				*_reg &= _data1;
				*_reg = temp_var;
				break;
			}
			case CPU_Opcode::TEST_AX_IMM16:
			{
				uint16_t temp_var{ _reg->get_val16() };
				*_reg &= _data1;
				*_reg = temp_var;
				break;
			}
			case CPU_Opcode::TEST_RM8_R8:
			{
				uint8_t temp_var{ _regmem->get_val8() };
				*_regmem &= *_reg;
				*_regmem = temp_var;
				break;
			}
			case CPU_Opcode::TEST_RM16_R16:
			{
				uint16_t temp_var{ _regmem->get_val16() };
				*_regmem &= *_reg;
				*_regmem = temp_var;
				break;
			}
			case CPU_Opcode::XCHG_RM8_R8:
			{
				uint8_t temp{ _regmem->get_val8() };
				*_regmem = *_reg;
				*_reg = temp;
				break;
			}
			case CPU_Opcode::XCHG_RM16_R16:
			{
				uint16_t temp{ _regmem->get_val16() };
				*_regmem = *_reg;
				*_reg = temp;
				break;
			}
			case CPU_Opcode::XCHG_AX_AX:
			case CPU_Opcode::XCHG_AX_CX:
			case CPU_Opcode::XCHG_AX_DX:
			case CPU_Opcode::XCHG_AX_BX:
			case CPU_Opcode::XCHG_AX_SP:
			case CPU_Opcode::XCHG_AX_BP:
			case CPU_Opcode::XCHG_AX_SI:
			case CPU_Opcode::XCHG_AX_DI:
			{
				uint16_t temp{ _reg->get_val16() };
				*_reg = _cpu.get_reg16(Registers::AX);
				_cpu.get_reg16(Registers::AX) = temp;
				break;
			}
			case CPU_Opcode::XLAT:
			{
				uint16_t temp{ static_cast<uint16_t>(_cpu.get_reg8(Registers_8::AL)) };
				_cpu.get_reg8(Registers_8::AL) = _cpu._bus.read8(to_linear(_data_seg, _cpu.get_reg16(Registers::BX) + temp), false);
				break;
			}
			case CPU_Opcode::XOR_AL_IMM8:
			case CPU_Opcode::XOR_AX_IMM16:
			{
				*_reg ^= _data1;
				break;
			}
			case CPU_Opcode::XOR_RM8_R8:
			case CPU_Opcode::XOR_RM16_R16:
			case CPU_Opcode::XOR_R8_RM8:
			case CPU_Opcode::XOR_R16_RM16:
			{
				if (_regmem_to_reg)
				{
					*_reg ^= *_regmem;
				}

				else
				{
					*_regmem ^= *_reg;
				}
				break;
			}
			default:
				printf("[CPU] Unknown Opcode %x, %x:%x\n", _operation, _segment, _offset);
				break;
			}
		}

		if (increase_ip)
		{
			_cpu.get_reg16(Registers::IP) += _length;
		}

		if (dec_cx)
		{
			_cpu.get_reg16(Registers::CX)--;
		}
	}


	_cpu.check_irq();
}