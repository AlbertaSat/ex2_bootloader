;-------------------------------------------------------------------------------
;  sys_core.asm
;
;  Author      : QJ Wang. qjwang@ti.com
;  Date        : 1-12-2016
;
;  Copyright (c) 2008-2011 Texas Instruments Incorporated.  All rights reserved.
;  Software License Agreement
;
;  Texas Instruments (TI) is supplying this software for use solely and
;  exclusively on TI's microcontroller products. The software is owned by
;  TI and/or its suppliers, and is protected under applicable copyright
;  laws. You may not combine this software with "viral" open-source
;  software in order to form a larger program.
;
;  THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
;  NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
;  NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
;  A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
;  CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
;  DAMAGES, FOR ANY REASON WHATSOEVER.
;
; *****************************************************************************


    .text
    .arm


    .def     _cal_crc16_
    .asmfunc

;#define pmsg        R0;  the first argument;  R0 is also thereturn value
;#define count       R1;  the second argument
;#define msg         R2
;#define poly        R3
;#define index       R4
;#define crc         R12

_cal_crc16_

    stmfd sp!, {r2-r4, r12}   ;r2 at old_sp-4, r1 at old_sp-8, r0 at old_sp-12
                              ;sp at r0
;   unsigned short crc = 0;
	MOV             R12, #0x0
;   while(count--) {
;do 1 byte each time
	CMP             R1, #0x0           ;R1 = count,  update flags based on R1-0x0
;	SUBS            R1, R1, #1         ;R1 = R1 -1, and update the flags
	BEQ             Byte_Loop_Done

Byte_Loop_Start:
;   crc = crc ^ *buf++ << 8;
	LDRB            R2, [R0], #1            ;R0 is pmsg, R2=[R0], then R0++
	EOR             R12, R12, R2, LSL #8    ;R12 = crc

;   for (i=0; i<8; i++) {
	MOV             R4, #0x0            ;i
	CMP             R4, #0x8
	BGE             Bit_Loop_Done

Bit_Loop_Start:  ;Shift 1 bit left each time, and XOR with poly if the uppermost bit is 1
;   if (crc & 0x8000) {
	TST             R12, #0x8000      ; perform bitwise AND to check if the uppermost bit is 1
	BEQ             UpperMostBit_EZ   ; uppermost bit is Zero

;   uppermost bit is One
;   crc = (crc << 1) ^ 0x1021
	MOVW            R3,  #0x1021             ;4129, R3 is poly
	EOR             R12, R3, R12, LSL #1     ;R12 = R4 ^ (R12 << 1)
	B               Bit_Loop_End

UpperMostBit_EZ:
;   crc = crc << 1;
	MOV             R12, R12, LSL #1  ;R12= (R14 << 1), logic shift left 1 bit

Bit_Loop_End:
;   for (i=0; i<8; i++) {
	ADD             R4, R4, #1          ;r4 = i
	CMP             R4, #8
	BLT             Bit_Loop_Start

Bit_Loop_Done:
;   while(count--) {
	SUBS            R1, R1, #1     ; R1 = R1-1, update the flags
	BNE             Byte_Loop_Start

Byte_Loop_Done:
;   return crc;
    MOV             R0, R12

    ldmfd sp!, {r2-r4, r12}
	BX              lr

    .endasmfunc
