#include "fstack.h"
#include <stdlib.h> 
#include <stdio.h>

static const size_t stack_size_incr = 10;

struct fstack {
	int top;
	double * data;
	size_t  size;
};

fstack_t * fstack_create(int size) {

	fstack_t *stack;

	// alloc the struct
	stack = calloc(1, sizeof(*stack));
	// alloc data
	stack->size = size;
	stack->data = malloc(stack->size * sizeof(double));
	stack->top = -1;

	return stack;
}

static void fstack_ensure_space(fstack_t * stack,int add_len)
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

void fstack_destroy(fstack_t * stack)
{
	if (stack == NULL) return;

	free(stack->data);
	free(stack);
}


int fstack_isempty(fstack_t * stack) {
	if (stack->top == -1)
		return 1;
	else
		return 0;
}

int fstack_len(fstack_t * stack) {
	return stack->top + 1;
}

double fstack_peek(fstack_t * stack) {
	if (stack->top == -1) {
		return 0;
	}
	else {
		return stack->data[stack->top];
	}
}

double fstack_pop(fstack_t * stack) {
	double data;

	if (!fstack_isempty(stack)) {
		data = stack->data[stack->top];
		stack->top = stack->top - 1;
		return data;
	}
	else {
		printf("Could not retrieve data, Stack is empty.\n");
		return 0;
	}
}

void fstack_push_str(fstack_t * stack, char * data) {
	fstack_ensure_space(stack, 1);

	stack->top = stack->top + 1;
	stack->data[stack->top] = atof(data);
}

void fstack_push_dbl(fstack_t * stack, double data) {
	fstack_ensure_space(stack, 1);

	stack->top = stack->top + 1;
	stack->data[stack->top] = data;
}
