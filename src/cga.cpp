#include "cga.h"
#include <SDL3/SDL.h>
#include <chrono>
#include <print>

CGA::CGA(SDL_Renderer* win_render) : _win_render{ win_render }
{
    std::println("[CGA] Init...");

    _palette_4bit = SDL_CreatePalette(16);
    SDL_SetPaletteColors(_palette_4bit, cga_4bit_palette.data(), 0, 16);

    // This is for 640x200 graphics mode
    SDL_Color color_1bit[2] = { {0, 0, 0, 0}, {0xFF, 0xFF, 0xFF, 0xFF} };
    _palette_1bit = SDL_CreatePalette(2);
    SDL_SetPaletteColors(_palette_1bit, color_1bit, 0, 2);

    _palette_0 = SDL_CreatePalette(4);
    SDL_SetPaletteColors(_palette_0, cga_palette_0.data(), 0, 4);

    _palette_1 = SDL_CreatePalette(4);
    SDL_SetPaletteColors(_palette_1, cga_palette_1.data(), 0, 4);
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
				SDL_DestroySurface(_surface);
				_surface = SDL_CreateSurface(640, 200, SDL_PIXELFORMAT_INDEX1MSB);
				SDL_SetSurfacePalette(_surface, _palette_1bit);
			}

			else if (_mode_ctrl_reg & mode_ctrl_graphics)
			{
				SDL_DestroySurface(_surface);
				_surface = SDL_CreateSurface(320, 200, SDL_PIXELFORMAT_INDEX2MSB);
				if (_color_ctrl_reg & color_ctrl_palette) // magenta, cyan, white
				{
					SDL_SetSurfacePalette(_surface, _palette_1);
				}

				else // red, green, yellow
				{
					SDL_SetSurfacePalette(_surface, _palette_0);
				}
			}

			else
			{
				SDL_DestroySurface(_surface);

				if (_mode_ctrl_reg & mode_ctrl_high_res)
				{
					_surface = SDL_CreateSurface(640, 200, SDL_PIXELFORMAT_INDEX4MSB);
				}

				else
				{
					_surface = SDL_CreateSurface(320, 200, SDL_PIXELFORMAT_INDEX4MSB);
				}

				SDL_SetSurfacePalette(_surface, _palette_4bit);
			}

			break;
		}

		case Port::Color_Control:
		{
			_color_ctrl_reg = data;

			// CGA_COLOR_CONTROL_BORDER_BACKGROUND_FOREGROUND set

			if (_color_ctrl_reg & mode_ctrl_high_res_graphics && _mode_ctrl_reg & mode_ctrl_graphics) // 640x200 1bpp (foreground of bit)
			{
                _palette_1bit->colors[1] = cga_4bit_palette[_color_ctrl_reg & color_ctrl_bbf];
				SDL_SetPaletteColors(_palette_1bit, _palette_1bit->colors, 0, 2);
			}

			else if (_mode_ctrl_reg & mode_ctrl_graphics) // 320x200 2bpp (background)
			{
				_palette_0->colors[0] = cga_4bit_palette[_color_ctrl_reg & color_ctrl_bbf];
				_palette_1->colors[0] = cga_4bit_palette[_color_ctrl_reg & color_ctrl_bbf];

				if (_color_ctrl_reg & color_ctrl_palette) // magenta, cyan, white
				{
					SDL_SetSurfacePalette(_surface, _palette_1);
				}

				else // red, green, yellow
				{
					SDL_SetSurfacePalette(_surface, _palette_0);
				}

				SDL_SetPaletteColors(_palette_0, _palette_0->colors, 0, 4);
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
            uint8_t retrace{ static_cast<uint8_t>((rand() % 2)) }; // give some random number because why man?

            return retrace ? 0x9 : 0x0;
            break;
        }
        }
    }

    return std::nullopt;
}

