                section         .text

                global          _start
_start:

                sub             rsp, 4 * 128 * 8         ;move pointer 2 single lengths and 1 double length back
                mov             rcx, 128                 ;single length entered

                lea             rdi, [rsp + 3 * 128 * 8] ;move the diapason for 1st number to -128*8 to 0
                call            read_long                ;read 1st number from -128*8 to 0
                ;call            set_zero

                lea             rdi, [rsp + 2 * 128 * 8] ;move the diapason for 2nd number to -2*128*8 to -128*8
                call            read_long                ;read 2nd number to -2*128*8 to -128*8. It is stored in rdi
                ;call            set_zero

                mov             rdi, rsp
                mov             rcx, 256                 ;double length
                call            set_zero
                mov             rcx, 128                 ;back to single length

                mov             r15, rsp                 ;the result will be written to r15
                lea             rdi, [rsp + 3 * 128 * 8] ;the 1st number is in rdi
                lea             rsi, [rsp + 2 * 128 * 8] ;the 2nd number is in rsi

                                            ; here we have:
                                            ; rsp = r15 = rdi - 2 * 128 * 8 = rsi - 3 * 128 * 8
                                            ; st pointer = double zero number = 2nd number = 1st number

                call            mul_long_long

                mov             rdi, r15

                add             rcx, rcx                 ;length of answer is 256
                call            write_long

                mov             al, 0x0a
                call            write_char

                jmp             exit

; multiplies two long number
;    rdi -- address of multi #1 (long number)
;    rsi -- address of multi #2 (long number)
;    r15 -- zero double long
;    rcx -- length of long number in qwords
; result:
;    multiplication is written to r15

mul_long_long:

push            rdi
push            rsi
push            rax
push            rdx
push            rcx

;rsp, rbp are for stack, we can use rax, rbx, rdx, r[8-15]

;rbp = r8
;r12 = r9

xor             r9, r9          ; address outer mover
mov             r8, 128         ; counter of long number for outer cycle (dicreases once)
clc

.loopOuter:

xor             r11, r11        ;address inner mover
xor             r13, r13        ;carry value
clc

.loopInner:
mov             rax, [rdi + r11];move 1st operator and put it in rax
mov             r10, [rsi + r9] ;move 2nd operator and put it in r10
lea             r14, [r11 + r9] ;r14 = big move

mul             r10             ;multiplication of rax and r10
add             rax, r13        ;add old carry to multiplication
adc             rdx, 0          ;rdx = rdx + CF
xor             r13, r13        ;carry added, so it's 0

add             [r15 + r14], rax;put answer in answer (r15 is an address of double long number, r14 - big move).
adc             r13, rdx        ;carry = carry from prev op. + CF
add             r11, 8          ;add 8 to inner mover
dec             rcx             ;inner cycle length --

jnz             .loopInner


pop             rcx             ;restore initial length
push            rcx
add             r9, 8           ;add 8 to outer mover
dec             r8              ;decrease outer length

jnz             .loopOuter


pop             rcx
pop             rdx
pop             rax
pop             rsi
pop             rdi
ret











; adds two long number
;    rdi -- address of summand #1 (long number)
;    rsi -- address of summand #2 (long number)
;    rcx -- length of long numbers in qwords
; result:
;    sum is written to rdi
add_long_long:
                push            rdi
                push            rsi
                push            rcx

                clc
.loop:
                mov             rax, [rsi]      ;rax' = lowest 8 bytes of long number 1 (rsi is an adress of it)
                lea             rsi, [rsi + 8]  ;rsi' = rsi + 8
                adc             [rdi], rax      ;l8b of long2' += l8b of long 1 + CF. rdi is adress of l8b of long 2.
                lea             rdi, [rdi + 8]  ;go to higher bytes
                dec             rcx             ;length--
                jnz             .loop

                pop             rcx
                pop             rsi
                pop             rdi
                ret

; adds 64-bit number to long number
;    rdi -- address of summand #1 (long number)
;    rax -- summand #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    sum is written to rdi
add_long_short:
                push            rdi
                push            rcx
                push            rdx

                xor             rdx,rdx
.loop:
                add             [rdi], rax
                adc             rdx, 0
                mov             rax, rdx
                xor             rdx, rdx
                add             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rdx
                pop             rcx
                pop             rdi
                ret

