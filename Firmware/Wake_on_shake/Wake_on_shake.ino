#define ADXL_REG_WRITE      0x0A
#define ADXL_REG_READ       0x0B
#define ADXL_FIFO_READ      0x0D
#define ADXL_CS             4

#define 

void setup() 
{
  configureUSART();
  configureINT0();
  configureSPI();
  sei();
  MCUCR |= ((1<<SM1)|(1<<SM0)|(1<<SE));
  PORTB |= (1<<ADXL_CS);
}

void loop() 
{
  asm("sleep":::);
  configureUSART();
  char data[8];
  char poop[5] = "poop";
  int a = 12345;
  //itoa(a, data, 10);
  USARTXMit(poop);
  PORTB &= !(1<<ADXL_CS);
  xferSPI(ADXL_REG_READ);
  xferSPI(0);
  itoa((int)xferSPI(0), data, 16);
  PORTB |= (1<<ADXL_CS);
  USARTXMit(data);
}

void configureUSART()
{
  UBRRH =  0;
  UBRRL = 25;
  UCSRB |= ( (1<<RXEN)|(1<<TXEN) );
}

void configureINT0()
{
  MCUCR &= B11111100;
  GIMSK |= (1<<INT0);
}

void configureSPI()
{
  USICR = B00010000;
  DDRB |= ( (1<<7)|(1<<6)|(1<<4) );
}

byte xferSPI(byte data)
{
  USIDR = data;
  USISR = (1<<USIOIF);
  for (byte i = 0; i <= 15; i++)
  {
    USICR = (1<<USIWM0)|(1<<USICS1)|(1<<USICLK)|(1<<USITC);
  }
  return USIDR;
}

void USARTXMit(char *data)
{
  byte data_index = 0;
  while (*(data+data_index) != '\0')
  {
    TXB = *(data+data_index);
    while ((UCSRA & B01000000) == 0);
    UCSRA |= B01000000;
    data_index++;
  }
}

char USARTBlockingReceive()
{
  while ((UCSRA & B10000000) == 0);
  return RXB;
}

ISR(INT0_vect)
{
}
