#include "keyboard.h"
#include <utility>
#include <print>

Keyboard::Keyboard(PPI& ppi, PIC& pic) : Device(false, 150000), _ppi{ ppi }, _pic{ pic }
{
    std::println("[XTKeyboard] Init...");
}

void Keyboard::cycle()
{
    SDL_Event event{};

    SDL_PollEvent(&event);

    if (event.type == SDL_EVENT_KEY_UP || event.type == SDL_EVENT_KEY_DOWN)
    {
        switch (event.key.scancode)
        {
        case SDL_SCANCODE_ESCAPE:
        {
            _scancodes.push_back(0x1);
            break;
        }
        case SDL_SCANCODE_RETURN:
        {
            _scancodes.push_back(0x1C);
            break;
        }
        case SDL_SCANCODE_A:
        {
            _scancodes.push_back(0x1E);
            break;
        }
        case SDL_SCANCODE_B:
        {
            _scancodes.push_back(0x30);
            break;
        }
        case SDL_SCANCODE_C:
        {
            _scancodes.push_back(0x2E);
            break;
        }
        case SDL_SCANCODE_D:
        {
            _scancodes.push_back(0x20);
            break;
        }
        case SDL_SCANCODE_E:
        {
            _scancodes.push_back(0x12);
            break;
        }
        case SDL_SCANCODE_F:
        {
            _scancodes.push_back(0x21);
            break;
        }
        case SDL_SCANCODE_G:
        {
            _scancodes.push_back(0x22);
            break;
        }
        case SDL_SCANCODE_H:
        {
            _scancodes.push_back(0x23);
            break;
        }
        case SDL_SCANCODE_I:
        {
            _scancodes.push_back(0x17);
            break;
        }
        case SDL_SCANCODE_J:
        {
            _scancodes.push_back(0x24);
            break;
        }
        case SDL_SCANCODE_K:
        {
            _scancodes.push_back(0x25);
            break;
        }
        case SDL_SCANCODE_L:
        {
            _scancodes.push_back(0x26);
            break;
        }
        case SDL_SCANCODE_M:
        {
            _scancodes.push_back(0x32);
            break;
        }
        case SDL_SCANCODE_N:
        {
            _scancodes.push_back(0x31);
            break;
        }
        case SDL_SCANCODE_O:
        {
            _scancodes.push_back(0x18);
            break;
        }
        case SDL_SCANCODE_P:
        {
            _scancodes.push_back(0x19);
            break;
        }
        case SDL_SCANCODE_Q:
        {
            _scancodes.push_back(0x10);
            break;
        }
        case SDL_SCANCODE_R:
        {
            _scancodes.push_back(0x13);
            break;
        }
        case SDL_SCANCODE_S:
        {
            _scancodes.push_back(0x1F);
            break;
        }
        case SDL_SCANCODE_T:
        {
            _scancodes.push_back(0x14);
            break;
        }
        case SDL_SCANCODE_U:
        {
            _scancodes.push_back(0x16);
            break;
        }
        case SDL_SCANCODE_V:
        {
            _scancodes.push_back(0x2F);
            break;
        }
        case SDL_SCANCODE_W:
        {
            _scancodes.push_back(0x11);
            break;
        }
        case SDL_SCANCODE_X:
        {
            _scancodes.push_back(0x2D);
            break;
        }
        case SDL_SCANCODE_Y:
        {
            _scancodes.push_back(0x15);
            break;
        }
        case SDL_SCANCODE_Z:
        {
            _scancodes.push_back(0x2C);
            break;
        }
        case SDL_SCANCODE_0:
        {
            _scancodes.push_back(0x0B);
            break;
        }
        case SDL_SCANCODE_1:
        {
            _scancodes.push_back(0x02);
            break;
        }
        case SDL_SCANCODE_2:
        {
            _scancodes.push_back(0x03);
            break;
        }
        case SDL_SCANCODE_3:
        {
            _scancodes.push_back(0x04);
            break;
        }
        case SDL_SCANCODE_4:
        {
            _scancodes.push_back(0x05);
            break;
        }
        case SDL_SCANCODE_5:
        {
            _scancodes.push_back(0x06);
            break;
        }
        case SDL_SCANCODE_6:
        {
            _scancodes.push_back(0x07);
            break;
        }
        case SDL_SCANCODE_7:
        {
            _scancodes.push_back(0x08);
            break;
        }
        case SDL_SCANCODE_8:
        {
            _scancodes.push_back(0x09);
            break;
        }
        case SDL_SCANCODE_9:
        {
            _scancodes.push_back(0x0A);
            break;
        }
        case SDL_SCANCODE_GRAVE:
        {
            _scancodes.push_back(0x29);
            break;
        }
        case SDL_SCANCODE_MINUS:
        {
            _scancodes.push_back(0x0C);
            break;
        }
        case SDL_SCANCODE_EQUALS:
        {
            _scancodes.push_back(0x0D);
            break;
        }
        case SDL_SCANCODE_BACKSLASH:
        {
            _scancodes.push_back(0x2B);
            break;
        }
        case SDL_SCANCODE_BACKSPACE:
        {
            _scancodes.push_back(0x0E);
            break;
        }
        case SDL_SCANCODE_SPACE:
        {
            _scancodes.push_back(0x39);
            break;
        }
        case SDL_SCANCODE_TAB:
        {
            _scancodes.push_back(0x0F);
            break;
        }
        case SDL_SCANCODE_CAPSLOCK:
        {
            _scancodes.push_back(0x3A);
            break;
        }
        case SDL_SCANCODE_LSHIFT:
        {
            _scancodes.push_back(0x2A);
            break;
        }
        case SDL_SCANCODE_LCTRL:
        {
            _scancodes.push_back(0x1D);
            break;
        }
        case SDL_SCANCODE_LALT:
        {
            _scancodes.push_back(0x38);
            break;
        }
        case SDL_SCANCODE_F1:
        {
            _scancodes.push_back(0x3b);
            break;
        }
        case SDL_SCANCODE_F2:
        {
            _scancodes.push_back(0x3c);
            break;
        }
        case SDL_SCANCODE_F3:
        {
            _scancodes.push_back(0x3d);
            break;
        }
        case SDL_SCANCODE_F4:
        {
            _scancodes.push_back(0x3e);
            break;
        }
        case SDL_SCANCODE_F5:
        {
            _scancodes.push_back(0x3f);
            break;
        }
        case SDL_SCANCODE_F6:
        {
            _scancodes.push_back(0x40);
            break;
        }
        case SDL_SCANCODE_F7:
        {
            _scancodes.push_back(0x41);
            break;
        }
        case SDL_SCANCODE_F8:
        {
            _scancodes.push_back(0x42);
            break;
        }
        case SDL_SCANCODE_F9:
        {
            _scancodes.push_back(0x43);
            break;
        }
        case SDL_SCANCODE_F10:
        {
            _scancodes.push_back(0x44);
            break;
        }
        case SDL_SCANCODE_F11:
        {
            _scancodes.push_back(0x57);
            break;
        }
        case SDL_SCANCODE_F12:
        {
            _scancodes.push_back(0x58);
            break;
        }
        case SDL_SCANCODE_UP:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x48);
            break;
        }
        case SDL_SCANCODE_DOWN:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x50);
            break;
        }
        case SDL_SCANCODE_LEFT:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x4B);
            break;
        }
        case SDL_SCANCODE_RIGHT:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x4D);
            break;
        }
        case SDL_SCANCODE_PERIOD:
        {
            _scancodes.push_back(0x34);
            break;
        }
        case SDL_SCANCODE_INSERT:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x52);
            break;
        }
        case SDL_SCANCODE_HOME:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x47);
            break;
        }
        case SDL_SCANCODE_PAGEUP:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x49);
            break;
        }
        case SDL_SCANCODE_DELETE:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x53);
            break;
        }
        case SDL_SCANCODE_END:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x4F);
            break;
        }
        case SDL_SCANCODE_PAGEDOWN:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x51);
            break;
        }
        case SDL_SCANCODE_RCTRL:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x1D);
            break;
        }
        case SDL_SCANCODE_RGUI:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x5C);

            break;
        }
        case SDL_SCANCODE_RALT:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x38);

            break;
        }
        case SDL_SCANCODE_APPLICATION:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x5D);
            break;
        }
        case SDL_SCANCODE_LEFTBRACKET:
        {
            _scancodes.push_back(0x1A);
            break;
        }
        case SDL_SCANCODE_RIGHTBRACKET:
        {
            _scancodes.push_back(0x1B);
            break;
        }
        case SDL_SCANCODE_SEMICOLON:
        {
            _scancodes.push_back(0x27);
            break;
        }
        case SDL_SCANCODE_COMMA:
        {
            _scancodes.push_back(0x33);
            break;
        }
        case SDL_SCANCODE_SLASH:
        {
            _scancodes.push_back(0x35);
            break;
        }
        case SDL_SCANCODE_APOSTROPHE:
        {
            _scancodes.push_back(0x28);
            break;
        }
        case SDL_SCANCODE_NUMLOCKCLEAR:
        {
            _scancodes.push_back(0x45);
            break;
        }
        }

        if (event.type == SDL_EVENT_KEY_UP && !_scancodes.empty())
        {
            _scancodes.back() += 0x80;
        }

        uint8_t ppi_port_b{ _ppi.read(std::to_underlying<PPI::Port>(PPI::Port::B), true) };

        if ((ppi_port_b & PPI::b_keyboard_disable) == 0)
        {
            if (!_scancodes.empty())
            {
                _ppi.port_a = _scancodes.back();
                _scancodes.pop_back();
            }

            _pic.raise(PIC::IRQ::Keyboard);
        }
    }
}