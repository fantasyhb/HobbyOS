#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"


void print_desc(DESCRIPTOR desc);
void clear_line(int st, int end);
int kernel_main()
{
     u8 privilege;
     u8 rpl;
     int eflags;
     
     PROCESS *p_proc = &proc_table[0];
     TASK *p_task = &task_table[0];
     char *p_task_stack = task_stack + STACK_SIZE_TOTAL;
     u16 selector_ldt = SELECTOR_LDT_FIRST;
     int i;
     int priority;
     for (i=0; i<NR_TASK+NR_PROC; i++)
     {
	  if ( i<NR_TASK )
	  {
	       p_task = task_table + i;
	       privilege = PRIVILEGE_TASK;
	       rpl = RPL_TASK;
	       eflags = 0x1202;
	       priority = 30;
	  }
	  else
	  {
	       p_task = user_proc_table + (i - NR_TASK);
	       privilege = PRIVILEGE_USER;
	       rpl = RPL_USER;
	       eflags = 0x202;
	       priority = 10;
	  }
	  
	  strcpy(p_proc->p_name, p_task->name);
	  p_proc->pid = i;
	  p_proc->ldt_sel = selector_ldt;
	  memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS>>3], sizeof(DESCRIPTOR));
	  p_proc->ldts[0].attr1 = DA_C | privilege << 5;
	  memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS>>3], sizeof(DESCRIPTOR));
	  p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;
  
	  p_proc->regs.cs = (0 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
	  p_proc->regs.ds = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
	  p_proc->regs.es = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
	  p_proc->regs.fs = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
	  p_proc->regs.ss = (8 & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
	  p_proc->regs.gs = (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
	  p_proc->regs.eip = (u32)p_task->initial_eip;
	  p_proc->regs.esp = (u32)p_task_stack;
	  p_proc->regs.eflags = eflags;  
    
	  p_task_stack -= p_task->stacksize;

	  p_proc->nr_tty = 0;
	  
	  p_proc->ticks = p_proc->priority = priority;
	  
	  p_proc->p_flags = 0;
	  p_proc->p_msg = 0;
	  p_proc->p_recvfrom = NO_TASK;
	  p_proc->p_sendto = NO_TASK;
	  p_proc->has_int_msg = 0;
	  p_proc->q_sending = 0;
	  p_proc->next_sending = 0;

	  p_proc++;
	  p_task++;
	  selector_ldt += 1<<3;

     }
     
    
    
     /* proc_table[1].nr_tty = 0; */
     /* proc_table[2].nr_tty = 1; */
     /* proc_table[3].nr_tty = 2; */

     p_proc_ready = proc_table;
     k_reenter = 0;
     ticks = 0;
     /* init clock */
     init_clock();
     /* init keyboard */
     init_keyboard();

     restart();  //kernel.asm

}

void TestA()
{
     /* 
     spin("easdfafd");
       assert(0);     printf("%d \n", get_ticks());
 */
     int i=0x1000;
     while(1)
     {
	  printf("%d ", get_ticks());
	  ms_delay(1000);
     }

}
void TestB()
{
     int i=0x2000;
     
     while(1)
     {
	  /* 
	  printf("BBB");
	   */

     } 

}
void TestC()
{
     int i=0x1500;

     while(1)
     {
	  /* 
	  printf("CCC");
	   */

     }
}

void print_desc(DESCRIPTOR desc)
{
     u32 limit = (u32)desc.limit_low + (u32)((desc.limit_high_attr2&0x0f)<<16);
     u32 base = (u32)(desc.base_high<<24) + (u32)(desc.base_mid<<16) + (u32)desc.base_low;
     u32 attr1 = (u32)desc.attr1;
     u32 attr2 = (u32)((desc.limit_high_attr2&0xf0)>>4);
     print_str("limit: ");  print_int(limit);
     print_str("\nbase: ");  print_int(base);
     print_str("\nattr1: "); print_int(attr1);
     print_str("\nattr2: "); print_int(attr2);
     print_str("\n");
}
void clear_line(int st, int end)
{
     int i, j;
     int temp = disp_pos;
     disp_pos = st*80;
     for (i=st; i<end; i++)
     {
	  for (j=0; j<80; j++)
	  {
	       print_str(" ");
      
	  }
     }
     disp_pos = temp;
}
void panic(const char* fmt, ...)
{
     int i ;
     char buf[STR_LEN];

     va_list arg = (va_list)((char*)&fmt + 4);

     i = vsprintf(buf, fmt, arg);

//     printf("%s", buf);
     printf("%cSYSTEM PANIC %s", MAG_CH_PANIC, buf);

}
int get_ticks()
{
     MESSAGE msg;
     memset(&msg, 0, sizeof(MESSAGE));
     msg.type = GET_TICKS;
     
     /* send_recv(SEND, TASK_SYS, &msg); */
     send_recv(BOTH, TASK_SYS, &msg);
     /* /\* debug *\/ */
     /* printf("t send ok\n"); */
     /* send_recv(RECEIVE, TASK_SYS, &msg); */
     /* printf("t receive ok .. ret: %d \n", msg.RETVAL); */
     /* printf("ticks: %d\n", ticks); */
     return msg.RETVAL;
}
