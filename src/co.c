#include "co.h"

coroutine *g_curr_co = NULL;

coroutine *co_new(pfunc func, size_t stack_size)
{
    coroutine *co = (coroutine *)calloc(sizeof(coroutine), 1);
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
        co->stack = (char *)calloc(stack_size, 1);
    }
    co->stack_size = stack_size;
    co->prev = NULL;
    co->status = CO_STATUS_INIT;
    co->func = func;
    memset(&co->ctx, 0, sizeof(co->ctx));
    return co;
}

void co_free(coroutine *co)
{
    free(co->stack);
    free(co);
}

// XXX: __builtin_apply in GCC
void _co_entrance(coroutine *co)
{
    co->func(); // TODO: Sometimes triggers Unknown Signal
    co->status = CO_STATUS_DEAD;
    co_yield ();
}

void co_ctx_make(coroutine *co)
{
    char *sp = co->stack + co->stack_size - sizeof(void *);
    sp = (char *)((intptr_t)sp & -16LL); // 0xff 0xff 0xff 0xf0
    *(void **)sp = (void *)_co_entrance;
    co->ctx.regs[CO_RSP] = sp;
    co->ctx.regs[CO_RAX] = (char *)_co_entrance;
    co->ctx.regs[CO_RCX] = co;
}

void _check_init()
{
    if (g_curr_co == NULL)
    {
        g_curr_co = co_new(NULL, 0, NULL, 0);
        g_curr_co->status = CO_STATUS_RUNNING;
    }
}

int co_resume(coroutine *next)
{
    _check_init();
    switch (next->status)
    {
    case CO_STATUS_INIT: {
        co_ctx_make(next);
    }
    case CO_STATUS_PENDING: {
        break;
    }
    default: {
        return 0;
    }
    }
    coroutine *curr = g_curr_co;
    g_curr_co = next;
    next->prev = curr;
    curr->status = CO_STATUS_NORMAL;
    next->status = CO_STATUS_RUNNING;
    co_ctx_swap(&curr->ctx, &next->ctx);
    return 1;
}

int co_yield ()
{
    _check_init();
    coroutine *curr = g_curr_co;
    coroutine *prev = curr->prev;

    if (!prev)
    {
        return 0;
    }
    g_curr_co = prev;
    if (curr->status != CO_STATUS_DEAD)
    {
        curr->status = CO_STATUS_PENDING;
    }
    prev->status = CO_STATUS_RUNNING;
    co_ctx_swap(&curr->ctx, &prev->ctx);

    return 0;
}
