#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"

void task_sys()
{
     MESSAGE msg;
     int i = 0;

     while (1){
	  send_recv(RECEIVE, ANY, &msg);
	  int src = msg.source;

	  switch (msg.type){
	  case GET_TICKS:
	       msg.RETVAL = ticks;
	       send_recv(SEND, src, &msg);
	       break;
	  default:
	    
	       panic("unknown msg type");
	       break;
	  }
     }

}     
