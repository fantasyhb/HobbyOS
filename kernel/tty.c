/* tty.c  */
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"
#include "keyboard.h"

void init_tty(TTY *);
static void tty_do_read(TTY*);
static void tty_do_write(TTY*);
static void put_key(TTY *p_tty, u32 key);

void task_tty()
{
     TTY *p_tty;

     for (p_tty=tty_table; p_tty<tty_table+NR_CONSOLE; p_tty++)
     {
	  init_tty(p_tty);
     }
     /* 
     nr_current_console = 0;
      */
     select_console(0);
     while(1)
     {
	  for (p_tty=tty_table; p_tty<tty_table+NR_CONSOLE; p_tty++)
	  {
	       tty_do_read(p_tty);
	       tty_do_write(p_tty);
	  }
     }

}

void in_process(TTY * p_tty, u32 key)
{
     char output[2] = {'\0', '\0'};
     if ( (key & FLAG_EXT) == 0)
     {	/* printable key 
	  output[0] = key & 0xFF;
	  print_str(output);
	  disable_int();
	  out_byte(CRTC_ADDR_REG, CURSOR_H);
	  out_byte(CRTC_DATA_REG, ((disp_pos/2)>>8) & 0xFF);
	  out_byte(CRTC_ADDR_REG, CURSOR_L);
	  out_byte(CRTC_DATA_REG, ((disp_pos/2) & 0xFF));	
	  enable_int();*/
	  put_key(p_tty, key);
     }
     else
     {	/* function or control key  */
	  int raw_code = key & MASK_RAW;
	  switch (raw_code)
	  {
	  case ENTER:
	       put_key(p_tty, '\n');
	       break;
	  case BACKSPACE:
	       put_key(p_tty, '\b');
	       break;
	  case DOWN:
	       scroll_screen(p_tty->p_console, 1);
	       break;
	  case UP:
	       scroll_screen(p_tty->p_console, -1);
	       break;
	  case F1:
	  case F2:
	  case F3:
	  case F4:
	  case F5:
	  case F6:
	  case F7:
	  case F8:
	  case F9:
	  case F10:
	  case F11:
	  case F12:
	       if ( (key & FLAG_ALT_L) || (key & FLAG_ALT_R))
	       {
		    select_console(raw_code - F1);
	       }
	       break;
	  default:
	       break;
	  }
     }
}

void init_tty(TTY *p_tty)
{
     p_tty->buf_count = 0;
     p_tty->p_head = p_tty->p_tail = p_tty->in_buf;

     init_console(p_tty);
     /* 
     p_tty->p_console = console_table + (p_tty - tty_table);
      */
}

static void tty_do_read(TTY *p_tty)
{
     if (is_current_consle(p_tty->p_console))
     {
	  keyboard_read(p_tty);
     }
}

static void tty_do_write(TTY*p_tty)
{
     if (p_tty->buf_count>0)
     {
	  char ch = *(p_tty->p_tail);
	  p_tty->p_tail++;
	  if (p_tty->p_tail == p_tty->in_buf + TTY_IN_BYTES)
	  {
	       p_tty->p_tail = p_tty->in_buf;
	  }
	  p_tty->buf_count--;

	  out_char(p_tty->p_console, ch);
     }
     
}

int is_current_consle(CONSOLE *p_con)
{
     return (p_con == &console_table[nr_current_console]);
}

static void put_key(TTY *p_tty, u32 key)
{
     if (p_tty->buf_count < TTY_IN_BYTES)
     {
	  *(p_tty->p_head) = key;
	  p_tty->p_head++;
	  if (p_tty->p_head == p_tty->in_buf + TTY_IN_BYTES)
	  {
	       p_tty->p_head = p_tty->in_buf;
	  }
	  p_tty->buf_count++;
     }
}

void tty_write(TTY* p_tty, char *buf, int len)
{
     char *p = buf;
     int i = len;
     while(i)
     {
	  out_char(p_tty->p_console, *p++);
	  i--;
     }
}

int sys_write(char *buf, int len, PROCESS *p_proc)
{
     tty_write(&(tty_table[p_proc->nr_tty]), buf, len);
     return 0;	       
}

