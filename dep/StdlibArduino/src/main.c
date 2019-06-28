#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>

int var0 = 0, var1 = 0, var2 = 0, var3 = 0, var4 = 0, var5 = 0, var6 = 0, var7 = 0, var8 = 0, var9 = 0, 
    var10 = 0, var11 = 0, var12 = 0, var13 = 0, var14 = 0, var15 = 0, var16 = 0, var17 = 0, var18 = 0, var19 = 0,
    var20 = 0, var21 = 0, var22 = 0, var23 = 0, var24 = 0, var25 = 0, var26 = 0, var27 = 0, var28 = 0, var29 = 0,
    var30 = 0, var31 = 0, var32 = 0, var33 = 0, var34 = 0, var35 = 0, var36 = 0, var37 = 0, var38 = 0, var39 = 0,
    var40 = 0, var41 = 0, var42 = 0, var43 = 0, var44 = 0, var45 = 0, var46 = 0, var47 = 0, var48 = 0, var49 = 0,
    var50 = 0, var51 = 0, var52 = 0, var53 = 0, var54 = 0, var55 = 0, var56 = 0, var57 = 0, var58 = 0, var59 = 0,
    var60 = 0, var61 = 0, var62 = 0, var63 = 0;

#define BAUD 9600
#define BAUD_REGISTER_VALUE (F_CPU / 16 / BAUD - 1)

int main()
{
	// init ports
	// D2...D5 (PD2...PD5): outputs set to low
	DDRD = (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5);
	// D6...D7 (PD6...PD7): inputs with pull-up
	PORTD = (1 << PD6) | (1 << PD7);
	// D8...D13 (PB0...PB5): outputs set to low
	DDRB = (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5);
	// A0...A5 (PC0...PC5): inputs with pull-up and optional ADC
	PORTC = (1 << PC0) | (1 << PC1) | (1 << PC2) | (1 << PC3) | (1 << PC4) | (1 << PC5);

	// init adc
	ADMUX = (0 << REFS1) | (1 << REFS0) |  // VCC is voltage reference
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
	
	// init UART
	UBRR0H = (uint8_t) (BAUD_REGISTER_VALUE >> 8);
	UBRR0L = (uint8_t) (BAUD_REGISTER_VALUE & 0xff);	
	UCSR0C = (0 << UMSEL01) | (0 << UMSEL00) | // asynchronous USART
		(0 << UPM01) | (0 << UPM00) | // no parity
		(0 << USBS0) | // one stop-bit
		(1 << UCSZ01) | (1 << UCSZ00); // (0)11 -> 8-bit mode
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// init servo
	ICR1 = 0xffff; // top value
	TCCR1A = (0 << COM1A1) | (0 << COM1A0) | // disable for now, prepared for clear on compare match (PB1=D9)
		(0 << COM1B1) | (0 << COM1B0) | // disable for now, prepared for clear on compare match (PB2=D10)
		(1 << WGM11) | (0 << WGM10); // fast pwm, top is ICR1
	TCCR1B = (1 << WGM13) | (0 << WGM12) | // see WGM11/0 above
		(0 << CS12) | (1 << CS11) | (0 << CS10); // clk/8

	// init pwm
	TCCR2A = (0 << COM2B1) | (0 << COM2B0) | // disable for now, prepared for clear on compare match (PD3=D3)
		(0 << COM2A1) | (0 << COM2A0) | // disable for now, prepared for clear on compare match (PB3=D11)
		(1 << WGM21) | (1 << WGM20); // fast pwm mode, top is 0xff
	TCCR2B = (0 << WGM22) | // see WGM21/20 above
		(1 << CS22) | (0 << CS21) | (1 << CS20); // clk/128 and starts pwm

	// label this position to include the call of user main later
	asm ("call_user_main:");
	asm volatile ("nop");
	asm volatile ("nop");
	asm volatile ("icall");

	return(0);
}

int __mulhi3(int a, int b) // we provide a function for multiplication (equal to ATTiny)
{
	return(a * b);
}

void serial_send(int ch)
{
	while(!(UCSR0A & (1 << UDRE0))) // wait for transmitter to become ready
		;
	UDR0 = (uint8_t) ch;
}

int serial_recv()
{
	while(!((UCSR0A) & (1 << RXC0)))
		;
	
	return((int) UDR0);
}

int serial_recv_if_avail()
{
	if(!((UCSR0A) & (1 << RXC0)))
		return(0);
	
	return((int) UDR0);
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
	return(serial_recv_if_avail());
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

int adc_read(int channel)
{
	PORTC &= ~(1 << channel); // disable pull-up
	ADMUX = (ADMUX & ~0x0f) | ((uint8_t) channel & 0xf); // select channel
	
	ADCSRA |= (1 << ADSC); // start conversion
	while((ADCSRA & (1 << ADIF)) == 0) // and wait for conversion to complete
		;

	PORTC |= 1 << channel; // and enable pull-up again
		
	return(ADCW);
}

void servo_output(int channel, unsigned int value)
{
   value = (value * 20) + 2000; // map 0...100 to 2000...4000
   
   if(channel == 0) { // PB1=D9
      OCR1A = value;
      TCCR1A |= (1 << COM1A1); // clear on compare match
   } else if(channel == 1) { // PB2=D10
      OCR1B = value;
      TCCR1A |= (1 << COM1B1); // clear on compare match
   }
}

void pwm_output(int channel, unsigned int value)
{
   if(channel == 0) { // PD3=D3
      OCR2B = value;
      TCCR2A |= (1 << COM2B1);
   } else if(channel == 1) { // PB3=D11
      OCR2A = value;
      TCCR2A |= (1 << COM2A1);
   }
}


