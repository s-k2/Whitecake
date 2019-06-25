#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "softserial.h"

int var0 = 0, var1 = 0, var2 = 0, var3 = 0, var4 = 0, var5 = 0, var6 = 0, var7 = 0, var8 = 0, var9 = 0, 
    var10 = 0, var11 = 0, var12 = 0, var13 = 0, var14 = 0, var15 = 0, var16 = 0, var17 = 0, var18 = 0, var19 = 0,
    var20 = 0, var21 = 0, var22 = 0, var23 = 0, var24 = 0, var25 = 0, var26 = 0, var27 = 0, var28 = 0, var29 = 0,
    var30 = 0, var31 = 0, var32 = 0, var33 = 0, var34 = 0, var35 = 0, var36 = 0, var37 = 0, var38 = 0, var39 = 0,
    var40 = 0, var41 = 0, var42 = 0, var43 = 0, var44 = 0, var45 = 0, var46 = 0, var47 = 0, var48 = 0, var49 = 0,
    var50 = 0, var51 = 0, var52 = 0, var53 = 0, var54 = 0, var55 = 0, var56 = 0, var57 = 0, var58 = 0, var59 = 0,
    var60 = 0, var61 = 0, var62 = 0, var63 = 0;

int main()
{
	// tmp
	// TODO: enable input and output-pins
	// end tmp

	ADMUX = (0 << REFS1) | (0 << REFS0) |  // VCC is voltage reference
		(0 << ADLAR) | // right justified output
		0; // Analog channel 0
	ADCSRA = (1 << ADEN) | // enable ADC
		(0 << ADSC) | // don't start conversion yet
		(0 << ADATE) | // disable ADC auto-trigger
		(0 << ADIF) | // unset interrupt flag
		(0 << ADIE) | // disable ADC interrupt
		(1 << ADPS2) |
		(1 << ADPS1) |
		(1 << ADPS0); // prescalar (111 = 128) -> ok for 8Mhz and 16Mhz
	// leave ADCSRB to 0 -> no bipolar input, no gain, (no internal reference selection), no auto-trigger source (= free-running mode)
	// DIDR0 and DIDR1 what about disabling digital input for adc-pins???
	
	
	DDRA |= (1 << 0) | (1 << 1) | (1 << 2);
	DDRB = (1 << 0);
	PORTA |= (1 << 6) | (1 << 7); // enable pull-ups

	serial_init();
	
	var0 = var1 * var2; // dummy instruction to force the generation of __mulhi3
	
	// label this position to include the call of user main later
	asm ("call_user_main:");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("icall");

	return(0);
}

void print_number(int16_t num)
{
	if(num < 0) {
		serial_send('-');
		num = -num;
	}
	
	char buffer[5]; // MAX_INT = 32767 -> 5 digits
	
	register uint8_t bufferPos = 0;
	do {
		buffer[bufferPos++] = '0' + num % 10;
		num = num / 10;
	} while(num != 0);
	
	while(bufferPos > 0) {
		serial_send(buffer[bufferPos - 1]);
		bufferPos--;
	}
}

void print_str(int16_t ptr)
{
	register uint8_t ch = pgm_read_byte_near(ptr);
	while(ch != 0x00) {
		serial_send(ch);
		ptr++;
		ch = pgm_read_byte_near(ptr);
	}
}

void print_newline()
{
	serial_send('\r');
	serial_send('\n');
}

int adc_read(int channel)
{
	ADMUX = (ADMUX & ~0x0f) | ((uint8_t) channel & 0xf); // select channel
	
	ADCSRA |= (1 << ADSC); // start conversion
	
	while((ADCSRA & (1 << ADIF)) == 0) // wait for conversion to complete
		;
		
	return(ADCW);
}

void servo_init()
{
}

void servo_set(int num, char value)
{
}

void pwm_init()
{
}

void pwm_set(int num, int value)
{
}

void print_char(int ch)
{
	serial_send((uint8_t) ch);
}

int waitchar()
{
	return(serial_recv());
}

int recvchar()
{
	return(serial_recv());
}

void waitms(uint16_t ms)
{
	while(ms > 0) {
		_delay_ms(1);
		ms--;
	}
}

void waitus(uint16_t us)
{
	while(us > 0) {
		_delay_us(1);
		us--;
	}
}