void CGA::cycle()
{
    SDL_ClearSurface(_surface, 0, 0, 0, 0);

    int width{};
    int height{};
    int columns{};
    int rows{};

    int bpp{};

    if (_mode_ctrl_reg & mode_ctrl_video) // Is this thing on?
    {
        if (_mode_ctrl_reg & mode_ctrl_graphics)
        {
            // graphics mode

            if (_mode_ctrl_reg & mode_ctrl_high_res_graphics) // 640x200 2 colors (1bpp, 640x200/8 = 16k)
            {
                width = 640;
                height = 200;
                bpp = 1;
            }

            else // 320x200 4 colors (2 bits per color, 320x200/(8/2) = 16k)
            {
                width = 320;
                height = 200;
                bpp = 2;
            }

            memcpy(_surface->pixels, _ram.data(), (width * height) / (8 / bpp));
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

                bpp = 4;
            }

            else //320x200 4bpp text mode(40x25)
            {
                width = 320;
                height = 200;
                columns = 40;
                rows = 25;

                bpp = 4;
            }

            for (int y{}; y < rows; y++)
            {
                for (int x{}; x < columns; x++)
                {
                    uint8_t fg_color{ static_cast<uint8_t>(_ram[(crtc._start_address * 2) + ((y * columns + x) * 2 + 1)] & 0x0f) };
                    uint8_t bg_color{ static_cast<uint8_t>((_ram[(crtc._start_address * 2) + ((y * columns + x) * 2 + 1)] & 0xf0) >> 4) };
                    uint8_t character{ _ram[(crtc._start_address * 2) + ((y * columns + x) * 2)] };

                    int write_location_y{ 8 * y };
                    int write_location_x{ 8 * x };

                    int font_location{ 8 * static_cast<int>(character) };

                    for (int font_y{}; font_y < 8; font_y++)
                    {
                        for (int font_x{}; font_x < 8; font_x += 2)
                        {
                            if ((font[font_location + font_y] << font_x) & 0x80)
                            {
                                reinterpret_cast<uint8_t*>(_surface->pixels)[(write_location_y + font_y) * (width / (8 / bpp)) + (write_location_x / (8 / bpp) + font_x / (8 / bpp))] |= (fg_color << 4);
                            }

                            else
                            {
                                reinterpret_cast<uint8_t*>(_surface->pixels)[(write_location_y + font_y) * (width / (8 / bpp)) + (write_location_x / (8 / bpp) + font_x / (8 / bpp))] |= (bg_color << 4);
                            }

                            if ((font[font_location + font_y] << font_x) & 0x40)
                            {
                                reinterpret_cast<uint8_t*>(_surface->pixels)[(write_location_y + font_y) * (width / (8 / bpp)) + (write_location_x / (8 / bpp) + font_x / (8 / bpp))] |= fg_color;
                            }

                            else
                            {
                                reinterpret_cast<uint8_t*>(_surface->pixels)[(write_location_y + font_y) * (width / (8 / bpp)) + (write_location_x / (8 / bpp) + font_x / (8 / bpp))] |= bg_color;
                            }
                        }
                    }
                }
            }

            int cursor_y{ crtc._cursor_address / columns };
            int cursor_x{ crtc._cursor_address % columns };

            int write_location_y{ cursor_y * 8 };
            int write_location_x{ cursor_x * 4 };

            if (cursor_x < columns && cursor_y < rows && cursor_x >= 0 && cursor_y >= 0)
            {
                for (int y = write_location_y + crtc._cursor_start_scan_line; y < write_location_y + crtc._cursor_end_scan_line; y++)
                {
                    for (int x = write_location_x; x < write_location_x + 8 / (8 / bpp); x++)
                    {
                        reinterpret_cast<uint8_t*>(_surface->pixels)[y * (width / (8 / bpp)) + x] = 0xFF;
                    }
                }
            }
        }
    }

    SDL_Surface* converted{ SDL_ConvertSurface(_surface, SDL_PIXELFORMAT_RGBA8888) };
    SDL_Texture* screen_texture{ SDL_CreateTextureFromSurface(_win_render, converted) };

    SDL_SetTextureScaleMode(screen_texture, SDL_SCALEMODE_NEAREST);
    SDL_RenderTexture(_win_render, screen_texture, NULL, NULL);
    SDL_RenderPresent(_win_render);
    SDL_DestroyTexture(screen_texture);

    SDL_DestroySurface(converted);
}

CGA::~CGA()
{
    SDL_DestroyRenderer(_win_render);
    SDL_DestroyPalette(_palette_0);
    SDL_DestroyPalette(_palette_1);
    SDL_DestroyPalette(_palette_1bit);
    SDL_DestroyPalette(_palette_4bit);
    SDL_DestroySurface(_surface);
}