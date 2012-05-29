	
;; ------------------------------------
;;	name:	DispStr16
;; 	AX:	string address offset
;;	CX:	string length
;;			
;; -----------------------------------
DispStr16:
	push 	dx
	push	es
	push	bx
	push	bp
	
	mov 	bp, ax
	mov	ax, ds
	mov 	es, ax
	mov 	ax, 1301h
	mov	bx, 000ch
	mov 	dl, 0
	int 	10h
	
	pop	bp
	pop	bx
	pop	es
	pop	dx
	ret
;; end of DispStr16
;; -----------------------------------

;; ------------------------------------
;;	name:	read sector
;; 	AX:	logic sector number (0..BPB_TotSec)
;;	CL:	sectors count to be read
;;	ES:BX:	data cache		
;; -----------------------------------
ReadSec:

	push	bp
	mov 	bp, sp
	sub	esp, 2
	mov	byte [bp-2], cl
	
	push	bx
	mov	bl, [BPB_SecPerTrk]
	div 	bl
	
	mov 	ch, al
	shr	ch, 1		;al mod/ 2
	inc 	ah
	mov 	cl, ah

	and	al, 1		;al & 1
	mov	dh, al
	mov 	dl, [BS_DrvNum]
	pop 	bx
.ReadAgain:
	mov 	ah, 02h
	mov	al, byte [bp-2]
	int 	13h
	jc	.ReadAgain

	add 	esp, 2
	pop	bp

	ret
;;end of ReadSec
;; ----------------------


;; ------------------------------------
;;	name:	GetFATEntry
;; 	input:
;;; 		AX:	previous FAT entry
;;; 	output:
;;; 		AX:	next FAT entry
;;	
;;			
;; -----------------------------------
GetFATEntry:
	push	es
	push	bx
	
	push	ax
	mov	ax, KernelBase
	sub	ax, 256
	mov	es, ax
	pop 	ax

	mov	bx, 3
	mul	bx
	mov	bx, 2
	div	bx
	push	dx

	xor	dx, dx
	mov	bx, [BPB_BytesPerSec]
	div	bx		;ax:sector offset	dx:FAT entry offset
	push 	dx
	
	mov	bx, 0		;ex:bx = 09000h*10h-0100h
	add	ax, FATSectorStart
	mov 	cl, 2
	call	ReadSec

	pop 	dx
	add	bx, dx
	mov	ax, [es:bx]

	pop	dx
	cmp	dx, 0
	jz	.Even
	shr	ax, 4
.Even:	
	and	ax, 0fffh
	pop	bx
	pop 	es
	ret
;;; end of GetFATEntry
;; -----------------------------------	
	