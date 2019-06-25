#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

#include "softserial.h"

int var0 = 0;
int var1 = 0;
int var2 = 0;
int var3 = 0;
int var4 = 0;
int var5 = 0;
int var6 = 0;
int var7 = 0;
int var8 = 0;
int var9 = 0;
int var10 = 0;
int var11 = 0;
int var12 = 0;
int var13 = 0;
int var14 = 0;
int var15 = 0;
int var16 = 0;
int var17 = 0;
int var18 = 0;
int var19 = 0;
int var20 = 0;
int var21 = 0;
int var22 = 0;
int var23 = 0;
int var24 = 0;
int var25 = 0;
int var26 = 0;
int var27 = 0;
int var28 = 0;
int var29 = 0;
int var30 = 0;
int var31 = 0;

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
	
	DDRA = (1 << 0) | (1 << 3) | (1 << 7);
	DDRB = (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3) | (1 << 5);
	PORTA |= (1 << 6) | (1 << 4); // enable pull-ups
	PORTB |= (1 << 6) | (1 << 4);

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
	TCCR1A = 0;  
	TCCR1B = ( 1 << CS13 );  // dto., prescaler 128  start Timer
	TCCR1C = 0;						//  präventiv löschen
	TCCR1D = 0;						// WGM11,10 = 0 -> Fast PWM
	TCCR1E = 0;						//  präventiv löschen
	PLLCSR = 0;						//  präventiv löschen
	TC1H = 0x03;// Top Value für Timer 1 auf 0x03FF setzen, mehr geht nicht
	OCR1C = 0xFF; 
	
	TCCR1A |= (1<<COM1A1 | 1<<PWM1A); 
	DDRB |= (1 << PB1);
	TCCR1A |= (1<<COM1B1 | 1<<PWM1B); 
	DDRB |= (1 << PB3);
	TCCR1C |= (1<<COM1D1 | 1<<PWM1D); 
	DDRB |= (1 << PB5);
}

#define SERVO_OFFSET 66 // value of 1ms pulse
void servo_set(int servonum, int position)
{
	if(position < 0)
		position = 0;
	if(position > 1023)
		position = 1023;
	
	position = position / 18 + SERVO_OFFSET;
	
	TC1H = (position >> 8);  // Highbyte ins temporäre High-Register
    if (servonum == 0) { 
       	OCR1A = position;   // lädt nur Lowbyte rein, weil 8 Bit Register
	}
	if (servonum == 1) {
        OCR1B = position;   // lädt nur Lowbyte rein, weil 8 Bit Register
	} 
	if (servonum == 2) {
        OCR1D = position;   // lädt nur Lowbyte rein, weil 8 Bit Register
	} 
}

void pwm_init()
{
	TCCR1A = 0;  
	TCCR1B = ( 1 << CS12 );  // dto., prescaler 8  start Timer
	TCCR1C = 0;						//  präventiv löschen
	TCCR1D = 0;						// WGM11,10 = 0 -> Fast PWM
	TCCR1E = 0;						//  präventiv löschen
	PLLCSR = 0;						//  präventiv löschen
	TC1H = 0x03;// Top Value für Timer 1 auf 0x03FF setzen, mehr geht nicht
	OCR1C = 0xFF;

	// enable all pwm-channels
	TCCR1A |= (1<<COM1A1 | 1<<PWM1A); 
	DDRB |= (1 << PB1);
	TCCR1A |= (1<<COM1B1 | 1<<PWM1B); 
	DDRB |= (1 << PB3);
	TCCR1C |= (1<<COM1D1 | 1<<PWM1D); 
	DDRB |= (1 << PB5);
}

void pwm_set(int channel, int value)
{	
	TC1H = (value >> 8); 	 // Highbyte ins temporäre High-Register
    if(channel == 0) { 
       	OCR1A = value; 		  // lädt nur Lowbyte rein, weil 8 Bit Register
	}
	if(channel == 1) {
        OCR1B = value;   // lädt nur Lowbyte rein, weil 8 Bit Register
	} 
	if(channel == 2) {
        OCR1D = value;   // lädt nur Lowbyte rein, weil 8 Bit Register
	}
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
