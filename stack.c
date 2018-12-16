#include "stack.h"
#include <stdlib.h> 
#include <stdio.h>

static const size_t stack_size_incr = 10;

struct stack {
	int top;
	void ** data;
	size_t  size;
};

stack_t * stack_create(int size) {

	stack_t *stack;

	// alloc the struct
	stack = calloc(1, sizeof(*stack));
	// alloc data
	stack->size = size;
	stack->data = malloc(stack->size * sizeof(void *));
	stack->top = -1;

	return stack;
}

static void stack_ensure_space(stack_t * stack,int add_len)
{
	if (stack == NULL || add_len == 0 || stack->top+1 < stack->size )
		return;

	// add a minimum is the incr
	if (add_len < stack_size_incr) {
		add_len = (int)stack_size_incr;
	}

	stack->size += add_len;
	stack->data = realloc(stack, stack->size * sizeof(void *));
}

void stack_destroy(stack_t * stack)
{
	if (stack == NULL) return;

	free(stack->data);
	free(stack);
}


int stack_isempty(stack_t * stack) {
	if (stack->top == -1)
		return 1;
	else
		return 0;
}

int stack_len(stack_t * stack) {
	return stack->top + 1;
}

void * stack_peek(stack_t * stack) {
	if (stack->top == -1) {
		return NULL;
	}
	else {
		return stack->data[stack->top];
	}
}

void * stack_pop(stack_t * stack) {
	void * data;

	if (!stack_isempty(stack)) {
		data = stack->data[stack->top];
		stack->top = stack->top - 1;
		return data;
	}
	else {
		printf("Could not retrieve data, Stack is empty.\n");
		return NULL;
	}
}

void stack_push(stack_t * stack,void * data) {
	stack_ensure_space(stack, 1);

	stack->top = stack->top + 1;
	stack->data[stack->top] = data;
}
