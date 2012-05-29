/* keyboard.c */
#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "protect.h"
#include "proc.h"
#include "kliba.h"
#include "global.h"
#include "keyboard.h"
#include "keymap.h"

void keyboard_handler(int irq);

static KB_INPUT kb_in;
static int code_with_E0 = 0;
static int shift_l;
static int shift_r;
static int alt_l;
static int alt_r;
static int ctrl_l;
static int ctrl_r;
static int caps_lock;
static int num_lock;
static int scroll_lock;
static int column;

static u8 keyboard_read_byte();
static void set_leds();
static void kb_wait();
static void kb_ack();

void init_keyboard()
{
     shift_l = shift_r = 0;
     alt_l = alt_r = 0;
     ctrl_l = ctrl_r  = 0;
     caps_lock = 0;
     num_lock = 0;
     scroll_lock = 0;

     set_leds();
     
     kb_in.count = 0;
     kb_in.p_head = kb_in.p_tail = kb_in.buf;
    
     set_irq_handler(KEYBOARD_IRQ, keyboard_handler);
     enable_irq(KEYBOARD_IRQ);
}

void keyboard_handler(int irq)
{
    
     u8 scan_code = in_byte(KB_DATA);
     if (kb_in.count < KB_IN_BYTES)
     {
	  *(kb_in.p_head) = scan_code;
	  kb_in.p_head++;
	  if (kb_in.p_head == kb_in.buf + KB_IN_BYTES){
	       kb_in.p_head = kb_in.buf;
	  }
	  kb_in.count++;
     }
/*    print_int(scan_code);	 */
     /*  print_str(" ");*/
}
void keyboard_read(TTY *p_tty)
{
     u8 scan_code;
     char output[2];
     int make;

     u32 *keyrow;
     u32 key = 0;
     
     memset(output, 0, 2);
     
     scan_code = keyboard_read_byte();
     /*
       print_int(scan_code);
       print_str(" ");

     */
     /* parse scan code  */
     if (scan_code == 0xE1)
     {
	  int i;
	  u8 pausebrk_codes[] = {0xE1, 0x1D, 0x45, 0xE1, 0x9D, 0xC5};
	  int is_pausebrk = 1;
	  for (i=1; i<6; i++)
	  {
	       if (keyboard_read_byte()!= pausebrk_codes[i])
	       {
		    is_pausebrk = 0;
		    break;
	       }
 	  }
	  if (is_pausebrk == 1)
	  {
	       key = PAUSEBREAK;
	  }
     }
     else  if (scan_code == 0xE0)
     {
	  scan_code = keyboard_read_byte();
	  /* printscreen key is down */
	  if (scan_code == 0x2A)
	  {
	       if (keyboard_read_byte() == 0xE0)
	       {
		    if (keyboard_read_byte()  == 0x37)
		    {
			 key = PRINTSCREEN;
			 make = 1;
		    }
	       }
	  }
	  /* printscreen key is up*/
	  if (scan_code == 0xB7)
	  {
	       if (keyboard_read_byte() == 0xE0)
	       {
		    if (keyboard_read_byte()  == 0xAA)
		    {
			 key = PRINTSCREEN;
			 make = 0;
		    }
	       }
	  }
	  /* others*/
	  if (key == 0)
	  {
	       code_with_E0 = 1;
	  }
     }
     if (key!=PAUSEBREAK && key!=PRINTSCREEN)
     {
	  make = scan_code & FLAG_BREAK ? 0 : 1;
	  keyrow = &keymap[(scan_code & 0x7F) * MAP_COLS];
	  column = 0;
	  int caps = shift_l || shift_r;
	  if (caps_lock)
	  {
	       if (keyrow[0] >= 'a' && keyrow[0] <= 'z')
	       {
		    caps = 1 - caps;
	       }
	  }
	  
	  if (caps)
	  {
	       column  = 1;
	  }
	  if (code_with_E0)
	  {
	       column = 2;
	       code_with_E0 = 0;
	  }
	  
	  key = keyrow[column];
	  
	  switch (key)
	  {
	  case SHIFT_L:
	       shift_l = make;
	       break;
	  case SHIFT_R:
	       shift_r = make;
	       break;
	  case CTRL_L:
	       ctrl_l = make;
	       break;
	  case CTRL_R:
	       ctrl_r = make;
	       break;
	  case ALT_L:
	       alt_l = make;
	       break;
	  case ALT_R:
	       alt_r = make;
	       break;
	  case CAPS_LOCK:
	       if (make)
	       {
		    caps_lock = 1 - caps_lock;
		    set_leds();
	       }
	       break;
	  case NUM_LOCK:
	       if (make)
	       {
		    num_lock =  1 - num_lock;
		    set_leds();
	       }
	       break;
	  case SCROLL_LOCK:
	       if (make)
	       {
		    scroll_lock = 1 - scroll_lock;
		    set_leds();
	       }
	       break;
	  default:
	       break;
	  }
	  
	  if (make)
	  {
	       int pad = 0;
	       if (key >= PAD_SLASH && key <= PAD_9)
	       {
		    pad = 1;
		    switch (key)
		    {
		    case PAD_SLASH:
			 key = '/';
			 break;
		    case PAD_STAR:
			 key = '*';
			 break;
		    case PAD_MINUS:
			 key = '-';
			 break;
		    case PAD_PLUS:
			 key = '+';
			 break;
		    case PAD_ENTER:
			 key = ENTER;
			 break;
		    default:
			 if (num_lock &&
			     (key >= PAD_0) &&
			     (key <= PAD_9)) {
			      key = key - PAD_0 + '0';
			 }
			 else if (num_lock &&
				  (key == PAD_DOT)) {
			      key = '.';
			 }
			 else{
			      switch(key) {
			      case PAD_HOME:
				   key = HOME;
				   break;
			      case PAD_END:
				   key = END;
				   break;
			      case PAD_PAGEUP:
				   key = PAGEUP;
				   break;
			      case PAD_PAGEDOWN:
				   key = PAGEDOWN;
				   break;
			      case PAD_INS:
				   key = INSERT;
				   break;
			      case PAD_UP:
				   key = UP;
				   break;
			      case PAD_DOWN:
				   key = DOWN;
				   break;
			      case PAD_LEFT:
				   key = LEFT;
				   break;
			      case PAD_RIGHT:
				   key = RIGHT;
				   break;
			      case PAD_DOT:
				   key = DELETE;
				   break;
			      default:
				   break;
			      }
			 }
			 break;
		    }
	       }
	       
	       key |= shift_l ? FLAG_SHIFT_L : 0;
	       key |= shift_r ? FLAG_SHIFT_R : 0;
	       key |= ctrl_l ? FLAG_CTRL_L : 0;
	       key |= ctrl_r ? FLAG_CTRL_R : 0;
	       key |= alt_l ? FLAG_ALT_L : 0;
	       key |= alt_r ? FLAG_ALT_R : 0;

	       in_process(p_tty, key);
	  }
	       

/*	  
	  print_int(scan_code);
	  print_str(" ");
	  /*   */
     }
}
static u8 keyboard_read_byte()
{
     u8 scan_code;
     while (kb_in.count <= 0) ;
     
     disable_int();
     scan_code = *(kb_in.p_tail);
     kb_in.p_tail++;
     if (kb_in.p_tail == kb_in.buf + KB_IN_BYTES)
     {
	  kb_in.p_tail = kb_in.buf;
     }
     kb_in.count--;
     enable_int();

     return scan_code;
}

static void kb_wait()
{
     u8 kb_stat;
     do {
	  kb_stat = in_byte(KB_CMD);
     }while(kb_stat & 0x02);
}

static void kb_ack()
{
     u8 kb_read;
     do{
	  kb_read = in_byte(KB_DATA);
     }while (kb_read != KB_ACK);
}

static void set_leds()
{
     u8 leds = (caps_lock << 2) | (num_lock << 1) | scroll_lock;
     kb_wait();
     out_byte(KB_DATA, LED_CODE);
     kb_ack();
     kb_wait();
     out_byte(KB_DATA, leds);
     kb_ack();
}
