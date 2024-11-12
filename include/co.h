#ifndef CO_H
#define CO_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>


#define MAX_PARAMS 8

enum {
    CO_R15 = 0,
    CO_R14,
    CO_R13,
    CO_R12,
    CO_R9,
    CO_R8,
    CO_RBP,
    CO_RDI,
    CO_RSI,
    CO_RAX,
    CO_RDX,
    CO_RCX,
    CO_RBX,
    CO_RSP,
};

enum {
    CO_STATUS_INIT = 0,
    CO_STATUS_PENDING,
    CO_STATUS_NORMAL,
    CO_STATUS_RUNNING,
    CO_STATUS_DEAD,
};

struct co_context {
    void *regs[13];
};

typedef void *(*pfunc)(int count, ...);

struct _coroutine {
    struct co_context ctx;
    char *stack;
    size_t stack_size;
    int status;
    struct _coroutine *prev;
    pfunc func;
    void *params[MAX_PARAMS];
    int param_size;
    void *ret;
};

typedef struct _coroutine coroutine;

coroutine *co_new(pfunc func, size_t stack_size, void *params[], int param_size);

void co_free(coroutine *co);

void co_ctx_make(coroutine *co);

int co_resume(coroutine *next, void **result);

int co_yield ();

extern void co_ctx_swap(struct co_context *curr, struct co_context *next);

#endif // CO_H