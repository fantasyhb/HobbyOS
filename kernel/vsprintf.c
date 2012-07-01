 /* vsprintf.c */
#include "type.h"
#include "const.h"
#include "string.h"

static char* i2a(int val, int base, char **ps)
{
     int m = val % base;
     int q = val / base;
     if (q){
	  i2a(q, base, ps);
     }
     **ps = ( m < 10 ) ? (m + '0') : (m - 10 + 'A');
     (*ps)++;
     return ps;
}

int vsprintf(char *buf, const char*fmt, va_list arg)
{
     char *p;
     char inner_buf[256];
     va_list p_next_arg = arg;
     int m;
     for (p=buf;*fmt!=0; fmt++)
     {
	  if (*fmt != '%')
	  {
	       *p++ = *fmt;
	       continue;
	  }
	  fmt++;
	  if (*fmt == '%')
	  {
	       *p++ = *fmt;
	       continue;
	  }
	  else{
	  }
	  
	  char *q = inner_buf;
	  memset(q, 0, sizeof(inner_buf));
	  
	  switch (*fmt)
	  {
	  case 'c':
	       *q++ = *((char*)p_next_arg);
	       p_next_arg += 4;
	       break;
	  case 'x':
	       m = *((int*)p_next_arg);
	       i2a(m, 16, &q);
	       p_next_arg += 4;
	       /* 
	       itoa_hex(inner_buf, *((int*)p_next_arg));
	       strcpy(p, inner_buf);
	       p_next_arg += 4;
	       p += strlen(inner_buf);
	        */
	       break;
	  case 's':
	       strcpy(q, (*((char**)p_next_arg)));
	       q += strlen(*((char**)p_next_arg));
	       p_next_arg += 4;
	       break;
	  case 'd':
	       m = *((int*)p_next_arg);
	       if (m < 0){
		    m = m * (-1);
		    *q++ = '-';
	       }
	       i2a(m, 10, &q);
	       p_next_arg += 4;
	       break;
	  default:
	       break;
	  }
	  q = inner_buf;
	  while (*q){
	       *p++ = *q++;
	  }
     }
     *p = 0;

     return (p-buf);
}
