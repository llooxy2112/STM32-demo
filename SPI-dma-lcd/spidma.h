int spi_init(SPI_TypeDef* SPIx);
int spi_read_write(SPI_TypeDef* SPIx,uint8_t *rbuf, const uint8_t *wbuf, int cnt, uint16_t speed);
int spi_read_write16(SPI_TypeDef* SPIx,uint16_t *rbuf, const uint16_t *wbuf, int cnt, uint16_t speed);
