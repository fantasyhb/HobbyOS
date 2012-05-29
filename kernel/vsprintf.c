/* vsprintf.c */
#include "type.h"
#include "const.h"
#include "string.h"

int vsprintf(char *buf, const char*fmt, va_list arg)
{
     char *p;
     char tmp[256];
     va_list p_next_arg = arg;

     for (p=buf;*fmt!=0; fmt++)
     {
	  if (*fmt != '%')
	  {
	       *p++ = *fmt;
	       continue;
	  }
	  fmt++;

	  switch (*fmt)
	  {
	  case 'x':
	       itoa_hex(tmp, *((int*)p_next_arg));
	       strcpy(p, tmp);
	       p_next_arg += 4;
	       p += strlen(tmp);
	       break;
	  case 's':
	       break;
	  default:
	       break;
	  }
     }

     return (p-buf);
}
