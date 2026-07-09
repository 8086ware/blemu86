#include "cga.h"
#include <chrono>
#include <print>
#include <cmath>
#include <bitset>

CGA::CGA(sf::RenderWindow& win) : _win{ win }
{
    std::println("[CGA] Init...");
    _color_ctrl_reg |= color_ctrl_bbf;
    _win.setVerticalSyncEnabled(false);
	_win.setFramerateLimit(1000);
}

bool CGA::write(int address, uint8_t data, bool io)
{
	if(io)
	{
		switch (static_cast<Port>(address))
		{
		case Port::Mode_Control:
		{
			_mode_ctrl_reg = data;

			if (_mode_ctrl_reg & mode_ctrl_high_res_graphics)
			{
                _image.resize({ 640, 200 });
			}

			else if (_mode_ctrl_reg & mode_ctrl_graphics)
			{
                _image.resize({ 320, 200 });
			}

			else
			{
				if (_mode_ctrl_reg & mode_ctrl_high_res)
				{
                    _image.resize({ 640, 200 });
                }

				else
				{
                    _image.resize({ 320, 200 });
                }
			}

			break;
		}

		case Port::Color_Control:
		{
			_color_ctrl_reg = data;
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
        if (address >= cga_address && address < cga_address + cga_ram_size)
        {
            _ram[address - cga_address] = data;
        }

        else
        {
            return false;
        }
    }

    return true;
}

std::optional<uint8_t> CGA::read(int address, bool io)
{
	if (!io) // 0xB8000
	{
		if (address >= cga_address && address < cga_address + cga_ram_size)
		{
			return _ram[address - cga_address];
		}
	}

    else
    {
        switch (static_cast<Port>(address))
        {
        case Port::Status_Register:
        {
            uint8_t retrace{ static_cast<uint8_t>(rand() % 2) }; // give some random number because why man?

            return retrace ? 0x9 : 0x0;
            break;
        }
        }
    }

    return std::nullopt;
}

void CGA::cycle()
{
    unsigned int width{};
    unsigned int height{};
    unsigned int columns{};
    unsigned int rows{};

    if (_mode_ctrl_reg & mode_ctrl_video) // Is this thing on?
    {
        _win.clear();

        if (_mode_ctrl_reg & mode_ctrl_graphics)
        {
            // graphics mode

            if (_mode_ctrl_reg & mode_ctrl_high_res_graphics) // 640x200 2 colors (1bpp, 640x200/8 = 16k)
            {
                width = 640;
                height = 200;

                for (unsigned int y{}; y < height; y++)
                {
                    for (unsigned int x{}; x < width; x += 8)
                    {
                        for (int i{}; i < 8; i++)
                        {
                            uint8_t color{ static_cast<uint8_t>((_ram[y * (width / 8) + x/8] >> (7 - i)) & 0x1) };

                            if (color)
                            {
                                _image.setPixel({ x + i, y }, sf::Color::White);
                            }

                            else
                            {
                                _image.setPixel({ x + i, y }, sf::Color::Black);
                            }
                        }
                    }
                }
            }

            else // 320x200 4 colors (2 bits per color, 320x200/(8/2) = 16k)
            {
                width = 320;
                height = 200;

                for (unsigned int y{}; y < height; y++)
                {
                    for (unsigned int x{}; x < width; x += 4)
                    {
                        for (int i{}; i < 4; i++)
                        {
                            uint8_t color{ static_cast<uint8_t>((_ram[y * (width / 4) + x / 4] >> (6 - i*2)) & 0x3) };

                            if (_color_ctrl_reg & color_ctrl_palette)
                            {
                                _image.setPixel({ x + i, y }, cga_palette_1[color]);
                            }

                            else
                            {
                                _image.setPixel({ x + i, y }, cga_palette_0[color]);
                            }
                        }
                    }
                }
            }

        }

        else
        {
            // text mode

            // 80x25 (high res text, 640x200, 16 colors)

            if (_mode_ctrl_reg & mode_ctrl_high_res)
            {
                width = 640;
                height = 200;
                columns = 80;
                rows = 25;
            }

            else //320x200 4bpp text mode(40x25)
            {
                width = 320;
                height = 200;
                columns = 40;
                rows = 25;
            }

            for (unsigned int y{}; y < rows; y++)
            {
                for (unsigned int x{}; x < columns; x++)
                {
                    uint8_t fg_color{ static_cast<uint8_t>(_ram[(crtc._start_address * 2) + ((y * columns + x) * 2 + 1)] & 0x0f) };
                    uint8_t bg_color{ static_cast<uint8_t>((_ram[(crtc._start_address * 2) + ((y * columns + x) * 2 + 1)] & 0xf0) >> 4) };
                    uint8_t character{ _ram[(crtc._start_address * 2) + ((y * columns + x) * 2)] };

                    unsigned int write_location_y{ 8 * y };
                    unsigned int write_location_x{ 8 * x };

                    unsigned int font_location{ 8 * static_cast<unsigned int>(character) };

                    for (unsigned int font_y{}; font_y < 8; font_y++)
                    {
                        for (unsigned int font_x{}; font_x < 8; font_x++)
                        {
                            if ((font[font_location + font_y] << font_x) & 0x80)
                            {
                                _image.setPixel({ write_location_x + font_x, write_location_y + font_y }, cga_4bit_palette[fg_color]);
                            }

                            else
                            {
                                _image.setPixel({ write_location_x + font_x, write_location_y + font_y }, cga_4bit_palette[bg_color]);
                            }
                        }
                    }
                }
            }

            unsigned int cursor_y{ crtc._cursor_address / columns };
            unsigned int cursor_x{ crtc._cursor_address % columns };

            unsigned int write_location_y{ cursor_y * 8 };
            unsigned int write_location_x{ cursor_x * 8 };

            if (cursor_x < columns && cursor_y < rows && cursor_x >= 0 && cursor_y >= 0)
            {
                for (unsigned int y{ crtc._cursor_start_scan_line }; y < crtc._cursor_end_scan_line; y++)
                {
                    for (unsigned int x{}; x < 8; x++)
                    {
                        _image.setPixel({ write_location_x + x, write_location_y + y }, sf::Color::White);
                    }
                }
            }
        }

        sf::Texture texture{ _image };
        sf::Sprite sprite{ texture };
        sf::Vector2u window_size{ _win.getView().getSize() };

        sprite.setScale({ static_cast<float>(window_size.x) / width, static_cast<float>(window_size.y) / height });
        sprite.setPosition({ 0,0 });

        _win.draw(sprite);
        _win.display();
    }
}
