#include <avr/io.h>
#include <util/delay.h>

#include "softserial.h"

void softserial_init(void)
{
	// set tx-pin as output and set it high
	TX_DDR |= (1 << TX_BIT);
	TX_PORT |= (1 << TX_BIT);

	// set RX-pin as input and enable pull-up
	RX_DDR &= ~(1 << RX_BIT);
	RX_PORT |= (1 << RX_BIT);
}

static inline void serial_send_1()
{
	TX_PORT |= (1 << TX_BIT);
	_delay_us(SERIAL_BIT_TIME_US);
}

static inline void serial_send_0()
{
	TX_PORT &= ~(1 << TX_BIT);
	_delay_us(SERIAL_BIT_TIME_US);
}

void softserial_send(uint8_t ch)
{
	serial_send_0(); // start-bit

	for(register char i = 0; i < 8; i++) {
		if(ch & 1)
			serial_send_1();
		else
			serial_send_0();
		ch >>= 1;
	}

	serial_send_1(); // stop-bit
}

uint8_t softserial_recv()
{
	return(0);
}
