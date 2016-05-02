#include "MakeI2CMessage.h"

// Calculate parity of a byte
uint8_t parityFromByte(uint8_t byte) {
	uint8_t p = 1;

	for (; byte; byte >>= 1)
		p ^= byte & 1;

	return p;
}

// Calculate parity of a word
uint8_t parityFromWord(uint16_t word) {
	uint8_t p = 1;

	for (; word; word >>= 1)
		p ^= word & 1;

	return p;
}

uint8_t makeReadMsg(uint8_t src, uint8_t port) {
	uint8_t ret = 0x00;
                                        // Bit 0 set to 0 for reading
	ret |= (src & 0x1) << 6;        // Bit 1 indicates read source
	ret |= (port & 0x3) << 4;       // Bits 2&3 indicate port
	ret |= parityFromByte(ret);     // Bit 7 is parity bit

	return ret;
}

uint16_t makeWriteMsg(uint8_t port, uint16_t pulseLen) {
	uint16_t ret = 0x0000;

	ret |= 0x8000;                  // Bit 0 set to 1 for writing
	ret |= (port & 0x3) << 12;      // Bits 2&3 are ID of port to write to
	ret |= (pulseLen & 0x7FF) << 1; // Write pulse length (truncated to 11 bits) to bits 4-14
	ret |= parityFromWord(ret);     // Bit 15 set to parity bit

	return ret;
}