; multiplies long number by a short
;    rdi -- address of multiplier #1 (long number)
;    rbx -- multiplier #2 (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    product is written to rdi
mul_long_short:
                push            rax
                push            rdi
                push            rcx

                xor             rsi, rsi
.loop:
                mov             rax, [rdi]
                mul             rbx
                add             rax, rsi
                adc             rdx, 0
                mov             [rdi], rax
                add             rdi, 8
                mov             rsi, rdx
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rdi
                pop             rax
                ret

; divides long number by a short
;    rdi -- address of dividend (long number)
;    rbx -- divisor (64-bit unsigned)
;    rcx -- length of long number in qwords
; result:
;    quotient is written to rdi
;    rdx -- remainder
div_long_short:
                push            rdi
                push            rax
                push            rcx

                lea             rdi, [rdi + 8 * rcx - 8]
                xor             rdx, rdx

.loop:
                mov             rax, [rdi]
                div             rbx
                mov             [rdi], rax
                sub             rdi, 8
                dec             rcx
                jnz             .loop

                pop             rcx
                pop             rax
                pop             rdi
                ret

; assigns a zero to long number
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
set_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep stosq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; checks if a long number is a zero
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
; result:
;    ZF=1 if zero
is_zero:
                push            rax
                push            rdi
                push            rcx

                xor             rax, rax
                rep scasq

                pop             rcx
                pop             rdi
                pop             rax
                ret

; read long number from stdin
;    rdi -- location for output (long number)
;    rcx -- length of long number in qwords
read_long:
                push            rcx
                push            rdi

                call            set_zero
.loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              .done
                cmp             rax, '0'
                jb              .invalid_char
                cmp             rax, '9'
                ja              .invalid_char

                sub             rax, '0'
                mov             rbx, 10
                call            mul_long_short
                call            add_long_short
                jmp             .loop

.done:
                pop             rdi
                pop             rcx
                ret

.invalid_char:
                mov             rsi, invalid_char_msg
                mov             rdx, invalid_char_msg_size
                call            print_string
                call            write_char
                mov             al, 0x0a
                call            write_char

.skip_loop:
                call            read_char
                or              rax, rax
                js              exit
                cmp             rax, 0x0a
                je              exit
                jmp             .skip_loop

; write long number to stdout
;    rdi -- argument (long number)
;    rcx -- length of long number in qwords
write_long:
                push            rax
                push            rcx

                mov             rax, 20
                mul             rcx
                mov             rbp, rsp
                sub             rsp, rax

                mov             rsi, rbp

.loop:
                mov             rbx, 10
                call            div_long_short
                add             rdx, '0'
                dec             rsi
                mov             [rsi], dl
                call            is_zero
                jnz             .loop

                mov             rdx, rbp
                sub             rdx, rsi
                call            print_string

                mov             rsp, rbp
                pop             rcx
                pop             rax
                ret

; read one char from stdin
; result:
;    rax == -1 if error occurs
;    rax \in [0; 255] if OK
read_char:
                push            rcx
                push            rdi

                sub             rsp, 1
                xor             rax, rax
                xor             rdi, rdi
                mov             rsi, rsp
                mov             rdx, 1
                syscall

                cmp             rax, 1
                jne             .error
                xor             rax, rax
                mov             al, [rsp]
                add             rsp, 1

                pop             rdi
                pop             rcx
                ret
.error:
                mov             rax, -1
                add             rsp, 1
                pop             rdi
                pop             rcx
                ret

; write one char to stdout, errors are ignored
;    al -- char
write_char:
                sub             rsp, 1
                mov             [rsp], al

                mov             rax, 1
                mov             rdi, 1
                mov             rsi, rsp
                mov             rdx, 1
                syscall
                add             rsp, 1
                ret

exit:
                mov             rax, 60
                xor             rdi, rdi
                syscall

; print string to stdout
;    rsi -- string
;    rdx -- size
print_string:
                push            rax

                mov             rax, 1
                mov             rdi, 1
                syscall

                pop             rax
                ret


                section         .rodata
invalid_char_msg:
                db              "Invalid character: "
invalid_char_msg_size: equ             $ - invalid_char_msg
