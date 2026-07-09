#include "keyboard.h"
#include <utility>
#include <print>

Keyboard::Keyboard(sf::Window& win, PPI& ppi, PIC& pic) : _ppi{ ppi }, _pic{ pic }, _win{ win }
{
    std::println("[XTKeyboard] Init...");
}

void Keyboard::cycle()
{
    const std::optional event{ _win.pollEvent() };

    if (event && (event->is<sf::Event::KeyPressed>() || event->is<sf::Event::KeyReleased>()))
    {
        sf::Keyboard::Scancode scancode{};

        if (event->is<sf::Event::KeyPressed>()) scancode = event->getIf<sf::Event::KeyPressed>()->scancode;
        if (event->is<sf::Event::KeyReleased>()) scancode = event->getIf<sf::Event::KeyReleased>()->scancode;

        switch (scancode)
        {
        case sf::Keyboard::Scancode::Escape:
        {
            _scancodes.push_back(0x1);
            break;
        }
        case sf::Keyboard::Scancode::Enter:
        {
            _scancodes.push_back(0x1C);
            break;
        }
        case sf::Keyboard::Scancode::A:
        {
            _scancodes.push_back(0x1E);
            break;
        }
        case sf::Keyboard::Scancode::B:
        {
            _scancodes.push_back(0x30);
            break;
        }
        case sf::Keyboard::Scancode::C:
        {
            _scancodes.push_back(0x2E);
            break;
        }
        case sf::Keyboard::Scancode::D:
        {
            _scancodes.push_back(0x20);
            break;
        }
        case sf::Keyboard::Scancode::E:
        {
            _scancodes.push_back(0x12);
            break;
        }
        case sf::Keyboard::Scancode::F:
        {
            _scancodes.push_back(0x21);
            break;
        }
        case sf::Keyboard::Scancode::G:
        {
            _scancodes.push_back(0x22);
            break;
        }
        case sf::Keyboard::Scancode::H:
        {
            _scancodes.push_back(0x23);
            break;
        }
        case sf::Keyboard::Scancode::I:
        {
            _scancodes.push_back(0x17);
            break;
        }
        case sf::Keyboard::Scancode::J:
        {
            _scancodes.push_back(0x24);
            break;
        }
        case sf::Keyboard::Scancode::K:
        {
            _scancodes.push_back(0x25);
            break;
        }
        case sf::Keyboard::Scancode::L:
        {
            _scancodes.push_back(0x26);
            break;
        }
        case sf::Keyboard::Scancode::M:
        {
            _scancodes.push_back(0x32);
            break;
        }
        case sf::Keyboard::Scancode::N:
        {
            _scancodes.push_back(0x31);
            break;
        }
        case sf::Keyboard::Scancode::O:
        {
            _scancodes.push_back(0x18);
            break;
        }
        case sf::Keyboard::Scancode::P:
        {
            _scancodes.push_back(0x19);
            break;
        }
        case sf::Keyboard::Scancode::Q:
        {
            _scancodes.push_back(0x10);
            break;
        }
        case sf::Keyboard::Scancode::R:
        {
            _scancodes.push_back(0x13);
            break;
        }
        case sf::Keyboard::Scancode::S:
        {
            _scancodes.push_back(0x1F);
            break;
        }
        case sf::Keyboard::Scancode::T:
        {
            _scancodes.push_back(0x14);
            break;
        }
        case sf::Keyboard::Scancode::U:
        {
            _scancodes.push_back(0x16);
            break;
        }
        case sf::Keyboard::Scancode::V:
        {
            _scancodes.push_back(0x2F);
            break;
        }
        case sf::Keyboard::Scancode::W:
        {
            _scancodes.push_back(0x11);
            break;
        }
        case sf::Keyboard::Scancode::X:
        {
            _scancodes.push_back(0x2D);
            break;
        }
        case sf::Keyboard::Scancode::Y:
        {
            _scancodes.push_back(0x15);
            break;
        }
        case sf::Keyboard::Scancode::Z:
        {
            _scancodes.push_back(0x2C);
            break;
        }
        case sf::Keyboard::Scancode::Num0:
        {
            _scancodes.push_back(0x0B);
            break;
        }
        case sf::Keyboard::Scancode::Num1:
        {
            _scancodes.push_back(0x02);
            break;
        }
        case sf::Keyboard::Scancode::Num2:
        {
            _scancodes.push_back(0x03);
            break;
        }
        case sf::Keyboard::Scancode::Num3:
        {
            _scancodes.push_back(0x04);
            break;
        }
        case sf::Keyboard::Scancode::Num4:
        {
            _scancodes.push_back(0x05);
            break;
        }
        case sf::Keyboard::Scancode::Num5:
        {
            _scancodes.push_back(0x06);
            break;
        }
        case sf::Keyboard::Scancode::Num6:
        {
            _scancodes.push_back(0x07);
            break;
        }
        case sf::Keyboard::Scancode::Num7:
        {
            _scancodes.push_back(0x08);
            break;
        }
        case sf::Keyboard::Scancode::Num8:
        {
            _scancodes.push_back(0x09);
            break;
        }
        case sf::Keyboard::Scancode::Num9:
        {
            _scancodes.push_back(0x0A);
            break;
        }
        case sf::Keyboard::Scancode::Grave:
        {
            _scancodes.push_back(0x29);
            break;
        }
        case sf::Keyboard::Scancode::NumpadMinus:
        {
            _scancodes.push_back(0x0C);
            break;
        }
        case sf::Keyboard::Scancode::Equal:
        {
            _scancodes.push_back(0x0D);
            break;
        }
        case sf::Keyboard::Scancode::Backslash:
        {
            _scancodes.push_back(0x2B);
            break;
        }
        case sf::Keyboard::Scancode::Backspace:
        {
            _scancodes.push_back(0x0E);
            break;
        }
        case sf::Keyboard::Scancode::Space:
        {
            _scancodes.push_back(0x39);
            break;
        }
        case sf::Keyboard::Scancode::Tab:
        {
            _scancodes.push_back(0x0F);
            break;
        }
        case sf::Keyboard::Scancode::CapsLock:
        {
            _scancodes.push_back(0x3A);
            break;
        }
        case sf::Keyboard::Scancode::LShift:
        {
            _scancodes.push_back(0x2A);
            break;
        }
        case sf::Keyboard::Scancode::LControl:
        {
            _scancodes.push_back(0x1D);
            break;
        }
        case sf::Keyboard::Scancode::LAlt:
        {
            _scancodes.push_back(0x38);
            break;
        }
        case sf::Keyboard::Scancode::F1:
        {
            _scancodes.push_back(0x3b);
            break;
        }
        case sf::Keyboard::Scancode::F2:
        {
            _scancodes.push_back(0x3c);
            break;
        }
        case sf::Keyboard::Scancode::F3:
        {
            _scancodes.push_back(0x3d);
            break;
        }
        case sf::Keyboard::Scancode::F4:
        {
            _scancodes.push_back(0x3e);
            break;
        }
        case sf::Keyboard::Scancode::F5:
        {
            _scancodes.push_back(0x3f);
            break;
        }
        case sf::Keyboard::Scancode::F6:
        {
            _scancodes.push_back(0x40);
            break;
        }
        case sf::Keyboard::Scancode::F7:
        {
            _scancodes.push_back(0x41);
            break;
        }
        case sf::Keyboard::Scancode::F8:
        {
            _scancodes.push_back(0x42);
            break;
        }
        case sf::Keyboard::Scancode::F9:
        {
            _scancodes.push_back(0x43);
            break;
        }
        case sf::Keyboard::Scancode::F10:
        {
            _scancodes.push_back(0x44);
            break;
        }
        case sf::Keyboard::Scancode::F11:
        {
            _scancodes.push_back(0x57);
            break;
        }
        case sf::Keyboard::Scancode::F12:
        {
            _scancodes.push_back(0x58);
            break;
        }
        case sf::Keyboard::Scancode::Up:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x48);
            break;
        }
        case sf::Keyboard::Scancode::Down:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x50);
            break;
        }
        case sf::Keyboard::Scancode::Left:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x4B);
            break;
        }
        case sf::Keyboard::Scancode::Right:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x4D);
            break;
        }
        case sf::Keyboard::Scancode::Period:
        {
            _scancodes.push_back(0x34);
            break;
        }
        case sf::Keyboard::Scancode::Insert:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x52);
            break;
        }
        case sf::Keyboard::Scancode::Home:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x47);
            break;
        }
        case sf::Keyboard::Scancode::PageUp:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x49);
            break;
        }
        case sf::Keyboard::Scancode::Delete:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x53);
            break;
        }
        case sf::Keyboard::Scancode::End:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x4F);
            break;
        }
        case sf::Keyboard::Scancode::PageDown:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x51);
            break;
        }
        case sf::Keyboard::Scancode::RControl:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x1D);
            break;
        }
        case sf::Keyboard::Scancode::RSystem:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x5C);

            break;
        }
        case sf::Keyboard::Scancode::RAlt:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x38);

            break;
        }
        case sf::Keyboard::Scancode::Application:
        {
            _scancodes.push_back(0xE0);
            _scancodes.push_back(0x5D);
            break;
        }
        case sf::Keyboard::Scancode::LBracket:
        {
            _scancodes.push_back(0x1A);
            break;
        }
        case sf::Keyboard::Scancode::RBracket:
        {
            _scancodes.push_back(0x1B);
            break;
        }
        case sf::Keyboard::Scancode::Semicolon:
        {
            _scancodes.push_back(0x27);
            break;
        }
        case sf::Keyboard::Scancode::Comma:
        {
            _scancodes.push_back(0x33);
            break;
        }
        case sf::Keyboard::Scancode::Slash:
        {
            _scancodes.push_back(0x35);
            break;
        }
        case sf::Keyboard::Scancode::Apostrophe:
        {
            _scancodes.push_back(0x28);
            break;
        }
        case sf::Keyboard::Scancode::NumLock:
        {
            _scancodes.push_back(0x45);
            break;
        }
        }

        if (event->is<sf::Event::KeyReleased>() && !_scancodes.empty())
        {
            _scancodes.back() += 0x80;
        }

        std::optional<uint8_t> ppi_port_b{ _ppi.read(std::to_underlying(PPI::Port::B), true) };

        if ((ppi_port_b.value() & PPI::b_keyboard_disable) == 0)
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