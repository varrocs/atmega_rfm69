#pragma once

extern FILE uart_output;
extern FILE uart_input;
#ifdef __cplusplus
extern "C" {
#endif
	int uart_putchar(char c, FILE *stream);
	int uart_getchar(FILE *stream);

	void uart_init(void);

#ifdef __cplusplus
}
#endif
