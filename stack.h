#include <stdio.h>

struct stack;
typedef struct stack stack_t;

stack_t * stack_create(int size);
void stack_destroy(stack_t *);
int stack_isempty(stack_t *);
int stack_isfull(stack_t *);
int stack_len(stack_t *);
void * stack_peek(stack_t *);
void * stack_pop(stack_t *);
void stack_push(stack_t *,void * data);

