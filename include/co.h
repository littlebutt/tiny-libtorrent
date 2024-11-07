#ifndef CO_H
#define CO_H

#include <stdlib.h>
#include <string.h>

#define MAX_PARAMS 5

enum
{
    CO_R15 = 0,
    CO_R14,
    CO_R13,
    CO_R12,
    CO_R9,
    CO_R8,
    CO_RBP,
    CO_RDI,
    CO_RSI,
    CO_RDX,
    CO_RCX,
    CO_RBX,
    CO_RSP,
};

struct co_context
{
    void *regs[13];
};

typedef void (*pfunc)();

typedef struct
{
    struct co_context ctx;
    char *stack;
    size_t stack_size;
    pfunc func;
}coroutine;

coroutine *co_new(pfunc func, size_t stack_size);

void co_free(coroutine *co);

void co_ctx_make(coroutine *co);

extern void co_ctx_swap(struct co_context *curr, struct co_context *next);

#endif // CO_H