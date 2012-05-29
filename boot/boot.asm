	org  07c00h			; Boot 状态, Bios 将把 Boot Sector 加载到 0:7C00 处并开始执行
	jmp short LABEL_START		; Start to boot.
	nop				; 这个 nop 不可少
%include	"fat12h.inc"
;;;;========================================
;;; some data 
;;; ========================================
Constant:	

BootMessage:		db	"Booting"
LenBootMsg	equ	$-BootMessage
LoaderFilename:		db	"LOADER  BIN"
LenLoaderFile	equ	$-LoaderFilename
LoaderMsg:		db	"Loading"
LenLoaderMsg	equ	$-LoaderMsg
LoaderError:		db 	"No Loader"
LenLoaderError	equ	$-LoaderError
	
StackTop	equ	7c00h	
LoaderBase	equ	9000h
LoaderOffset	equ	0100h

;;; ------------------------------	
Var:
dwSectorStart	dw	0
dwRootDirSecIndex	dw	0
;;; ========================================
	
LABEL_START:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov 	ss, ax
	mov	sp, StackTop

	mov	ax, 0600h
	mov	bx, 0700h
	mov	cx, 0
	mov 	dx, 314fh
	int	10h		; clear screen

	mov	dh, 0		;line no.
	mov 	ax, BootMessage
	mov 	cx, LenBootMsg
 	Call	DispStr16			

	xor 	ah, ah
	xor	dl, dl
	int 	13h		;reset fd

	mov	ax, LoaderBase
	mov	es, ax
	mov 	bx, LoaderOffset
	mov	ax, 19
	mov 	cl, 1
 	call	ReadSec
		
;;; begin to find loader.bin and load it if existed
	mov	dword [dwSectorStart], RootDirSectorStart
	mov	dword [dwRootDirSecIndex], RootDirSectorCount
LoopReadRootDir:
	cmp    	word [dwRootDirSecIndex], 0
	jz	NoLoaderFile
	dec	word [dwRootDirSecIndex]
	mov 	ax, LoaderBase
	mov 	es, ax
	mov	bx, LoaderOffset
	mov 	ax, [dwSectorStart]
	mov	cl, 1
	call	ReadSec
	mov	si, LoaderFilename 		;ds:si -> LoaderFilename: 'Loader.bin'
	mov	di, LoaderOffset		;es:di -> LoaderBase:loaderOffset
	mov 	dx, 16		;files number per sector :512B/32B
LoopReadFilesInSector:
	cmp	dx, 0
	jz	NextRootDirSector
	dec	dx

	mov	cx, LenLoaderFile
LoopCmpFilename:
	cmp	cx, 0
	jz	FindLoaderFile
	dec	cx
	lodsb
	cmp	al, byte [es:di]
	jnz	NextDirFile
	inc	di
	jmp	LoopCmpFilename
NextDirFile:
	and	di, 0ffe0h	;remove	low 32-bits for file offset in each sector
	add	di, 20h
	mov	si, LoaderFilename
	jmp	LoopReadFilesInSector
NextRootDirSector:
	inc	word [dwSectorStart]
	jmp	LoopReadRootDir
NoLoaderFile:
	mov 	ax, LoaderError
	mov	cx, LenLoaderError
	call	DispStr16
	jmp	$			; 无限循环
FindLoaderFile:
	mov	dh, 1
	mov 	ax, LoaderMsg
	mov 	cx, LenLoaderMsg
	call	DispStr16

	and	di, 0ffe0h
	add	di, 01ah
	
	mov	cx, word [es:di] ;first	FAT entry
	push	cx		 ;save this entry
	add	cx, DataSectorStart
	sub	cx, 2		;

	mov 	ax, LoaderBase
	mov	es, ax
	mov	bx, LoaderOffset
	mov	ax, cx
	
LoadLoaderFile:
	;; print ....
	push	ax
	push	bx
	mov 	ah, 0eh
	mov	al, '.'
	mov 	bl, 0fh
	int 	10h
	pop 	bx
	pop 	ax

	mov	cl, 1
	call	ReadSec
	
	pop	ax		;get the saved entry
	call	GetFATEntry	;get next FAT entry
	;;	cmp	ax, 0ffh	
	;;;	jz	LoaderDone
	cmp 	ax, 0fffh
	jz	LoaderDone
	push	ax
	add	ax, DataSectorStart
	sub	ax, 2
	add	bx, [BPB_BytesPerSec]
	jmp	LoadLoaderFile	;go on loading file
LoaderDone:	
	jmp	LoaderBase:LoaderOffset	; jmp to loader.bin!
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
	mov	ax, LoaderBase
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
;; -----------------------------------"
	
times 	510-($-$$)	db	0	; 填充剩下的空间，使生成的二进制代码恰好为512字节
dw 	0xaa55				; 结束标志
