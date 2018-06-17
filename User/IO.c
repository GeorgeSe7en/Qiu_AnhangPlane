#ifndef _USART
#define _USART  0
#endif

#if _USART
#include "Driver_USART.h"
#else
#include "Driver_UART.h"
#endif


#if _USART
extern  ARM_DRIVER_USART Driver_USART1;

int stdout_putchar (int ch)
{
    char c;
    
    c = ch;
    if(1 == Driver_USART1.Send(&c, 1))
        return ch;
    else
        return -1;
}

int stdin_getchar (void) {
  int32_t ch=-1;
  char dat=-1;
  do {
    Driver_USART1.Recive(&dat, 1);
  } while (dat == -1);
  ch = dat;
  return (ch);
}

#else

extern  ARM_DRIVER_UART Driver_UART1;

int stdout_putchar (int ch)
{
    uint8_t c;
    
    c = ch;
    if(1 == Driver_UART1.WriteData(&c, 1))
        return ch;
    else
        return -1;
}

int stdin_getchar (void) {
  int32_t ch=-1;
  uint8_t dat;

  while(0==Driver_UART1.ReadData(&dat, 1));
  ch = stdout_putchar(dat);
  if(dat == '\r')  {
    stdout_putchar('\n');
  }
  else if(dat == '\b') {
    stdout_putchar(' ');
    stdout_putchar('\b');
  }
  
  return (ch);
}

#endif

#undef _USART

