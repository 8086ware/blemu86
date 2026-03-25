#include "cga.h"

bool CGA::CrtC::write(int address, uint8_t data, bool io)
{
    if (io)
    {
        switch (static_cast<Port>(address))
        {
        case Port::Index:
        {
            _index = data;
            break;
        }
        case Port::Data:
        {
            switch (static_cast<Data_Reg_Options>(_index))
            {
            case Data_Reg_Options::Cursor_Start_Scanline:
            {
                _cursor_start_scan_line = data;
                break;
            }
            case Data_Reg_Options::Cursor_End_Scanline:
            {
                _cursor_end_scan_line = data;
                break;
            }
            case Data_Reg_Options::Start_Address_High:
            {
                uint16_t real_value = data;
                real_value <<= 8;
                real_value |= (_start_address & 0x00ff);
                _start_address = real_value;
                break;
            }
            case Data_Reg_Options::Start_Address_Low:
            {
                uint16_t real_value = data;
                real_value |= (_start_address & 0xff00);
                _start_address = real_value;
                break;
            }
            case Data_Reg_Options::Cursor_Address_High:
            {
                uint16_t real_value = data;
                real_value <<= 8;
                real_value |= (_cursor_address & 0x00ff);
                _cursor_address = real_value;
                break;
            }
            case Data_Reg_Options::Cursor_Address_Low:
            {
                uint16_t real_value = data;
                real_value |= (_cursor_address & 0xff00);
                _cursor_address = real_value;
                break;
            }
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

std::optional<uint8_t> CGA::CrtC::read(int address, bool io)
{
    if (io)
    {
        switch (static_cast<Port>(address))
        {
        case Port::Data:
        {
            switch (static_cast<Data_Reg_Options>(_index))
            {
            case Data_Reg_Options::Cursor_Start_Scanline:
            {
                return _cursor_start_scan_line;
                break;
            }
            case Data_Reg_Options::Cursor_End_Scanline:
            {
                return _cursor_end_scan_line;
                break;
            }
            case Data_Reg_Options::Start_Address_High:
            {
                return (_start_address & 0xff00) >> 8;
                break;
            }
            case Data_Reg_Options::Start_Address_Low:
            {
                return _start_address & 0x00ff;
                break;
            }
            case Data_Reg_Options::Cursor_Address_High:
            {
                return (_cursor_address & 0xff00) >> 8;
                break;
            }
            case Data_Reg_Options::Cursor_Address_Low:
            {
                return _cursor_address & 0x00ff;
                break;
            }
            }
            break;
        }
        }
    }

    return std::nullopt;
}

CGA::CrtC::CrtC() : Device(true, 0)
{
    std::println("[CrtC] Init...");
}