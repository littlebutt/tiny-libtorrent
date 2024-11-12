public co_ctx_swap

_text segment

co_ctx_swap proc
    mov [rcx + 104], rsp
    mov [rcx + 96], rbx
    mov [rcx + 88], rcx
    mov [rcx + 80], rdx
    mov [rcx + 72], rax
    mov [rcx + 64], rsi
    mov [rcx + 56], rdi
    mov [rcx + 48], rbp
    mov [rcx + 40], r8
    mov [rcx + 32], r9
    mov [rcx + 24], r12
    mov [rcx + 16], r13
    mov [rcx + 8], r14
    mov [rcx], r15

    mov r15, [rdx]
    mov r14, [rdx + 8]
    mov r13, [rdx + 16]
    mov r12, [rdx + 24]
    mov r9, [rdx + 32]
    mov r8, [rdx + 40]
    mov rbp, [rdx + 48]
    mov rdi, [rdx + 56]
    mov rsi, [rdx + 64]
    mov rax, [rdx + 72]
    mov rcx, [rdx + 88]
    mov rbx, [rdx + 96]
    mov rsp, [rdx + 104]
    mov rdx, [rdx + 80]

    ret
co_ctx_swap endp

_text ends
end