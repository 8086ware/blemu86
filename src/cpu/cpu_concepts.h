#pragma once
#include <concepts>

template<typename T> concept CPU_Width = std::same_as<T, uint16_t> || std::same_as<T, uint8_t>;
