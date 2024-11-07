.global co_ctx_swap

#if !defined( __APPLE__ ) && !defined( __MINGW32__ )
    .def co_ctx_swap
    .type co_ctx_swap, @function
    .endef
#endif

co_ctx_swap:
#if defined( __x86_64__ )
    movq %rsp, 96(%rdi)
    movq %rbx, 88(%rdi)
    movq %rcx, 80(%rdi)
    movq %rdx, 72(%rdi)
    movq %rsi, 64(%rdi)
    movq %rdi, 56(%rdi)
    movq %rbp, 48(%rdi)
    movq %r8, 40(%rdi)
    movq %r9, 32(%rdi)
    movq %r12, 24(%rdi)
    movq %r13, 16(%rdi)
    movq %r14, 8(%rdi)
    movq %r15, (%rdi)

    movq (%rsi), %r15
    movq 8(%rsi), %r14
    movq 16(%rsi), %r13
    movq 24(%rsi), %r12
    movq 32(%rsi), %r9
    movq 40(%rsi), %r8
    movq 48(%rsi), %rbp
    movq 56(%rsi), %rdi
    movq 72(%rsi), %rdx
    movq 80(%rsi), %rcx
    movq 88(%rsi), %rbx
    movq 96(%rsi), %rsp
    movq 64(%rsi), %rsi

    ret

#elif defined(__i386__)
    movl %esp, 24(%edi)
    movl %ebx, 20(%edi)
    movl %ecx, 16(%edi)
    movl %edx, 12(%edi)
    movl %esi, 8(%edi)
    movl %edi, 4(%edi)
    movl %ebp, (%edi)

    movl 24(%esi), %esp
    movl 20(%esi), %ebx
    movl 16(%esi), %ecx
    movl 12(%esi), %edx
    movl 8(%esi), %esi
    movl 4(%esi), %edi
    movl (%esi), %ebp

    ret
#endif