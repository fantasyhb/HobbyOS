/* console.h */

#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#define SCR_WIDTH 80
#define SCR_SIZE (80*25)

typedef struct s_console
{
     u32 current_start_addr;
     u32 origin_addr;
     u32 v_mem_limit;
     u32 cursor;
}CONSOLE;

#define DEFAULT_CHAR_COLOR 0x07

#endif
