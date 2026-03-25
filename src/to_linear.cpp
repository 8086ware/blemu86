#include "to_linear.h"

int to_linear(uint16_t segment, uint16_t offset)
{
	int linear{ segment * 0x10 + offset }; // 20 bits

	if (linear >= 0x100000)
	{
		linear %= 0x100000; // wrap
	}

	return linear;
}