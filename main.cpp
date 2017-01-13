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

	for (;;) {
		_delay_ms(100); // wait some time
		PORTB ^= 0xFF; // invert all the pins
		_delay_ms(100); // wait some time

		printf("Mukodik!!\n");
	}
	return 0;
}
