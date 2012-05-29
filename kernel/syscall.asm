
global 	get_ticks		;
global  write
	
NR_get_ticks		equ 	0
INT_VECTOR_SYS_CALL	equ	0x90

NR_write		equ 	1
	
bits	32
[section .text]

get_ticks:
	mov	eax, NR_get_ticks ;
	int	INT_VECTOR_SYS_CALL
	ret

write:
	mov 	eax, NR_write
	mov	ebx, [esp+4]
	mov 	ecx, [esp+8]
	int	INT_VECTOR_SYS_CALL
	ret