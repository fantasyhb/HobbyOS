/* printf.c */
#include "type.h"
#include "const.h"

int printf(const char *fmt, ... )
{
     int i ;
     char buf[STR_LEN];
     
     va_list arg = (va_list)((char*)(&fmt)+4);
          
     i = vsprintf(buf, fmt, arg);
     /* 
          write(buf, i);
 */
     printx(buf);
     return i;
}     
