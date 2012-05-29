/* console.c */
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"
#include "keyboard.h"

static void set_cursor(u32 pos);
static void set_v_start_addr(u32 addr);
static void flush(CONSOLE *p_con);
     
void out_char(CONSOLE *p_con, char ch)
{
     u8 *p_vmem = (u8 *) (V_MEM_BASE + p_con->cursor*2);

     switch (ch)
     {
	  
     case '\n':
	  if (p_con->cursor < p_con->origin_addr + p_con->v_mem_limit - SCR_WIDTH)
	  {
	       p_con->cursor = p_con->origin_addr + SCR_WIDTH*((p_con->cursor - p_con->origin_addr)/SCR_WIDTH+1);
	  }
	  break;
     case '\b':
	  if (p_con->cursor > p_con->origin_addr)
	  {
	       p_con->cursor--;
	       *(p_vmem-2) = ' ';
	       *(p_vmem-1) = DEFAULT_CHAR_COLOR;
	  }
	  break;
     default:
	  if (p_con->cursor < p_con->origin_addr + p_con->v_mem_limit - 1)
	  {
	       *p_vmem++ = ch;
	       *p_vmem++ = DEFAULT_CHAR_COLOR;
	       p_con->cursor++;
	  }
     }
     while (p_con->cursor >= p_con->current_start_addr + SCR_SIZE)
     {
	  scroll_screen(p_con, 1);
     }

     flush(p_con);
     /* 
     set_cursor(p_con->cursor);
      */
}

static void set_cursor(u32 pos)
{
     disable_int();
     out_byte(CRTC_ADDR_REG, CURSOR_H);
     out_byte(CRTC_DATA_REG, (pos>>8) & 0xFF);
     out_byte(CRTC_ADDR_REG, CURSOR_L);
     out_byte(CRTC_DATA_REG, ((pos) & 0xFF));	
     enable_int();
     
}

void init_console(TTY *p_tty)
{
     int nr_con = p_tty - tty_table;
     
     p_tty->p_console = &console_table[nr_con];

     int v_mem_size = V_MEM_SIZE >>1;
     int con_v_mem_size = v_mem_size / NR_CONSOLE;

     p_tty->p_console->origin_addr = nr_con * con_v_mem_size;
     p_tty->p_console->v_mem_limit = con_v_mem_size;
     p_tty->p_console->current_start_addr = p_tty->p_console->origin_addr;
     p_tty->p_console->cursor = p_tty->p_console->origin_addr;
     if (nr_con == 0)
     {
          p_tty->p_console->cursor = disp_pos/2;
	  disp_pos = 0;
     }else
     {
	  out_char(p_tty->p_console, nr_con + '0');
	  out_char(p_tty->p_console, '#');
     }
     set_cursor(p_tty->p_console->cursor);
}
void select_console(int nr_con)
{
     if (nr_con<0 || nr_con>=NR_CONSOLE)
     {	  
	  return;
     }
     nr_current_console = nr_con;

     set_cursor(console_table[nr_con].cursor);
     set_v_start_addr(console_table[nr_con].current_start_addr);
}

static void set_v_start_addr(u32 addr)
{
     disable_int();
     out_byte(CRTC_ADDR_REG, START_ADDR_H);
     out_byte(CRTC_DATA_REG, (addr>>8) & 0xFF);
     out_byte(CRTC_ADDR_REG, START_ADDR_L);
     out_byte(CRTC_DATA_REG, (addr & 0xFF));	
     enable_int();         
}

void scroll_screen(CONSOLE *p_con, int direction)
{
     if ( direction > 0)
     {
	  int factor = p_con->current_start_addr + SCR_WIDTH;
	  if (factor < p_con->origin_addr+p_con->v_mem_limit)
	  {
	       p_con->current_start_addr = factor;	    
	  }	      
     }
     else if ( direction < 0)
     {
	  if (p_con->current_start_addr > p_con->origin_addr)
	  {
	       p_con->current_start_addr -= SCR_WIDTH;
	  }
     }
     else
     {
	  return;
     }

     flush(p_con);
     /* 
     set_v_start_addr(p_con->current_start_addr);
     set_cursor(p_con->cursor);
      */
}

static void flush(CONSOLE *p_con)
{
     set_cursor(p_con->cursor);
     set_v_start_addr(p_con->current_start_addr);
}
