#define USART_BUFFER_SIZE 100

int uart_init(USART_TypeDef* USARTx, uint32_t baud, uint32_t flags);
int uart_close(USART_TypeDef* USARTx);
int uart_write(USART_TypeDef* USARTx, uint8_t *data, int nbytes);
