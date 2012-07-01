#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"


static int msg_send(struct s_proc *current, int dest, MESSAGE*m);
static int msg_receive(struct s_proc * current, int src, MESSAGE *m);
static int deadlock(int src, int dest);
static void block(PROCESS *p);
static void unblock(PROCESS *p);
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
	       if (p->p_flags == 0){
		    if (p->ticks > max_ticks){
			 max_ticks = p->ticks;
			 p_proc_ready = p;
		    }
	       }
	  }
	  if (max_ticks == 0)
	  {
	       for (p=proc_table; p<proc_table+NR_TASK+NR_PROC;p++)
	       {
		    if (p->p_flags == 0){
			 p->ticks = p->priority;
		    }
	       }
	  }
     }
     /* if ( p_proc_ready == &proc_table[1]){ */
     /* 	  printf("%s ", p_proc_ready->p_name); */
     /* 	  printf("%d ", p_proc_ready->p_flags); */
     /* 	  printf("%d ", p_proc_ready->ticks); */
     /* 	  while (i<100000) i++; */
     /* } */
}

int ldt_seg_linear(PROCESS *p, int idx)
{
     DESCRIPTOR *d  = &p->ldts[idx];
     return d->base_high<<24 | d->base_mid << 16 | d->base_low;
     
}

void* va2la(int pid, void *va)
{
     PROCESS *p = &proc_table[pid];

     u32 seg_base = ldt_seg_linear(p, INDEX_LDT_RW);
     u32 la = seg_base + (u32)va;

     if (pid < NR_TASK + NR_PROC)
     {
	  assert( la == (u32)va);
	  
     }
     return (void*)la;
}
int proc2pid(PROCESS *p)
{
     return p->pid;
}
int sys_sendrec(int function, int src_dest, MESSAGE *m, struct s_proc*p)
{
     int ret  = 0;
     int caller = proc2pid(p);
     MESSAGE *msg_la = (MESSAGE*)va2la(caller, m);
     msg_la->source = caller;

     if (function == SEND){
	  ret = msg_send(p, src_dest, m);
	  if (ret != 0){
	       return ret;
	  }
     }
     else if (function == RECEIVE){
	  ret = msg_receive(p, src_dest, m);
	  if (ret != 0){
	       return ret;
	  }
     }
     else{
	  panic("{sys_sendrec} invalid function: "
		"%d (SEND:%d, RECEIVE:%d) ", function, SEND, RECEIVE);
     }

     return 0;
}

static int deadlock(int src, int dest)
{
     struct s_proc * p = proc_table + dest;
     while (1){
	  if (p->p_flags & SENDING){
	       if (p->p_sendto == src){
		    /* deadlock occurs */
		    p = proc_table + dest;
		    do {
			 assert(p->p_msg);
			 printf("->%s", p->p_name);
			 p = proc_table + p->p_sendto;
		    }while (p != proc_table + src);
		    return 1;
	       }
	       p = proc_table + p->p_sendto;
	  }
	  else{
	       break;
	  }
     }
}
static int msg_send(struct s_proc *current, int dest, MESSAGE*m)
{
     struct s_proc * p_src = current;
     struct s_proc * p_dest = &proc_table[dest];
     int src = proc2pid(current);
     m->source = src;

     if ( (p_dest->p_flags & RECEIVING) &&
	  (p_dest->p_recvfrom == ANY || p_dest->p_recvfrom == proc2pid(p_src))){
	  memcpy(va2la(dest, p_dest->p_msg),
		 va2la(src, m),
		 sizeof(MESSAGE));

	  p_dest->p_flags &= ~RECEIVING;
	  p_dest->p_recvfrom = NO_TASK;
	  unblock(p_dest);
     }
     else{
	  p_src->p_flags |= SENDING;
	  p_src->p_sendto = dest;
	  p_src->p_msg = m;
	  struct s_proc *p;
	  if (p_dest->q_sending){
	       p = p_dest->q_sending;
	       while (p->next_sending){
		    p = p->next_sending;
	       }
	       p->next_sending = p_src;	 
	  }
	  else{
	       p_dest->q_sending = p_src;
	  }
	  p_src->next_sending = 0;
	  block(p_src);
     }
     return 0;
}
static int msg_receive(struct s_proc * current, int src, MESSAGE *m)
{
     struct s_proc * p_recv = current;
     struct s_proc * p_src = 0;
     struct s_proc * p_prev = 0;
     int copyok = 0;

     if (p_recv->has_int_msg &&
	 ((src == ANY) || (src == INTERRUPT))){
	  MESSAGE msg;
	  memset(&msg, 0, sizeof(MESSAGE));
	  msg.source = INTERRUPT;
	  msg.type = HARD_INT;
	  memcpy(va2la(proc2pid(p_recv), m), &msg, sizeof(MESSAGE));
	  p_recv->has_int_msg = 0;
	  return 0;
     }

     if (src == ANY){
	  if ( p_recv->q_sending){
	       p_src = p_recv->q_sending;
	       copyok = 1;
	  }
     }
     else
     {
	  p_src = &proc_table[src];
	  if ((p_src->p_flags & SENDING) &&
	      (p_src->p_sendto == proc2pid(p_recv))){
	       copyok = 1;
	       struct s_proc *p = p_recv->q_sending;
	       while (p){
		    if (proc2pid(p) == src){
			 p_src = p;
			 break;
		    }
		    p_prev = p;
		    p = p->next_sending;
	       }
	  }
     }

     if (copyok){
	  if (p_src == p_recv->q_sending){
	       p_recv->q_sending = p_src->next_sending;
	  }
	  else {
	       p_recv->next_sending = p_src->next_sending;
	  }
	  p_src->next_sending = 0;

	  memcpy(va2la(proc2pid(p_recv), m),
		 va2la(proc2pid(p_src), p_src->p_msg),
		 sizeof(MESSAGE));
	  p_src->p_msg = 0;
	  p_src->p_sendto = NO_TASK;
	  p_src->p_flags &= ~SENDING;
	  unblock(p_src);
     }
     else{
	  p_recv->p_flags |= RECEIVING;
	  p_recv->p_msg = m;
	  if (src == ANY){
	       p_recv->p_recvfrom = ANY;
	  }
	  else{
	       p_recv->p_recvfrom = proc2pid(p_src);
	  }
	  block(p_recv);
     }
     
     return 0;
}

int send_recv(int function, int src_dest, MESSAGE *msg)
{
     int ret = 0;

     if ( function == RECEIVE ){
	  memset(msg, 0, sizeof(MESSAGE));
     }

     switch ( function ){
     case BOTH:
	  ret = sendrec(SEND, src_dest, msg);
	  if ( ret == 0 ){
	       ret = sendrec(RECEIVE, src_dest, msg);
	  }
	  break;
     case SEND:
     case RECEIVE:
	  ret = sendrec(function, src_dest, msg);
	  break;
     default:
	  break;
     }

     return ret;
}
static void block(PROCESS *p)
{
     assert(p->p_flags);
     schedule();
}
static void unblock(PROCESS *p)
{
     assert(p->p_flags == 0);
}
