/* tty.h */

#ifndef _TTY_H_
#define _TTY_H_

#define TTY_IN_BYTES 256

struct s_console;

typedef struct s_tty
{
     u32 in_buf[TTY_IN_BYTES];
     u32* p_head;
     u32* p_tail;
     int buf_count;

     struct s_console *p_console;
}TTY;



#endif
