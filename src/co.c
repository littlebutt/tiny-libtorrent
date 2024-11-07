#include "co.h"

coroutine *co_new(pfunc func, size_t stack_size)
{
    coroutine *co = (coroutine *)malloc(sizeof(coroutine));
    if (co == NULL)
    {
        return NULL;
    }
    if (stack_size == 0)
    {
        co->stack = NULL;
    }
    else
    {
        co->stack = (char *)malloc(stack_size);
    }
    co->stack_size = stack_size;
    co->func = func;
    memset(&co->ctx, 0, sizeof(co->ctx));
    return co;
}

void co_free(coroutine *co)
{
    free(co->stack);
    free(co);
}

void co_ctx_make(coroutine *co)
{
    char *sp = co->stack + co->stack_size - sizeof(void *);
    sp = (char *)((intptr_t)sp & -16LL); // 0xff 0xff 0xff 0xf0
    *(void **)sp = (void *)co->func;
    co->ctx.regs[CO_RSP] = sp;
}
