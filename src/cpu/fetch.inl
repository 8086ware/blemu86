#pragma once
#include "cpu.h"
#include "to_linear.h"

template<Op_Normal_Type O> O CPU::fetch(uint16_t data_seg)
{
    if constexpr (std::is_same_v<O, Op_Mem<decltype(std::declval<O>().get_val())>>)
    {
        get_reg(Registers::IP) += sizeof(uint16_t);
        return Op_Mem<decltype(std::declval<O>().get_val())>(*this, to_linear(data_seg, _bus.read<uint16_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP) - sizeof(uint16_t)), false)));
    }

    else if constexpr (std::is_same_v<O, Op_Imm<decltype(std::declval<O>().get_val())>>)
    {
        get_reg(Registers::IP) += sizeof(decltype(std::declval<O>().get_val()));
        return Op_Imm<decltype(std::declval<O>().get_val())>(*this, _bus.read<decltype(std::declval<O>().get_val())>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP) - sizeof(decltype(std::declval<O>().get_val()))), false));
    }
}

template<CPU_Width T, bool sreg> std::pair<Op_Regmem<T>, Op_Reg<T>> CPU::fetch_modrm(bool default_seg, uint16_t data_seg)
{
    uint8_t modrm{ _bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false) };
    get_reg(Registers::IP)++;

    bool mem_or_reg{}; // mem = 1, reg = 0

    // modrm == 0b10'000'000

    uint8_t mod_val{ static_cast<uint8_t>((modrm & 0b11000000) >> 6) };
    uint8_t rm_val{ static_cast<uint8_t>(modrm & 0b00000111) };
    uint8_t reg_val{ static_cast<uint8_t>((modrm & 0b00111000) >> 3) };

	std::variant<int16_t, int8_t> displacement{};

    if (mod_val == 0b10 || (mod_val == 0b00 && rm_val == 6))
    {
        displacement.emplace<int16_t>(_bus.read<uint16_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false));
        get_reg(Registers::IP) += sizeof(uint16_t);
    }

    else if(mod_val == 0b01)
    {
        displacement.emplace<int8_t>(_bus.read<uint8_t>(to_linear(get_sreg(Segment_Registers::CS), get_reg(Registers::IP)), false));
        get_reg(Registers::IP) += sizeof(uint8_t);
    }

    uint16_t regmem_mem{};
    T* regmem_reg{ nullptr };

    // Using mem, not reg in regmem
    if (mod_val == 0b01 || mod_val == 0b10 || mod_val == 0b00) // No displacement for mod 0 (except rm_val == 6)
    {
        mem_or_reg = true;

        switch (rm_val)
        {
        case 0:
        {
            regmem_mem += get_reg(Registers::BX) + get_reg(Registers::SI);
            break;
        }
        case 1:
        {
            regmem_mem += get_reg(Registers::BX) + get_reg(Registers::DI);
            break;
        }
        case 2:
        {
            if (default_seg)
            {
                data_seg = get_sreg(Segment_Registers::SS);
            }

            regmem_mem += get_reg(Registers::BP) + get_reg(Registers::SI);
            break;
        }
        case 3:
        {
            if (default_seg)
            {
                data_seg = get_sreg(Segment_Registers::SS);
            }

            regmem_mem += get_reg(Registers::BP) + get_reg(Registers::DI);
            break;
        }
        case 4:
        {
            regmem_mem += get_reg(Registers::SI);
            break;
        }
        case 5:
        {
            regmem_mem += get_reg(Registers::DI);
            break;
        }
        case 6:
        {
            if (mod_val == 0)
            {
                regmem_mem += std::get<int16_t>(displacement);
            }

            else
            {
                if (default_seg)
                {
                    data_seg = get_sreg(Segment_Registers::SS);
                }

                regmem_mem += get_reg(Registers::BP);
            }

            break;
        }
        case 7:
        {
            regmem_mem += get_reg(Registers::BX);
            break;
        }
        }

        // Add displacement to regmem first before adding (if displacement provided)

        if (mod_val == 0b10)
        {
            regmem_mem += std::get<int16_t>(displacement);
        }

        else if(mod_val == 0b01)
        {
            regmem_mem += std::get<int8_t>(displacement);
        }
    }

    // rm is same as reg

    else if (mod_val == 0b11)
    {
        mem_or_reg = false;

        if constexpr (std::is_same_v<T, uint16_t>)
        {
            regmem_reg = &get_reg(static_cast<Registers>(rm_val));
        }

        else if constexpr (std::is_same_v<T, uint8_t>)
        {
            regmem_reg = &get_reg(static_cast<Registers_8>(rm_val));
        }
    }

    if constexpr (std::is_same_v<T, uint16_t>)
    {
        if constexpr (sreg)
        {
            if (mem_or_reg)
            {
                return std::make_pair(Op_Regmem<T>(*this, to_linear(data_seg, regmem_mem)), Op_Reg<T>(*this, &get_sreg(static_cast<Segment_Registers>(reg_val))));
            }

            else
            {
                return std::make_pair(Op_Regmem<T>(*this, &get_reg(static_cast<Registers>(rm_val))), Op_Reg<T>(*this, &get_sreg(static_cast<Segment_Registers>(reg_val))));
            }
        }

        else
        {
            if (mem_or_reg)
            {
                return std::make_pair(Op_Regmem<T>(*this, to_linear(data_seg, regmem_mem)), Op_Reg<T>(*this, &get_reg(static_cast<Registers>(reg_val))));
            }

            else
            {
                return std::make_pair(Op_Regmem<T>(*this, &get_reg(static_cast<Registers>(rm_val))), Op_Reg<T>(*this, &get_reg(static_cast<Registers>(reg_val))));
            }
        }
    }

    else if constexpr (std::is_same_v<T, uint8_t>)
    {
        if (mem_or_reg)
        {
            return std::make_pair(Op_Regmem<T>(*this, to_linear(data_seg, regmem_mem)), Op_Reg<T>(*this, &get_reg(static_cast<Registers_8>(reg_val))));
        }

        else
        {
            return std::make_pair(Op_Regmem<T>(*this, &get_reg(static_cast<Registers_8>(rm_val))), Op_Reg<T>(*this, &get_reg(static_cast<Registers_8>(reg_val))));
        }
    }

}
