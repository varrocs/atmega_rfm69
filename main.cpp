#include "constants.h"
#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "uart.h"

int main(void) {

	uart_init();
    	stdout = &uart_output;
        stdin  = &uart_input;

	DDRB = 0xFF; // PORTB is output, all pins
	PORTB = 0x00; // Make pins low to start

//	printf("Started\n");

	for (;;) {
		PORTB ^= 0xFF; // invert all the pins
		_delay_ms(1000); // wait some time
		uart_putchar('A', NULL);
		for (int i='A'; i<'Z'; i++) {
		uart_putchar(i, NULL);
		}
//		printf("tikk\n");
	}
	return 0;
}
