#include "type.h"
#include "const.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "protect.h"
#include "proc.h"
#include "kliba.h"
#include "global.h"

/*
int disp_pos;
u8 gdt_ptr[8];
DESCRIPTOR gdt[GDT_SIZE];
*/
void cstart()
{
  print_str("\n\n"
	    "----------"
	    "cstart"
	    "----------\n");
  memcpy(&gdt,
	 (void*)(*((u32*)(&gdt_ptr[2]))),
	 *((u16*)(&gdt_ptr[0]))+1);
  *((u16*)(&gdt_ptr[0])) = GDT_SIZE * sizeof(DESCRIPTOR) - 1;
  *((u32*)(&gdt_ptr[2])) = (u32)&gdt;

  *((u16*)(&idt_ptr[0])) = IDT_SIZE * sizeof(GATE) - 1;
  *((u32*)(&idt_ptr[2])) = (u32)&idt;

  init_prot();
  print_str("----------"
	    "cstart"
	    "----------\n");
  
}
