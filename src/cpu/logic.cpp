#include "cpu.h"
#include <print>
#include "to_linear.h"
#include <utility>

void CPU::logic() {
	ticks_total++;

	bool exec{ true };
	bool dec_cx{};
	bool reset_ip{};

	if (_sti_enable)
	{
		get_reg(Registers::Flags) |= flag_interrupt;
		_sti_enable = false;
	}

	if (!_halted) {
		// TODO: Fetch prefixes here
		bool default_seg{true};
		CPU_Opcode rep{};
		bool prefix_instruction_done{};
		uint16_t opcode_ip{get_reg(Registers::IP)};
		uint16_t data_seg{get_sreg(Segment_Registers::DS)};

		for(int i{}; !prefix_instruction_done; i++)
		{
			CPU_Opcode cur_byte{ static_cast<CPU_Opcode>(_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), opcode_ip + i), false)) };

			switch (cur_byte)
			{
			case CPU_Opcode::PREFIX_ES:
			case CPU_Opcode::PREFIX_CS:
			case CPU_Opcode::PREFIX_SS:
			case CPU_Opcode::PREFIX_DS:
			{
				data_seg = get_sreg(static_cast<Segment_Registers>((std::to_underlying(cur_byte) >> 3) & 0x3));
				default_seg = false;
				get_reg(Registers::IP)++;
				break;
			}
			case CPU_Opcode::PREFIX_REP_OR_REPE:
			case CPU_Opcode::PREFIX_REPNE:
			{
				rep = cur_byte; // repetition
				get_reg(Registers::IP)++;
				break;
			}
			case static_cast<CPU_Opcode>(0xf0):
			{
				get_reg(Registers::IP)++;
				break;
			}
			default:
			{
				prefix_instruction_done = true;
			}
			}
		}

		switch (rep)
		{
		case CPU_Opcode::PREFIX_REP_OR_REPE:
		{
			if (_zero_flag_check) // this is for instructions that check if the zero flag is on, but it uses the same rep prefix byte.
			{
				if ((get_reg(Registers::Flags) & flag_zero) && get_reg(Registers::CX) != 0)
				{
					reset_ip = true;
					dec_cx = true;
				}

				else
				{
					exec = false;
				}

				_zero_flag_check = false;
			}

			else
			{
				if (get_reg(Registers::CX) != 0)
				{
					reset_ip = true;
					dec_cx = true;
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
			if ((get_reg(Registers::Flags) & flag_zero) == 0 && get_reg(Registers::CX) != 0)
			{
				reset_ip = true;
				dec_cx = true;
			}

			else
			{
				exec = false;
			}

			break;
		}
		}

		CPU_Opcode opcode{ static_cast<CPU_Opcode>(_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false)) };

		get_reg(Registers::IP)++;

		switch (opcode) // actual opcode
		{
		case static_cast<CPU_Opcode>(0x82):
		case CPU_Opcode::GROUP_OPCODE_80:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint8_t>(default_seg, data_seg).first;
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_8X>(group_opcode))
				{
					case CPU_Group_Opcode_8X::ADD:
					{
						regmem += imm;
						break;
					}
					case CPU_Group_Opcode_8X::ADC:
					{
						regmem += (static_cast<uint8_t>(imm.get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
						break;
					}
					case CPU_Group_Opcode_8X::SBB:
					{
						regmem -= (static_cast<uint8_t>(imm.get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
						break;
					}
					case CPU_Group_Opcode_8X::AND:
					{
						regmem &= imm;
						break;
					}
					case CPU_Group_Opcode_8X::CMP: // 80 mm ii
					{
						uint8_t temp_var{ regmem.get_val() };
						regmem -= imm;
						regmem = (temp_var);
						break;
					}
					case CPU_Group_Opcode_8X::SUB:
					{
						regmem -= imm;
						break;
					}
					case CPU_Group_Opcode_8X::OR:
					{
						regmem |= imm;
						break;
					}
					case CPU_Group_Opcode_8X::XOR:
					{
						regmem ^= imm;
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}

			break;
		}
		case CPU_Opcode::GROUP_OPCODE_81:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg);
			if (exec) {
				switch (static_cast<CPU_Group_Opcode_8X>(group_opcode))
				{
					case CPU_Group_Opcode_8X::ADD:
					{
						regmem += imm;
						break;
					}
					case CPU_Group_Opcode_8X::ADC:
					{
						regmem += (static_cast<uint16_t>(imm.get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));
						break;
					}
					case CPU_Group_Opcode_8X::SBB:
					{
						regmem -= (static_cast<uint16_t>(imm.get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));
						break;
					}
					case CPU_Group_Opcode_8X::AND:
					{
						regmem &= imm;
						break;
					}
					case CPU_Group_Opcode_8X::CMP: // 83 mm ii
					{
						uint16_t temp_var{ regmem.get_val() };
						regmem -= imm;
						regmem = (temp_var);
						break;
					}
					case CPU_Group_Opcode_8X::SUB:
					{
						regmem -= imm;
						break;
					}
					case CPU_Group_Opcode_8X::OR:
					{
						regmem |= imm;
						break;
					}
					case CPU_Group_Opcode_8X::XOR:
					{
						regmem ^= imm;
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}
			break;
		}
		case CPU_Opcode::GROUP_OPCODE_83:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);

			if (exec) {
				switch (static_cast<CPU_Group_Opcode_8X>(group_opcode))
				{
					case CPU_Group_Opcode_8X::ADD:
					{
						regmem += static_cast<uint16_t>(imm.get_val());
						break;
					}
					case CPU_Group_Opcode_8X::ADC:
					{
						regmem += (static_cast<uint8_t>(imm.get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
						break;
					}
					case CPU_Group_Opcode_8X::SBB:
					{
						regmem -= (static_cast<uint8_t>(imm.get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
						break;
					}
					case CPU_Group_Opcode_8X::AND:
					{
						regmem &= static_cast<uint16_t>(imm.get_val());
						break;
					}
					case CPU_Group_Opcode_8X::CMP: // 83 mm ii
					{
						uint16_t temp_var{ regmem.get_val() };
						regmem -= static_cast<uint16_t>(imm.get_val());
						regmem = (temp_var);
						break;
					}
					case CPU_Group_Opcode_8X::SUB:
					{
						regmem -= static_cast<uint16_t>(imm.get_val());
						break;
					}
					case CPU_Group_Opcode_8X::OR:
					{
						regmem |= static_cast<uint16_t>(imm.get_val());
						break;
					}
					case CPU_Group_Opcode_8X::XOR:
					{
						regmem ^= static_cast<uint16_t>(imm.get_val());
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}
			break;
		}
		case CPU_Opcode::GROUP_OPCODE_C0:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint8_t>(default_seg, data_seg).first;
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_CX_DX>(group_opcode))
				{
					case CPU_Group_Opcode_CX_DX::ROL:
					{
						regmem.rol(imm);
						break;
					}
					case CPU_Group_Opcode_CX_DX::ROR:
					{
						regmem.ror(imm);
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCL:
					{
						regmem.rcl(imm);
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCR:
					{
						regmem.rcr(imm);
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAL: // D0 mm
					{
						regmem <<= imm;
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAR: // D0 mm
					{
						regmem.sar(imm);
						break;
					}
					case CPU_Group_Opcode_CX_DX::SHR: // D0 mm
					{
						regmem >>= imm;
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}

			break;
		}
		case CPU_Opcode::GROUP_OPCODE_C1:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);
			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_CX_DX>(group_opcode))
				{
					case CPU_Group_Opcode_CX_DX::ROL:
					{
						regmem.rol(static_cast<uint16_t>(imm.get_val()));
						break;
					}
					case CPU_Group_Opcode_CX_DX::ROR:
					{
						regmem.ror(static_cast<uint16_t>(imm.get_val()));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCL:
					{
						regmem.rcl(static_cast<uint16_t>(imm.get_val()));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCR:
					{
						regmem.rcr(static_cast<uint16_t>(imm.get_val()));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAL: // D0 mm
					{
						regmem <<= static_cast<uint16_t>(imm.get_val());
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAR: // D0 mm
					{
						regmem.sar(static_cast<uint16_t>(imm.get_val()));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SHR: // D0 mm
					{
						regmem >>= static_cast<uint16_t>(imm.get_val());
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}

			break;
		}
		case CPU_Opcode::GROUP_OPCODE_D0:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint8_t>(default_seg, data_seg).first;

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_CX_DX>(group_opcode))
				{
					case CPU_Group_Opcode_CX_DX::ROL:
					{
						regmem.rol(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::ROR:
					{
						regmem.ror(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCL:
					{
						regmem.rcl(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCR:
					{
						regmem.rcr(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAL: // D0 mm
					{
						regmem <<= static_cast<uint8_t>(1);
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAR: // D0 mm
					{
						regmem.sar(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SHR: // D0 mm
					{
						regmem >>= static_cast<uint8_t>(1);
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}

			break;
		}
		case CPU_Opcode::GROUP_OPCODE_D1:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;
			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_CX_DX>(group_opcode))
				{
					case CPU_Group_Opcode_CX_DX::ROL:
					{
						regmem.rol(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::ROR:
					{
						regmem.ror(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCL:
					{
						regmem.rcl(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCR:
					{
						regmem.rcr(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAL: // D0 mm
					{
						regmem <<= static_cast<uint8_t>(1);
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAR: // D0 mm
					{
						regmem.sar(static_cast<uint8_t>(1));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SHR: // D0 mm
					{
						regmem >>= static_cast<uint8_t>(1);
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}

			break;
		}
		case CPU_Opcode::GROUP_OPCODE_D2:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint8_t>(default_seg, data_seg).first;
			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_CX_DX>(group_opcode))
				{
					case CPU_Group_Opcode_CX_DX::ROL:
					{
						regmem.rol(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::ROR:
					{
						regmem.ror(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCL:
					{
						regmem.rcl(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCR:
					{
						regmem.rcr(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAL: // D0 mm
					{
						regmem <<= get_reg(Registers_8::CL);
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAR: // D0 mm
					{
						regmem.sar(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SHR: // D0 mm
					{
						regmem >>= get_reg(Registers_8::CL);
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);
						break;
					}
				}
			}
			break;
		}
		case CPU_Opcode::GROUP_OPCODE_D3:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_CX_DX>(group_opcode))
				{
					case CPU_Group_Opcode_CX_DX::ROL:
					{
						regmem.rol(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::ROR:
					{
						regmem.ror(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCL:
					{
						regmem.rcl(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::RCR:
					{
						regmem.rcr(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAL: // D0 mm
					{
						regmem <<= get_reg(Registers_8::CL);
						break;
					}
					case CPU_Group_Opcode_CX_DX::SAR: // D0 mm
					{
						regmem.sar(get_reg(Registers_8::CL));
						break;
					}
					case CPU_Group_Opcode_CX_DX::SHR: // D0 mm
					{
						regmem >>= get_reg(Registers_8::CL);
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}

			break;
		}
		case CPU_Opcode::GROUP_OPCODE_F6:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint8_t>(default_seg, data_seg).first;

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_F6_F7>(group_opcode))
				{
					case CPU_Group_Opcode_F6_F7::TEST:
					{
						auto imm = fetch<Op_Imm<uint8_t>>(data_seg);
						uint8_t temp_var{ regmem.get_val() };
						regmem &= imm;
						regmem = (temp_var);
						break;
					}
					case CPU_Group_Opcode_F6_F7::MUL: // F6 mm
					{
						regmem.mul();
						break;
					}
					case CPU_Group_Opcode_F6_F7::IMUL:
					{
						regmem.imul();
						break;
					}
					case CPU_Group_Opcode_F6_F7::NEG:
					{
						regmem.neg();
						break;
					}
					case CPU_Group_Opcode_F6_F7::NOT:
					{
						regmem = (static_cast<uint8_t>(~regmem.get_val()));
						break;
					}
					case CPU_Group_Opcode_F6_F7::DIV:
					{
						regmem.div();
						break;
					}
					case CPU_Group_Opcode_F6_F7::IDIV:
					{
						regmem.idiv();
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}
			break;
		}
		case CPU_Opcode::GROUP_OPCODE_F7:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_F6_F7>(group_opcode))
				{
					case CPU_Group_Opcode_F6_F7::TEST:
					{
						auto imm = fetch<Op_Imm<uint16_t>>(data_seg);
						uint16_t temp_var{ regmem.get_val() };
						regmem &= imm;
						regmem = (temp_var);
						break;
					}
					case CPU_Group_Opcode_F6_F7::MUL: // F7 mm
					{
						regmem.mul();
						break;
					}
					case CPU_Group_Opcode_F6_F7::IMUL:
					{
						regmem.imul();
						break;
					}
					case CPU_Group_Opcode_F6_F7::NEG:
					{
						regmem.neg();
						break;
					}
					case CPU_Group_Opcode_F6_F7::NOT:
					{
						regmem = (static_cast<uint16_t>(~regmem.get_val()));
						break;
					}
					case CPU_Group_Opcode_F6_F7::DIV:
					{
						regmem.div();
						break;
					}
					case CPU_Group_Opcode_F6_F7::IDIV:
					{
						regmem.idiv();
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}
			break;
		}
		case CPU_Opcode::GROUP_OPCODE_FE:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint8_t>(default_seg, data_seg).first;

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_FE>(group_opcode))
				{
					case CPU_Group_Opcode_FE::DEC_RM8: // FE mm dd dd
					{
						regmem--;
						break;
					}
					case CPU_Group_Opcode_FE::INC_RM8: // FE mm dd dd
					{
						regmem++;
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);
						break;
					}
				}
			}
			break;
		}
		case CPU_Opcode::GROUP_OPCODE_FF:
		{
			uint8_t group_opcode{ static_cast<uint8_t>((_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) >> 3) & 0x7) };
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;

			if (exec)
			{
				switch (static_cast<CPU_Group_Opcode_FF>(group_opcode))
				{
					case CPU_Group_Opcode_FF::DEC_RM16: // FF mm dd dd
					{
						regmem--;
						break;
					}
					case CPU_Group_Opcode_FF::CALL_RM16:
					{
						push(get_reg(Registers::IP));
						get_reg(Registers::IP) = regmem.get_val();
						break;
					}
					case CPU_Group_Opcode_FF::CALL_M16_16: // FF mm
					{
						uint16_t offset{ regmem.get_val() };
						uint16_t segment{ Op_Mem<uint16_t>(*this, std::get<int>(regmem.get_op()) + 2).get_val() };

						push(get_sreg(Segment_Registers::CS));
						push(get_reg(Registers::IP));

						get_reg(Registers::IP) = offset;
						get_sreg(Segment_Registers::CS) = segment;
						break;
					}
					case CPU_Group_Opcode_FF::JMP_RM16: // FF mm dd dd
					{
						get_reg(Registers::IP) = regmem.get_val();
						break;
					}
					case CPU_Group_Opcode_FF::JMP_M16_16:
					{
						uint16_t offset{ regmem.get_val() };
						uint16_t segment{ Op_Mem<uint16_t>(*this, std::get<int>(regmem.get_op()) + 2).get_val() };

						get_reg(Registers::IP) = offset;
						get_sreg(Segment_Registers::CS) = segment;
						break;
					}
					case CPU_Group_Opcode_FF::PUSH_RM16: // FF mm dd dd
					{
						push(regmem.get_val());
						break;
					}
					case CPU_Group_Opcode_FF::INC_RM16: // FF mm dd dd
					{
						regmem++;
						break;
					}
					default:
					{
						std::println("[CPU] Unkown Opcode {:x} /{}", static_cast<uint8_t>(opcode), group_opcode);

						break;
					}
				}
			}

			break;
		}
		case CPU_Opcode::AAA:
		{
			if (((get_reg(Registers_8::AL) & 0xf) > 9) || get_reg(Registers::Flags) & flag_half_carry)
			{
				get_reg(Registers::AX) += 0x106;
				get_reg(Registers::Flags) |= flag_half_carry | flag_carry;
			}

			else
			{
				get_reg(Registers::Flags) &= ~flag_half_carry;
				get_reg(Registers::Flags) &= ~flag_carry;
			}

			get_reg(Registers_8::AL) &= 0x0f;

			break;
		}
		case CPU_Opcode::AAD:
		{
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg).get_val();

			if (exec)
			{
				uint8_t tempal{ get_reg(Registers_8::AL) };
				uint8_t tempah{ get_reg(Registers_8::AH) };

				get_reg(Registers_8::AL) = (tempal + (tempah * imm)) & 0xff;
				get_reg(Registers_8::AH) = 0;

				modify_flag_sign(get_reg(Registers_8::AL));
				modify_flag_zero(get_reg(Registers_8::AL));
				modify_flag_parity(get_reg(Registers_8::AL));
			}
			break;
		}
		case CPU_Opcode::AAM:
		{
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg).get_val();

			if (exec)
			{
				uint8_t temp{ get_reg(Registers_8::AL) };

				get_reg(Registers_8::AH) = temp / imm;
				get_reg(Registers_8::AL) = temp % imm;

				modify_flag_sign(get_reg(Registers_8::AL));
				modify_flag_zero(get_reg(Registers_8::AL));
				modify_flag_parity(get_reg(Registers_8::AL));
			}
			break;
		}
		case CPU_Opcode::AAS:
		{
			if (exec)
			{
				if ((get_reg(Registers_8::AL) & 0xf) > 9 || get_reg(Registers::Flags) & flag_half_carry)
				{
					get_reg(Registers::AX) -= 6;
					get_reg(Registers_8::AH)--;
					get_reg(Registers::Flags) |= flag_half_carry;
					get_reg(Registers::Flags) |= flag_carry;
					get_reg(Registers_8::AL) &= 0xf;
				}

				else
				{
					get_reg(Registers::Flags) &= ~flag_half_carry;
					get_reg(Registers::Flags) &= ~flag_carry;
					get_reg(Registers_8::AL) &= 0xf;
				}
			}
			break;
		}
		case CPU_Opcode::ADC_AL_IMM8:
		{
			auto imm = static_cast<uint8_t>((fetch<Op_Imm<uint8_t>>(data_seg).get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
			if (exec) Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL)) += imm;
			break;
		}
		case CPU_Opcode::ADC_AX_IMM16:
		{
			auto imm = static_cast<uint16_t>((fetch<Op_Imm<uint16_t>>(data_seg).get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));
			if (exec) Op_Reg<uint16_t>(*this, &get_reg(Registers::AX)) += imm;
			break;
		}
		case CPU_Opcode::ADC_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) += (static_cast<uint8_t>(std::get<Op_Reg<uint8_t>>(modrm).get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::ADC_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) += (static_cast<uint16_t>(std::get<Op_Reg<uint16_t>>(modrm).get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::ADC_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) += (static_cast<uint8_t>(std::get<Op_Regmem<uint8_t>>(modrm).get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::ADC_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) += (static_cast<uint16_t>(std::get<Op_Regmem<uint16_t>>(modrm).get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::ADD_AL_IMM8:
		{
			auto imm = static_cast<uint8_t>((fetch<Op_Imm<uint8_t>>(data_seg).get_val()));
			if (exec) Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL)) += imm;
			break;
		}
		case CPU_Opcode::ADD_AX_IMM16:
		{
			auto imm = static_cast<uint16_t>((fetch<Op_Imm<uint16_t>>(data_seg).get_val()));
			if (exec) Op_Reg<uint16_t>(*this, &get_reg(Registers::AX)) += imm;
			break;
		}
		case CPU_Opcode::ADD_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) += std::get<Op_Reg<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::ADD_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) += std::get<Op_Reg<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::ADD_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) += std::get<Op_Regmem<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::ADD_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) += std::get<Op_Regmem<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::AND_AL_IMM8:
		{
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);
			if (exec) Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL)) &= imm;
			break;
		}
		case CPU_Opcode::AND_AX_IMM16:
		{
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg);
			if (exec) Op_Reg<uint16_t>(*this, &get_reg(Registers::AX)) &= imm;
			break;
		}
		case CPU_Opcode::AND_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) &= std::get<Op_Reg<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::AND_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) &= std::get<Op_Reg<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::AND_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) &= std::get<Op_Regmem<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::AND_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) &= std::get<Op_Regmem<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::CALL_PTR16_16: // 9A ii ii ii ii
		{
			uint16_t new_ip{ fetch<Op_Imm<uint16_t>>(data_seg).get_val() };
			uint16_t new_cs{ fetch<Op_Imm<uint16_t>>(data_seg).get_val() };

			if (exec)
			{
				push(get_sreg(Segment_Registers::CS));
				push(get_reg(Registers::IP));

				get_sreg(Segment_Registers::CS) = new_cs;
				get_reg(Registers::IP) = new_ip;
			}
			break;
		}
		case CPU_Opcode::CALL_REL16: // E8 ii ii
		{
			int16_t call_value{ static_cast<int16_t>(fetch<Op_Imm<uint16_t>>(data_seg).get_val()) };

			if (exec)
			{
				push(get_reg(Registers::IP));
				get_reg(Registers::IP) += call_value;
			}
			break;
		}
		case CPU_Opcode::CBW:
		{
			if (exec)
			{
				if (static_cast<int8_t>(get_reg(Registers_8::AL)) < 0)
				{
					get_reg(Registers_8::AH) = 0xFF;
				}

				else
				{
					get_reg(Registers_8::AH) = 0;
				}
			}
			break;
		}
		case CPU_Opcode::CLC:
		{
			if (exec) get_reg(Registers::Flags) &= ~flag_carry;
			break;
		}
		case CPU_Opcode::CLD:
		{
			if (exec) get_reg(Registers::Flags) &= ~flag_direction;
			break;
		}
		case CPU_Opcode::CLI:
		{
			if (exec) get_reg(Registers::Flags) &= ~flag_interrupt;
			break;
		}
		case CPU_Opcode::CMC:
		{
			if (exec)
			{
				if (get_reg(Registers::Flags) & flag_carry)
				{
					get_reg(Registers::Flags) &= ~flag_carry;
				}

				else
				{
					get_reg(Registers::Flags) |= flag_carry;
				}
			}
			break;
		}
		case CPU_Opcode::CMP_AL_IMM8: // 3C ii
		{
			Op_Reg<uint8_t> reg{*this, &get_reg(Registers_8::AL)};
			uint8_t temp_var {reg.get_val()};
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);
			if (exec) reg -= imm;
			reg = (temp_var);
			break;
		}
		case CPU_Opcode::CMP_AX_IMM16: // 3D ii ii
		{
			Op_Reg<uint16_t> reg{*this, &get_reg(Registers::AX)};
			uint16_t temp_var {reg.get_val()};
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg);
			if (exec) reg -= imm;
			reg = (temp_var);
			break;
		}
		case CPU_Opcode::CMP_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			uint8_t temp_var{ std::get<Op_Regmem<uint8_t>>(modrm).get_val() };
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) -= std::get<Op_Reg<uint8_t>>(modrm);
			std::get<Op_Regmem<uint8_t>>(modrm) = (temp_var);
			break;
		}
		case CPU_Opcode::CMP_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			uint16_t temp_var{ std::get<Op_Regmem<uint16_t>>(modrm).get_val() };
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) -= std::get<Op_Reg<uint16_t>>(modrm);
			std::get<Op_Regmem<uint16_t>>(modrm) = (temp_var);
			break;
		}
		case CPU_Opcode::CMP_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			uint8_t temp_var{ std::get<Op_Reg<uint8_t>>(modrm).get_val()};
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) -= std::get<Op_Regmem<uint8_t>>(modrm);
			std::get<Op_Reg<uint8_t>>(modrm) = (temp_var);
			break;
		}
		case CPU_Opcode::CMP_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			uint16_t temp_var{ std::get<Op_Reg<uint16_t>>(modrm).get_val() };
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) -= std::get<Op_Regmem<uint16_t>>(modrm);
			std::get<Op_Reg<uint16_t>>(modrm) = (temp_var);
			break;
		}
		case CPU_Opcode::CMPSB:
		{
			if (exec)
			{
				Op_Mem<uint8_t> temp{*this, to_linear(data_seg, get_reg(Registers::SI))};
				uint8_t temp_var{ temp.get_val() };
				temp -= _bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), false);
				temp = (temp_var);
		
				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI)--;
					get_reg(Registers::DI)--;
				}
		
				else
				{
					get_reg(Registers::SI)++;
					get_reg(Registers::DI)++;
				}
		
				_zero_flag_check = true;
			}
			break;
		}
		case CPU_Opcode::CMPSW:
		{
			if (exec)
			{
				Op_Mem<uint16_t> temp{*this, to_linear(data_seg, get_reg(Registers::SI))};
				uint16_t temp_var{ temp.get_val() };
				temp -= _bus.read<uint16_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), false);
				temp = (temp_var);
		
				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI) -= 2;
					get_reg(Registers::DI) -= 2;
				}
		
				else
				{
					get_reg(Registers::SI) += 2;
					get_reg(Registers::DI) += 2;
				}
		
				_zero_flag_check = true;
			}
			break;
		}
		case CPU_Opcode::CWD:
		{
			if (exec)
			{
				if (static_cast<int16_t>(get_reg(Registers::AX)) < 0)
				{
					get_reg(Registers::DX) = 0xFFFF;
				}
		
				else
				{
					get_reg(Registers::DX) = 0;
				}
			}
			break;
		}
		case CPU_Opcode::DAA:
		{
			if (exec)
			{
				uint8_t old_al{ get_reg(Registers_8::AL) };
				uint16_t old_cf{ static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry) };
				get_reg(Registers::Flags) &= ~flag_carry;
		
				if(((get_reg(Registers_8::AL) & 0xf) > 9) || get_reg(Registers::Flags) & flag_half_carry)
				{
					Op_Reg<uint8_t> temp_al{*this, &get_reg(Registers_8::AL)};
					temp_al += static_cast<uint8_t>(6);
					get_reg(Registers::Flags) |= old_cf;
					get_reg(Registers::Flags) |= flag_half_carry;
				}
				else
				{
					get_reg(Registers::Flags) &= ~flag_half_carry;
				}
				if ((old_al > 0x99) || (old_cf == true))
				{
					get_reg(Registers_8::AL) += 0x60;
					get_reg(Registers::Flags) |= flag_carry;
				}
				else
				{
					get_reg(Registers::Flags) &= ~flag_carry;
				}
			}
			break;
		}
		case CPU_Opcode::DAS:
		{
			if (exec)
			{
				uint8_t old_al{ get_reg(Registers_8::AL) };
				bool old_cf{ static_cast<bool>(get_reg(Registers::Flags) & flag_carry) };
		
				get_reg(Registers::Flags) &= ~flag_carry;
		
				if ((get_reg(Registers_8::AL) & 0xf) > 9 || get_reg(Registers::Flags) & flag_half_carry)
				{
					get_reg(Registers_8::AL) -= 6;
		
					if (old_cf)
					{
						get_reg(Registers::Flags) |= flag_carry;
					}
		
					else
					{
						get_reg(Registers::Flags) &= ~flag_carry;
					}
		
					get_reg(Registers::Flags) |= flag_half_carry;
				}
		
				else
				{
					get_reg(Registers::Flags) &= ~flag_half_carry;
				}
		
				if ((old_al > 0x99) || old_cf)
				{
					get_reg(Registers_8::AL) -= 0x60;
					get_reg(Registers::Flags) |= flag_carry;
				}
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
			Op_Reg<uint16_t> reg{ *this, &get_reg(static_cast<Registers>(static_cast<uint8_t>(opcode) & 0x7)) };
			if (exec) reg--;
			break;
		}
		case CPU_Opcode::HLT:
		{
			_halted = true;
			break;
		}
 		case CPU_Opcode::IMUL_R16_RM16_IMM8:
		{
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg).get_val();
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);

			if(exec) std::get<Op_Reg<uint16_t>>(modrm).imul(std::get<Op_Regmem<uint16_t>>(modrm), imm);
			break;
		}
		case CPU_Opcode::IMUL_R16_RM16_IMM16:
		{
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg).get_val();
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);

			if (exec) std::get<Op_Reg<uint16_t>>(modrm).imul(std::get<Op_Regmem<uint16_t>>(modrm), imm);
			break;
		}
		case CPU_Opcode::IN_AL_IMM8:
		{
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg).get_val();
			if (exec) get_reg(Registers_8::AL) = _bus.read<uint8_t>(imm, true);
			break;
		}
		case CPU_Opcode::IN_AX_IMM8:
		{
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg).get_val();
			if (exec) get_reg(Registers::AX) = _bus.read<uint16_t>(imm, true);
			break;
		}
		case CPU_Opcode::IN_AL_DX:
		{
			if (exec) get_reg(Registers_8::AL) = _bus.read<uint8_t>(get_reg(Registers::DX), true);
			break;
		}
		case CPU_Opcode::IN_AX_DX:
		{
			if (exec) get_reg(Registers::AX) = _bus.read<uint16_t>(get_reg(Registers::DX), true);
			break;
		}
		case CPU_Opcode::INSB:
		{
			if (exec)
			{
				get_reg(Registers_8::AL) = _bus.read<uint8_t>(get_reg(Registers::DX), true);
		
				_bus.write<uint8_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), get_reg(Registers_8::AL), false);
		
				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::DI)--;
				}
		
				else
				{
					get_reg(Registers::DI)++;
				}
			}
			break;
		}
		case CPU_Opcode::INSW:
		{
			if (exec)
			{
				get_reg(Registers::AX) = _bus.read<uint16_t>(get_reg(Registers::DX), true);
		
				_bus.write<uint16_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), get_reg(Registers::AX), false);
		
				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::DI) -= 2;
				}
		
				else
				{
					get_reg(Registers::DI) += 2;
				}
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
			Op_Reg<uint16_t> reg{ *this, &get_reg(static_cast<Registers>(static_cast<uint8_t>(opcode) & 0x7)) };
			if (exec) reg++;
			break;
		}
		case CPU_Opcode::INT_IMM8: // CD ii
		{
			uint8_t interrupt{ fetch<Op_Imm<uint8_t>>(data_seg).get_val() };

			if (interrupt == 0x13)
			{
				int a{};
			}
			// Look up in interrupt vector table. Example: int 0x10, 0x10 * 4 = 0x40, get offset at 0x40 and segment at 0x42, jump there
			if (exec)
			{
				uint16_t interrupt_offset{ _bus.read<uint16_t>(to_linear(0, interrupt * 4), false) };
				uint16_t interrupt_segment{ _bus.read<uint16_t>(to_linear(0, interrupt * 4) + 2, false) };

				get_reg(Registers::Flags) &= ~flag_interrupt;

				push(get_reg(Registers::Flags));
				push(get_sreg(Segment_Registers::CS));
				push(get_reg(Registers::IP));

				get_sreg(Segment_Registers::CS) = interrupt_segment;
				get_reg(Registers::IP) = interrupt_offset;
			}
			break;
		}
		case CPU_Opcode::IRET: // CF
		{
			// pop ip, cs, and flags after interrupt
			if (exec)
			{
				get_reg(Registers::IP) = pop();
				get_sreg(Segment_Registers::CS) = pop();
				get_reg(Registers::Flags) = pop();
			}
			break;
		}
		case CPU_Opcode::JA_REL8: // 77 ii
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if ((get_reg(Registers::Flags) & flag_carry) == 0 && (get_reg(Registers::Flags) & flag_zero) == 0)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JAE_REL8: // 73 ii // Same as JNB_REL8, JNC_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if ((get_reg(Registers::Flags) & flag_carry) == 0)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JB_REL8: // 72 ii // Same as JC_REL8, JNAE_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::Flags) & flag_carry)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JBE_REL8: // 76 ii // Same as JNA_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::Flags) & flag_carry || get_reg(Registers::Flags) & flag_zero)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JCXZ_REL8: // E3 ii
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::CX) == 0)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JE_REL8: // 74 ii // Same as JZ_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::Flags) & flag_zero)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JG_REL8: // 7f ii // Same as JNLE_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if ((get_reg(Registers::Flags) & flag_zero) == 0 && (((get_reg(Registers::Flags) & flag_sign) == 0 && (get_reg(Registers::Flags) & flag_overflow) == 0) || ((get_reg(Registers::Flags) & flag_sign) && (get_reg(Registers::Flags) & flag_overflow))))
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JGE_REL8: // 7D ii // Same as JNL_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (((get_reg(Registers::Flags) & flag_sign) == 0 && (get_reg(Registers::Flags) & flag_overflow) == 0) || ((get_reg(Registers::Flags) & flag_sign) && (get_reg(Registers::Flags) & flag_overflow)))
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JL_REL8: // 7c ii // Same as JNGE_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (((get_reg(Registers::Flags) & flag_sign) == 0 && get_reg(Registers::Flags) & flag_overflow) || (get_reg(Registers::Flags) & flag_sign && (get_reg(Registers::Flags) & flag_overflow) == 0))
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JLE_REL8: // 7E ii // Same as JNG_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::Flags) & flag_zero || (((get_reg(Registers::Flags) & flag_sign) == 0 && get_reg(Registers::Flags) & flag_overflow) || (get_reg(Registers::Flags) & flag_sign && (get_reg(Registers::Flags) & flag_overflow) == 0)))
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JNE_REL8: // 75 ii // Same as JNZ_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if ((get_reg(Registers::Flags) & flag_zero) == 0)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JNO_REL8: // 71 ii
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if ((get_reg(Registers::Flags) & flag_overflow) == 0)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JNP_REL8: // 7b ii // Same as JPO_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if ((get_reg(Registers::Flags) & flag_parity) == 0)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JNS_REL8: // 79 ii
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if ((get_reg(Registers::Flags) & flag_sign) == 0)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JO_REL8: // 77 ii
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::Flags) & flag_overflow)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JP_REL8: // 7A ii // Same as JPE_REL8
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::Flags) & flag_parity)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JS_REL8: // 78 ii
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (get_reg(Registers::Flags) & flag_sign)
			{
				if (exec) get_reg(Registers::IP) += imm;
			}
			break;
		}
		case CPU_Opcode::JMP_REL8: // EB ii
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());
			if (exec) get_reg(Registers::IP) += imm;
			break;
		}
		case CPU_Opcode::JMP_REL16: // E9 ii ii
		{
			auto imm = static_cast<int16_t>(fetch<Op_Imm<uint16_t>>(data_seg).get_val());
			if (exec) get_reg(Registers::IP) += imm;
			break;
		}
		case CPU_Opcode::JMP_PTR16_16: // EA ii ii ii ii
		{
			uint16_t new_ip{fetch<Op_Imm<uint16_t>>(data_seg).get_val()};
			uint16_t new_cs{fetch<Op_Imm<uint16_t>>(data_seg).get_val()};
			if (exec)
			{
				get_reg(Registers::IP) = new_ip;
				get_sreg(Segment_Registers::CS) = new_cs;
			}
			break;
		}
		case CPU_Opcode::LAHF:
		{
			if (exec) get_reg(Registers_8::AH) = get_reg(Registers::Flags) & 0x00ff;
			break;
		}
		case CPU_Opcode::LDS:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);

			if (exec)
			{
				auto regmem = modrm.first;
				auto reg = modrm.second;
				reg = (regmem);

				uint16_t segment{ Op_Mem<uint16_t>(*this, std::get<int>(regmem.get_op()) + 2).get_val() };

				get_sreg(Segment_Registers::DS) = segment;
			}
			break;
		}
		case CPU_Opcode::LEA:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);

			if (exec)
			{
				auto regmem = modrm.first;
				auto reg = modrm.second;

				uint16_t addr{ static_cast<uint16_t>(std::get<int>(regmem.get_op()) - (to_linear(data_seg, 0))) };
				reg = (addr);
			}
			break;
		}
		case CPU_Opcode::LES:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);

			if (exec)
			{
				auto regmem = modrm.first;
				auto reg = modrm.second;

				reg = (regmem);

				uint16_t segment{ Op_Mem<uint16_t>(*this, std::get<int>(regmem.get_op()) + 2).get_val() };

				get_sreg(Segment_Registers::ES) = segment;
			}
			break;
		}
		case CPU_Opcode::LODSB:
		{
			if (exec)
			{
				get_reg(Registers_8::AL) = _bus.read<uint8_t>(to_linear(data_seg, get_reg(Registers::SI)), false);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI)--;
				}

				else
				{
					get_reg(Registers::SI)++;
				}
			}
			break;
		}
		case CPU_Opcode::LODSW:
		{
			if (exec)
			{
				get_reg(Registers::AX) = _bus.read<uint16_t>(to_linear(data_seg, get_reg(Registers::SI)), false);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI) -= 2;
				}

				else
				{
					get_reg(Registers::SI) += 2;
				}
			}
			break;
		}
		case CPU_Opcode::LOOP_REL8:
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (exec)
			{
				get_reg(Registers::CX)--;

				if (get_reg(Registers::CX) != 0)
				{
					get_reg(Registers::IP) += imm;
				}
			}
			break;
		}
		case CPU_Opcode::LOOPE_REL8:
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (exec)
			{
				get_reg(Registers::CX)--;

				if (get_reg(Registers::CX) != 0 && get_reg(Registers::Flags) & flag_zero)
				{
					get_reg(Registers::IP) += imm;
				}
			}
			break;
		}
		case CPU_Opcode::LOOPNE_REL8:
		{
			auto imm = static_cast<int8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val());

			if (exec)
			{
				get_reg(Registers::CX)--;

				if (get_reg(Registers::CX) != 0 && (get_reg(Registers::Flags) & flag_zero) == 0)
				{
					get_reg(Registers::IP) += imm;
				}
			}
			break;
		}
		case CPU_Opcode::MOV_RM8_IMM8:
		{
			auto regmem = fetch_modrm<uint8_t>(default_seg, data_seg).first;
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);
			if (exec) regmem = (imm);
			break;
		}
		case CPU_Opcode::MOV_RM16_IMM16:
		{
			auto regmem = fetch_modrm<uint16_t>(default_seg, data_seg).first;
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg);
			if (exec) regmem = (imm);
			break;
		}
		case CPU_Opcode::MOV_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) = (std::get<Op_Reg<uint8_t>>(modrm));
			break;
		}
		case CPU_Opcode::MOV_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) = (std::get<Op_Reg<uint16_t>>(modrm));
			break;
		}
		case CPU_Opcode::MOV_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) = (std::get<Op_Regmem<uint8_t>>(modrm));
			break;
		}
		case CPU_Opcode::MOV_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) = (std::get<Op_Regmem<uint16_t>>(modrm));
			break;
		}
		case CPU_Opcode::MOV_RM16_SREG:
		{
			auto modrm = fetch_modrm<uint16_t, true>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) = (std::get<Op_Reg<uint16_t>>(modrm));
			break;
		}
		case CPU_Opcode::MOV_SREG_RM16:
		{
			auto modrm = fetch_modrm<uint16_t, true>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) = (std::get<Op_Regmem<uint16_t>>(modrm));
			break;
		}
		case CPU_Opcode::MOV_AL_IMM8: 
		case CPU_Opcode::MOV_CL_IMM8: 
		case CPU_Opcode::MOV_DL_IMM8: 
		case CPU_Opcode::MOV_BL_IMM8: 
		case CPU_Opcode::MOV_AH_IMM8: 
		case CPU_Opcode::MOV_CH_IMM8: 
		case CPU_Opcode::MOV_DH_IMM8: 
		case CPU_Opcode::MOV_BH_IMM8:
		{
			Op_Reg<uint8_t> reg{ *this, &get_reg(static_cast<Registers_8>(static_cast<uint8_t>(opcode) & 0x7)) };
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);
			if(exec) reg = (imm);
			break;
		}
		case CPU_Opcode::MOV_AX_IMM16: 
		case CPU_Opcode::MOV_CX_IMM16: 
		case CPU_Opcode::MOV_DX_IMM16: 
		case CPU_Opcode::MOV_BX_IMM16: 
		case CPU_Opcode::MOV_SP_IMM16:
		case CPU_Opcode::MOV_BP_IMM16:
		case CPU_Opcode::MOV_SI_IMM16:
		case CPU_Opcode::MOV_DI_IMM16:
		{
			Op_Reg<uint16_t> reg{ *this, &get_reg(static_cast<Registers>(static_cast<uint8_t>(opcode) & 0x7)) };
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg);
			if(exec) reg = (imm);
			break;
		}
		case CPU_Opcode::MOV_AL_MOFFS8: get_reg(Registers_8::AL) = fetch<Op_Mem<uint8_t>>(data_seg).get_val(); break;
		case CPU_Opcode::MOV_AX_MOFFS16: get_reg(Registers::AX) = fetch<Op_Mem<uint16_t>>(data_seg).get_val(); break;
		case CPU_Opcode::MOV_MOFFS8_AL: fetch<Op_Mem<uint8_t>>(data_seg) = (get_reg(Registers_8::AL)); break;
		case CPU_Opcode::MOV_MOFFS16_AX: fetch<Op_Mem<uint16_t>>(data_seg) = (get_reg(Registers::AX)); break;
		case CPU_Opcode::MOVSB:
		{
			if (exec)
			{
				_bus.write<uint8_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), _bus.read<uint8_t>(to_linear(data_seg, get_reg(Registers::SI)), false), false);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI)--;
					get_reg(Registers::DI)--;
				}

				else
				{
					get_reg(Registers::SI)++;
					get_reg(Registers::DI)++;
				}
			}
			break;
		}
		case CPU_Opcode::MOVSW:
		{
			if (exec)
			{
				_bus.write<uint16_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), _bus.read<uint16_t>(to_linear(data_seg, get_reg(Registers::SI)), false), false);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI) -= 2;
					get_reg(Registers::DI) -= 2;
				}

				else
				{
					get_reg(Registers::SI) += 2;
					get_reg(Registers::DI) += 2;
				}
			}
			break;
		}
		case CPU_Opcode::OR_AL_IMM8: Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL)) |= fetch<Op_Imm<uint8_t>>(data_seg); break;
		case CPU_Opcode::OR_AX_IMM16: Op_Reg<uint16_t>(*this, &get_reg(Registers::AX)) |= fetch<Op_Imm<uint16_t>>(data_seg); break;
		case CPU_Opcode::OR_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) |= std::get<Op_Reg<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::OR_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) |= std::get<Op_Reg<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::OR_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) |= std::get<Op_Regmem<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::OR_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) |= std::get<Op_Regmem<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::OUT_IMM8_AL: _bus.write<uint8_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val(), get_reg(Registers_8::AL), true); break;
		case CPU_Opcode::OUT_IMM8_AX: _bus.write<uint16_t>(fetch<Op_Imm<uint8_t>>(data_seg).get_val(), get_reg(Registers::AX), true); break;
		case CPU_Opcode::OUT_DX_AL: _bus.write<uint8_t>(get_reg(Registers::DX), get_reg(Registers_8::AL), true); break;
		case CPU_Opcode::OUT_DX_AX: _bus.write<uint16_t>(get_reg(Registers::DX), get_reg(Registers::AX), true); break;
		case CPU_Opcode::OUTSB:
		{
			if (exec)
			{
				get_reg(Registers_8::AL) = _bus.read<uint8_t>(to_linear(data_seg, get_reg(Registers::SI)), true);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI)--;
				}

				else
				{
					get_reg(Registers::SI)++;
				}

				_bus.write<uint8_t>(get_reg(Registers::DX), get_reg(Registers_8::AL), true);
			}
			break;
		}
		case CPU_Opcode::OUTSW:
		{
			if (exec)
			{
				get_reg(Registers::AX) = _bus.read<uint16_t>(to_linear(data_seg, get_reg(Registers::SI)), true);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::SI) -= 2;
				}

				else
				{
					get_reg(Registers::SI) += 2;
				}

				_bus.write<uint16_t>(get_reg(Registers::DX), get_reg(Registers::AX), true);
			}
			break;
		}
		case CPU_Opcode::PUSHA:
		{
			for (int i{}; i < 8; i++)
			{
				push(get_reg(static_cast<Registers>(i)));
			}

			break;
		}
		case CPU_Opcode::PUSH_AX:
		case CPU_Opcode::PUSH_CX:
		case CPU_Opcode::PUSH_DX:
		case CPU_Opcode::PUSH_BX:
		case CPU_Opcode::PUSH_SP:
		case CPU_Opcode::PUSH_BP:
		case CPU_Opcode::PUSH_SI:
		case CPU_Opcode::PUSH_DI:
		{
			if (exec) push(get_reg(static_cast<Registers>(static_cast<uint8_t>(opcode) & 0x7)));
			break;
		}
		case CPU_Opcode::PUSH_ES: if (exec) push(get_sreg(Segment_Registers::ES)); break;
		case CPU_Opcode::PUSH_CS: if (exec) push(get_sreg(Segment_Registers::CS)); break;
		case CPU_Opcode::PUSH_SS: if (exec) push(get_sreg(Segment_Registers::SS)); break;
		case CPU_Opcode::PUSH_DS: if (exec) push(get_sreg(Segment_Registers::DS)); break;
		case CPU_Opcode::PUSHF: if (exec) push(get_reg(Registers::Flags)); break;
		case CPU_Opcode::PUSH_IMM8: push(fetch<Op_Imm<uint8_t>>(data_seg).get_val()); break;
		case CPU_Opcode::PUSH_IMM16: push(fetch<Op_Imm<uint16_t>>(data_seg).get_val()); break;
		case CPU_Opcode::POPA:
		{
			for (int i{ 7 }; i >= 0; i--)
			{
				if (i == 4) // popping sp (into nothing)
				{
					pop();
				}

				else
				{
					get_reg(static_cast<Registers>(i)) = pop();
				}
			}
			break;
		}
		case CPU_Opcode::POP_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if(exec) std::get<Op_Regmem<uint16_t>>(modrm) = (pop()); break;
			break;
		}
		case CPU_Opcode::POP_AX:
		case CPU_Opcode::POP_CX:
		case CPU_Opcode::POP_DX:
		case CPU_Opcode::POP_BX:
		case CPU_Opcode::POP_SP:
		case CPU_Opcode::POP_BP:
		case CPU_Opcode::POP_SI:
		case CPU_Opcode::POP_DI:
		{
			if (exec) get_reg(static_cast<Registers>(static_cast<uint8_t>(opcode) & 0x7)) = pop();
			break;
		}
		case CPU_Opcode::POP_ES: if (exec) get_sreg(Segment_Registers::ES) = pop(); break;
		case CPU_Opcode::POP_CS: if (exec) get_sreg(Segment_Registers::CS) = pop(); break;
		case CPU_Opcode::POP_SS: if (exec) get_sreg(Segment_Registers::SS) = pop(); break;
		case CPU_Opcode::POP_DS: if (exec) get_sreg(Segment_Registers::DS) = pop(); break;
		case CPU_Opcode::POPF: if (exec) get_reg(Registers::Flags) = pop(); break;
		case CPU_Opcode::RET_FAR:
		{
			if (exec)
			{
				get_reg(Registers::IP) = pop();
				get_sreg(Segment_Registers::CS) = pop();
			}
			break;
		}
		case CPU_Opcode::RET_FAR_IMM16:
		{
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg).get_val();

			if (exec)
			{
				get_reg(Registers::IP) = pop();
				get_sreg(Segment_Registers::CS) = pop();
				get_reg(Registers::SP) += imm;
			}
			break;
		}
		case CPU_Opcode::RET_NEAR: if (exec) get_reg(Registers::IP) = pop(); break;
		case CPU_Opcode::RET_NEAR_IMM16:
		{
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg).get_val();

			if (exec)
			{
				get_reg(Registers::IP) = pop();
				get_reg(Registers::SP) += imm;
			}
			break;
		}
		case CPU_Opcode::SAHF:
		{
			if (exec)
			{
				get_reg(Registers::Flags) &= ~0x00ff;
				get_reg(Registers::Flags) |= get_reg(Registers_8::AH);
			}
			break;
		}
		case CPU_Opcode::SBB_AL_IMM8: Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL)) -= static_cast<uint8_t>((fetch<Op_Imm<uint8_t>>(data_seg).get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry))); break;
		case CPU_Opcode::SBB_AX_IMM16: Op_Reg<uint16_t>(*this, &get_reg(Registers::AX)) -= static_cast<uint16_t>((fetch<Op_Imm<uint16_t>>(data_seg).get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));; break;
		case CPU_Opcode::SBB_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) -= static_cast<uint8_t>((std::get<Op_Reg<uint8_t>>(modrm).get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::SBB_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) -= static_cast<uint16_t>((std::get<Op_Reg<uint16_t>>(modrm).get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::SBB_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) -= static_cast<uint8_t>((std::get<Op_Regmem<uint8_t>>(modrm).get_val() + static_cast<uint8_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::SBB_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) -= static_cast<uint16_t>((std::get<Op_Regmem<uint16_t>>(modrm).get_val() + static_cast<uint16_t>(get_reg(Registers::Flags) & flag_carry)));
			break;
		}
		case CPU_Opcode::SCASB:
		{
			if (exec)
			{
				uint8_t temp_var{ get_reg(Registers_8::AL) };
				Op_Reg<uint8_t> temp{ *this, &get_reg(Registers_8::AL) };
				temp -= _bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), false);
				temp = (temp_var);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::DI)--;
				}

				else
				{
					get_reg(Registers::DI)++;
				}

				_zero_flag_check = true;
			}

			break;
		}
		case CPU_Opcode::SCASW:
		{
			if (exec)
			{
				uint16_t temp_var{ get_reg(Registers::AX) };
				Op_Reg<uint16_t> temp{ *this, &get_reg(Registers::AX) };
				temp -= _bus.read<uint16_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), false);
				temp = (temp_var);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::DI) -= 2;
				}

				else
				{
					get_reg(Registers::DI) += 2;
				}

				_zero_flag_check = true;
			}

			break;
		}
		case CPU_Opcode::STC:
		{
			if (exec) get_reg(Registers::Flags) |= flag_carry;
			break;
		}
		case CPU_Opcode::STD:
		{
			if (exec) get_reg(Registers::Flags) |= flag_direction;
			break;
		}
		case CPU_Opcode::STI:
		{
			if (exec) _sti_enable = true;
			break;
		}
		case CPU_Opcode::STOSB:
		{
			if (exec)
			{
				_bus.write<uint8_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), get_reg(Registers_8::AL), false);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::DI)--;
				}

				else
				{
					get_reg(Registers::DI)++;
				}
			}
			break;
		}
		case CPU_Opcode::STOSW:
		{
			if (exec)
			{
				_bus.write<uint16_t>(to_linear(get_sreg(Segment_Registers::ES), get_reg(Registers::DI)), get_reg(Registers::AX), false);

				if (get_reg(Registers::Flags) & flag_direction)
				{
					get_reg(Registers::DI) -= 2;
				}

				else
				{
					get_reg(Registers::DI) += 2;
				}
			}

			break;
		}
		case CPU_Opcode::SUB_AL_IMM8: Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL)) -= fetch<Op_Imm<uint8_t>>(data_seg); break;
		case CPU_Opcode::SUB_AX_IMM16: Op_Reg<uint16_t>(*this, &get_reg(Registers::AX)) -= fetch<Op_Imm<uint16_t>>(data_seg); break;
		case CPU_Opcode::SUB_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if(exec) std::get<Op_Regmem<uint8_t>>(modrm) -= std::get<Op_Reg<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::SUB_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) -= std::get<Op_Reg<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::SUB_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) -= std::get<Op_Regmem<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::SUB_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) -= std::get<Op_Regmem<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::TEST_AL_IMM8:
		{
			auto reg = Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL));
			auto imm = fetch<Op_Imm<uint8_t>>(data_seg);
			uint8_t temp_var{ reg.get_val() };
			if (exec) reg &= imm;
			reg = (temp_var);
			break;
		}
		case CPU_Opcode::TEST_AX_IMM16:
		{
			auto reg = Op_Reg<uint16_t>(*this, &get_reg(Registers::AX));
			auto imm = fetch<Op_Imm<uint16_t>>(data_seg);
			uint16_t temp_var{ reg.get_val() };
			if (exec) reg &= imm;
			reg = (temp_var);
			break;
		}
		case CPU_Opcode::TEST_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			uint8_t temp_var{ std::get<Op_Regmem<uint8_t>>(modrm).get_val() };
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) &= std::get<Op_Reg<uint8_t>>(modrm);
			std::get<Op_Regmem<uint8_t>>(modrm) = (temp_var);
			break;
		}
		case CPU_Opcode::TEST_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			uint16_t temp_var{ std::get<Op_Regmem<uint16_t>>(modrm).get_val() };
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) &= std::get<Op_Reg<uint16_t>>(modrm);
			std::get<Op_Regmem<uint16_t>>(modrm) = (temp_var);
			break;
		}
		case CPU_Opcode::XCHG_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			uint8_t temp_var{std::get<Op_Regmem<uint8_t>>(modrm).get_val()};
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) = (std::get<Op_Reg<uint8_t>>(modrm));
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) = (temp_var);
			break;
		}
		case CPU_Opcode::XCHG_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			uint16_t temp_var{std::get<Op_Regmem<uint16_t>>(modrm).get_val()};
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) = (std::get<Op_Reg<uint16_t>>(modrm));
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) = (temp_var);
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
			Op_Reg<uint16_t> reg{ *this, &get_reg(static_cast<Registers>(static_cast<uint8_t>(opcode) & 0x7)) };
			uint16_t temp{get_reg(Registers::AX)};
			if (exec) get_reg(Registers::AX) = reg.get_val();
			if (exec) reg = (temp);
			break;
		}
		case CPU_Opcode::XLAT:
		{
			uint16_t temp{ static_cast<uint16_t>(get_reg(Registers_8::AL)) };
			if (exec) get_reg(Registers_8::AL) = _bus.read<uint8_t>(to_linear(data_seg, get_reg(Registers::BX) + temp), false);
			break;
		}
		case CPU_Opcode::XOR_AL_IMM8: Op_Reg<uint8_t>(*this, &get_reg(Registers_8::AL)) ^= fetch<Op_Imm<uint8_t>>(data_seg); break;
		case CPU_Opcode::XOR_AX_IMM16: Op_Reg<uint16_t>(*this, &get_reg(Registers::AX)) ^= fetch<Op_Imm<uint16_t>>(data_seg); break;
		case CPU_Opcode::XOR_RM8_R8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint8_t>>(modrm) ^= std::get<Op_Reg<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::XOR_RM16_R16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Regmem<uint16_t>>(modrm) ^= std::get<Op_Reg<uint16_t>>(modrm);
			break;
		}
		case CPU_Opcode::XOR_R8_RM8:
		{
			auto modrm = fetch_modrm<uint8_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint8_t>>(modrm) ^= std::get<Op_Regmem<uint8_t>>(modrm);
			break;
		}
		case CPU_Opcode::XOR_R16_RM16:
		{
			auto modrm = fetch_modrm<uint16_t>(default_seg, data_seg);
			if (exec) std::get<Op_Reg<uint16_t>>(modrm) ^= std::get<Op_Regmem<uint16_t>>(modrm);
			break;
		}
		default:
		{
			std::println("[CPU] Unknown Opcode {:x}, {:x}:{:x}", static_cast<uint8_t>(opcode), get_sreg(Segment_Registers::CS), opcode_ip);
			break;
		}
		}

		if (get_reg(Registers::IP) == 0xecc2)
		{
			int a{};
		}

		if(reset_ip)
		{
			get_reg(Registers::IP) = opcode_ip;
		}

		if (dec_cx)
		{
			get_reg(Registers::CX)--;
		}
	}

	check_irq();
}
