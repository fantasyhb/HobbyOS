	;; global 	get_ticks
;global  write
global printx
global sendrec
	
NR_get_ticks		equ 	0
INT_VECTOR_SYS_CALL	equ	0x90

NR_write		equ 	1

NR_printx		equ 	0
NR_sendrec		equ 	1
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

printx:
	mov	eax, NR_printx
	mov	edx, [esp + 4]
	int	INT_VECTOR_SYS_CALL
	ret
sendrec:
	mov 	eax, NR_sendrec
	mov	ebx, [esp + 4]	;function
	mov 	ecx, [esp + 8]	;src_dest
	mov 	edx, [esp + 12]	;p_msg
	int 	INT_VECTOR_SYS_CALL
	ret