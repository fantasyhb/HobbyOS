/* clock.c*/
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"

void clock_handler(int irq);

void init_clock()
{
     /*initialize 8253*/
     out_byte(TIMER_MODE, RATE_GENERATOR);
     out_byte(TIMER0, (u8)(TIMER_FREQ/HZ));/* write low byte of counter*/
     out_byte(TIMER0, (u8)((TIMER_FREQ/HZ)>>8));/*write high byte counter*/

     set_irq_handler(CLOCK_IRQ, clock_handler);
     enable_irq(CLOCK_IRQ);

}

void clock_handler(int irq)
{
     ticks++;
     p_proc_ready->ticks--;

   
     if  (k_reenter != 0)
     {
	  return;
     }
     if (p_proc_ready->ticks>0)
     {
	  return;
     }
     /*print_str("#");  
       print_int(k_reenter);
       p_proc_ready++;
       if (p_proc_ready >= proc_table + NR_TASK)
       p_proc_ready = proc_table;
     */
     schedule();
}
void ms_delay(int ms)
{
     int t = get_ticks();
     while(((get_ticks()-t)*1000/HZ)<ms);
}

