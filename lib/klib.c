#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"




void itoa_hex(char *str, int num)
{
  char *p = str;
  char ch;
  int i;
  *p++ = '0';
  *p++ = 'x';
  if (num == 0)
  {
    *p++ = '0';
    *p = 0;
    return;
  }
  int flag = 0;
  for (i=0; i<8; i++)
  {
    ch = (num >> (8-i-1)*4) & 0xf;
    if (flag || ch>0)
    {
      flag = 1;
      if (ch < 10)
      {
	ch = '0'+ ch;
      }
      else
      {
	ch = 'A' + ch - 10;
      }
      *p++ = ch;
    }
  }
  *p = 0;
}

void print_int(int num)
{
  char str[11];
  itoa_hex(str, num);
  print_str(str);
}

void delay(int time)
{
  int i, j;
  for (i=0; i<time*6; i++)
  {
      for (j=0; j<10000; j++)
      {
	
      }
  }
}
