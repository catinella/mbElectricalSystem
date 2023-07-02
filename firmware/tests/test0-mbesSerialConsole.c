#include <mbesSerialConsole.h>

int main() {
	USART_Init(0);
	USART_writeChar('+');
	USART_writeString("Hello world\n");

	return(0);
}
