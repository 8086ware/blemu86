#include "blemu86.h"
#include <memory>

int main()
{
	auto emulator{ std::make_unique<Blemu86>("GLABIOS_0.4.2_8X.ROM")};
	while (emulator->loop());
}