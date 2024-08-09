; code: language=nasm tabSize=8
%include "defines.inc"

cold_boot:	

; Adapted from Victor 9000 ROM

	mov ax, As6500				; access CRT controller
	mov es, ax
	mov byte [es:CrtRg0], 1
	mov byte [es:CrtRg1], 0		; shut down CRT

;	Test screen ram, to ensure CRT could possibly operate.
;	Test with 55AA and AA55 hex.  Leave memory at 0000.

	mov ax, ScreenRam		; base of screen ram
	mov es, ax
	mov ax, 055AAh			; first test pattern

.test_screen_ram:
	xor di, di				; start at offset of 0
	mov cx, 2048			; for 2k words (4k bytes)
	cld						; auto increment locations
	rep stosw

	cmp ax, 0				; done?
	jz  .init_pic

	xor di, di				; compare starting at offset 0
	mov cx, 2048
	repz scasw
	jnz .bad_screen_ram		; error, did not match

	xor ax, 0FFFFh			; switch pattern from 55AA to AA55
	jl .test_screen_ram		; not done, test with pattern #2

	xor ax, ax				; final pattern is zeroes
	jmp .test_screen_ram

; screen RAM is bad...
.bad_screen_ram:
	mov al, FE_bad_screen	; error code for field service
	mov dx, FE_Ioport

.error_display:
	out dx, al				; issue the error for FE scoping
	jmp .error_display		; that's all folks

.init_pic:					; initialize Intel 8259 PIC
	mov ax, SegExtraIo
	mov es, ax				; address the memory-mapped I/O

	mov byte [es:PicPort0], 17h		; edge triggered mode
	mov byte [es:PicPort1], 20h 	; ICW2
	mov byte [es:PicPort1], 01h 	; ICW4 = 8086 mode

	mov cx, 8				; all interrupts are masked
	mov al, 60h				; set up to clear all pending interrupts

.cl1:
	mov [es:PicPort0], al 	; issue SEOI
	inc al
	loop .cl1				; clear all ISR bits



%if 0
; From Super PC/Turbo XT BIOS

%define TURBO_ENABLED 1

	cld
	mov	al, 0				; Prepare to initialize
	out	0A0h, al			;   no NMI interrupts
	mov	dx, 3D8h			; Load Color Graphic port
	out	dx, al				;   no video display
	mov	dl, 0B8h			; Load Monochrome port
	inc	al				;   no video display
	out	dx, al				;   write it out
	mov	al, 10011001b			; Program 8255 PIA chip
	out	63h, al 			;   Ports A & C, inputs
%ifdef	TURBO_ENABLED
	mov	al, 10100101b			; Set (non)turbo mode
	out	61h, al 			;   on main board
%endif
	mov	al, 01010100b			; IC 8253 inits memory refresh
	out	43h, al 			;   chan 1 pulses IC 8237 to
	mov	al, 12h 			;   DMA every 12h clock ticks
	out	41h, al 			;   64K done in 1 millisecond
	mov	al, 01000000b			; Latch value 12h in 8253 clock
	out	43h, al 			;   chip channel 1 counter

.init_dma:
	mov	al, 0				; Do some initialization
	out	81h, al 			;   DMA page reg, chan 2
	out	82h, al 			;   DMA page reg, chan 3
	out	83h, al 			;   DMA page reg, chan 0,1
	out	0Dh, al 			; Stop DMA on 8237 chip
	mov	al, 01011000b			; Refresh auto-init dummy read
	out	0Bh, al 			;   on channel 0 of DMA chip
	mov	al, 01000001b			; Block verify
	out	0Bh, al 			;   on channel 1 of DMA chip
	mov	al, 01000010b			; Block verify
	out	0Bh, al 			;   on channel 2 of DMA chip
	mov	al, 01000011b			; Block verify
	out	0Bh, al 			;   on channel 3 of DMA chip
	mov	al, 0FFh			; Refresh byte count
	out	1, al				;   send lo order
	out	1, al				;   send hi order
	inc	ax				; Initialize 8237 command reg
	out	8, al				;   with zero
	out	0Ah, al 			; Enable DMA on all channels
	mov	al, 00110110b			; Set up 8253 timer chip
	out	43h, al 			;   chan 0 is time of day
	mov	al, 0				; Request a divide by
	out	40h, al 			;   65536 decimal
	out	40h, al 			;   0000h or 18.2 tick/sec
	mov	dx, 213h			; Expansion unit port
	inc	ax				;   enable it
	out	dx, al				;   do the enable

%endif
