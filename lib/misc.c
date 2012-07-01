/* misc.c */
#include "type.h"
#include "const.h"
#include "protect.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "kliba.h"
#include "global.h"
#include "keyboard.h"

void assertion_failure(char *exp, char *file, char *base_file, int nline)
{
     printl("%c  assert(%s) failed: file: %s, base file: %s, ln: %d",
	    MAG_CH_ASSERT,
	    exp, file, base_file, nline);

     spin("assertion failuer()");
     __asm__ __volatile__("ud2");
}

void spin( char *info)
{
     printl("\nspinning in %s...\n", info);
     while (1) ;
}
