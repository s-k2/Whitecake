#include <avr/io.h>

#ifndef SOFTSERIAL_H
#define SOFTSERIAL_H

#if F_CPU != 8000000
#error "You must manually adjust the prescalers for clock-rates != 8 MHz"
#endif

#define SERIAL_BIT_TIME_US 416 // = 1s / 2400baud

// tinyTick-config
#define TX_DDR DDRB
#define TX_PORT PORTB
#define TX_BIT PB4

#define RX_DDR DDRB
#define RX_PIN PINB
#define RX_PORT PORTB
#define RX_BIT PB6


extern void softserial_init(void);
extern void softserial_send(uint8_t ch);
extern uint8_t softserial_recv();

#define serial_init softserial_init
#define serial_send softserial_send
#define serial_recv softserial_recv

#endif /* SOFTSERIAL_H */