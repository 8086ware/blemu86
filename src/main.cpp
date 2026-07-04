#include "blemu86.h"
#include <memory>

int main()
{
	auto emulator{ std::make_unique<Blemu86>("pcxtbios.bin")};
	while (emulator->loop());
}