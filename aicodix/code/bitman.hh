/*
Bit manipulation of byte arrays

Copyright 2018 Ahmet Inan <inan@aicodix.de>
*/

#pragma once

namespace CODE {

static inline void xor_be_bit(uint8_t *buf, int pos, bool val)
{
	buf[pos/8] ^= val<<(7-pos%8);
}

static inline void xor_le_bit(uint8_t *buf, int pos, bool val)
{
	buf[pos/8] ^= val<<(pos%8);
}

static inline void set_be_bit(uint8_t *buf, int pos, bool val)
{
	buf[pos/8] = (~(1<<(7-pos%8))&buf[pos/8])|(val<<(7-pos%8));
}

static inline void set_le_bit(uint8_t *buf, int pos, bool val)
{
	buf[pos/8] = (~(1<<(pos%8))&buf[pos/8])|(val<<(pos%8));
}

static inline bool get_be_bit(const uint8_t *buf, int pos)
{
	return (buf[pos/8]>>(7-pos%8))&1;
}

static inline bool get_le_bit(const uint8_t *buf, int pos)
{
	return (buf[pos/8]>>(pos%8))&1;
}

}

