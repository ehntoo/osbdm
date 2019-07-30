; JM60 BGND Driver
; assembly level applied for execution speed, C should be evaluated.
; 


; BGND_WR_RD is the primary transfer function.
; Function Routine inputs provide all necessary information to complete a transfer.

; Global variables for pointers and buffers...
; Memory space location may require zero ram or direct page space <0x100

BG_WR_COUNT	RMB	1	; 0 - 255, number of bytes to write on this transfer
BG_RD_COUNT	RMB	1	; 0 - 255, number of bytes to read
BG_WR_PTR	RMB	2	; Write Buffer pointer, points at data to write
BG_RD_PTR	RMB	2	; Read Buffer pointer, points to received data
BG_ERR_FLG	RMB	1	; no error if clear  

; Communication Timing Global Variables...

BIT_TIME:	RMB	2	; 16 bit value = minimum 16 target clock time periods 
				; This value = minimum 16x Target Clocks
				; Value is applied as the PWM Timer modulus or time out time
				; This value is determined by the Target SYNC pulse time value or
				; 	provided as the clock setting by the host.
				; Time values are based on a 6Mhz frequency. 
				; Minimum target clock frequency for SYNC command is 11.8Khz 
				; Minimum target frequency for communication is 1.5Khz
				; Maximum Target write baud rate = 827.5Khz
				; Maximum target clock for writes = 24Mhz
				; Maximum Target clock for reads =  
				;  

; These values are applied for edge aligned PWM timer duty cycle time
1_TIME:		RMB	2	; 16 bit value = 4 Target clock time periods
0_TIME:		RMB	2	; 16 bit value = 12 Target clock time periods
RD_TIME:	RMB	2	; 16 bit value = 10 Target clock time periods


; Communication routine local variables...

Bit_Count:	RMB	1	; 8 bit value = bit count for transfer, set by sub routine.	




;*************************************************************************

; BG_WR_RD does a write than read of of data.  All transfer packets start with a write (command). 


BG_WR_RD:
	psha			; save the core
	pshx			; 
	pshh			;
	clr	BG_ERR_FLG	; clear flag

; service Write first, check for bits only..
	



;****************************************************************************
; Backgound write output
; 
; A = transfer data byte
; h:x = time value transfer register 
; 0_Time = 16 bit timer value for 12 target clocks, 0 bit time
; 1_Time = 16 bit timer value for 4 Target clocks, 1 bit time
; Bit_count = temporary local variable
; DIR output = TX = logic 0 = write to target mode

BGND_WRITE_BYTEs:
	ldhx	BIT_TIME		; get the bit time period (6Mhz count)
;	aix	#2			; allow +8 JM60 cycles per bit
	sthx	TPM1MODH		; set the timer modulus
	mov	#7,Bit_Count		; set the bit count = byte -1 = 7
	ldhx	BG_WR_PTR		; get the buffer pointer
	ldaa	,x			; get first byte out
	lsla				; get the first bit
	bcc	BGND_WRITE_0		; if 0, branch
	ldhx	1_TIME			; get the 1 bit timer value
	bra	BGND_WRITE1		; go load
BGND_WRITE_0:
	ldhx	0_TIME			; get the 0 bit time value
BGND_WRITE_1:
	sthx	TPM1C0VH		; set the bit time
	mov	#$0A,TPM1SC		; start the timer
	mov	#$28,TPM1C0SC		; start BGND out timer channel
; send byte loop, will send write buffer per wr byte count	
BG_WR_BYTE:
	lsla				; 1, get next bit
	bcs	BG_WR_ONE		; 3, branch if 1 bit	
BG_WR_ZERO:
	ldhx	0_TIME			; 4, load next bit time
BG_LOOP0:
	brclr	7,TPM1C0SC BG_LOOP1	; 5, wait for timer to fire
	sthx	TPM1C0VH		; 4, set next bit time
	mov	#$28,TPM1C0SC		; 5, Re-start timer
	dbjnz	Bit_Count, BG_WR_BYTE	; 7, check count, loop with bits
	bra	BG_WR_IDLE		; 3, go update
BG_WR_ONE:
	ldhx	1_TIME			; 4, load next bit time
BG_LOOP1:
	brclr	7,TPM1C0SC BG_LOOP1	; 5, wait for timer to fire
	sthx	TPM1C0VH		; 4, set next bit time
	mov	#$28,TPM1C0SC		; 5, Re-start timer
	dbjnz	Bit_Count, BG_WR_BYTE	; 7, check count, loop with bits
				; flow to BG_WR_IDLE
; Test for end or more writes...
; BGND output = high after compare..
; The buffer service time is predictable if in assembly and timer period for bit 8
; may be extended to compensate, timer remains active until end of buffer data.
 
BG_WR_IDLE:
	dbjnz	WR_Byte_Cnt,BG_WR_NXT	; 7, branch with more bytes to write
	clr	TPM1C0VH		; 5, set 0 time, no bits out
	clr	TPM1C0VL		; 5, set 0 time
;	clr	TPM1SC			; 5, Timer is off
	rts				; done with write out

; get next byte to write...
BG_WR_NXT:
	ldhx	BG_WR_PTR		; get the buffer pointer
	aix	#1			; next address
	sthx	BG_WR_PTR		; save new pointer
BG_WR_START:
	lda	,x			; get the new byte
	mov	#8,Bit_Count		; update bit count
	bra	BG_WR_BYTE		; back to top 


; Background read loop

	mov	#$28,TPM1C0SC		; start BGND out timer channel
	mov	#$28,TPM1C1SC		; start DIR out timer channel
	ldhx	RD_TIME			; load the read sample time
	sthx	TPM0C0VH		; set the sample time
	ldhx	1_TIME			; load the direction change time
	sthx	TPM1C1VH		; set the direction change time
;	mov	#$0A,TPM1SC		; start the timer (only required if off)
	mov	#8,Bit_Count		; set the bit count

; hardware change: move BGND in to PortF 0 for speed.
; Port f0 = input routine...
; applying the timer compare function on the BGND input and looping for a
; high going edge may speed up the receive loop.  I am concerend about high
; speed input rate and the 5 cycle bit test and branch time on the sample
; timer flag. 


BG_READ:
	brclr	7,TPM1C0SC BG_READ	; 5, wait for timer to fire
	lda	PORTF			; 3, get the port
	lsra				; 1, get the bit
	rolx				; 1, save the bit
	bclr	7,TPM1C0SC		; 5, clear timer flag
	dbnz	Bit_Count,BG_READ	; 7, check bit count - loop
; service byte in


; port bit is masked...
BG_READ:
	lda	#$02			; 2, load mask
BG_READ_1:
	brclr	7,TPM1C0SC BG_READ	; 5, wait for timer to fire
	lda	PORTF			; 3, get the port

	dbnz	Bit_Count,BG_READ	; 7, check bit count - loop






