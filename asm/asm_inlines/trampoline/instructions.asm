/* Simple case (when there are <= 5 arguments) */

# Move all registers with args by one
\x4d\x89\xc1 # mov r9 r8
\x49\x89\xc8 # mov r8 rcx
\x48\x89\xd1 # mov rcx, rdx
\x48\x89\xf2 # mov rdx, rsi
\x48\x89\xfe # mov rsi, rdi
\x48\xbf_1 # mov rdi, imm; _1 -> do_call parameter ie functor

\x48\xb8_2 # mov rax, imm; _2 -> do_call address
\xff\xe0 # jmp rax ie jump to address in rax ans start executing do_call

/* Hard case (when there are >= 6 arguments) */
