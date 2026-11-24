
template<typename T> uint8_t get_hibyte(T value)
{
	return static_cast<uint8_t>(value >> (8*sizeof(T) - 8));
}

template<typename T> uint8_t get_lobyte(T value)
{
	return static_cast<uint8_t>(value & 0xff);
}
