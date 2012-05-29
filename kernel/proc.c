#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"

int sys_get_ticks()
{
  return ticks;
}
void schedule()
{
  PROCESS *p;
  int max_ticks = 0;
  int i = 0;
  while (max_ticks == 0)
  {
      /*      print_int(i++);	        */
      for (p=proc_table; p<proc_table+NR_TASK+NR_PROC;p++)
      {
	  if (p->ticks > max_ticks){
	    max_ticks = p->ticks;
	    p_proc_ready = p;
	  }
      }
      if (max_ticks == 0)
      {
	  for (p=proc_table; p<proc_table+NR_TASK+NR_PROC;p++)
	      {
		  p->ticks = p->priority;
	      }
      }
  }
}
