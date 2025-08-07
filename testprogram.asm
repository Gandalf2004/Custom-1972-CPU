;Copyright © Martin H. Sharp; August 2025 testprogram.asm
;---------------------------------------------------
; Entry point because the Cpu starts execution with address 0x00

;----------------------------------------------------
        CALL MAIN
        HLT            ; Halt program

;--------------------------------------------------
; SUBROUTINE: ADD@
;   On entry stack (SP↓) holds:
;     [SP]     = return PC
;     [SP+1]   = operand2
;     [SP+2]   = operand1
;
;   Returns with RA = operand1 + operand2
;   SP is unchanged so RET pops the correct return PC.
;--------------------------------------------------

ADD@:
    ;— fetch operand2 into RC (SP+1) —
    MOVR RA SP
    ADD   1
    MOVR RB RA
    MOVA_PTRB
    ;OUT
    MOVR RC RA

    ;— fetch operand1 into RB (SP+2) —
    MOVR RA SP
    ADD   2
    MOVR RB RA
    MOVA_PTRB
    ;OUT
    MOVR RB RA

    MOVR RA RB
    ADDR RC
    ;OUT

    RET


;--------------------------------------------------
; MAIN PROGRAM
;   Compute 5 + 3 and print the result via OUT.
;--------------------------------------------------

MAIN:
        ;— Push operand1  = 3 —
        LDIMA   3
        PUSH    RA

        ;— push operand2 = 5 —
        LDIMA   5
        PUSH    RA

        ;— call Add; result comes back in RA —
        CALL    ADD@

        OUT             ; print RA (should be 8)
        RET