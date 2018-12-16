#include "stackinfo.h"
#include <stdlib.h> 
#include <stdio.h>

static const size_t stack_size_incr = 1;

struct stackinfo {
	int top;
	RpnInfo_t ** data;
	size_t  size;
};

stackinfo_t * stack_create() {

	stackinfo_t *stack;

	// alloc the struct
	stack = calloc(1, sizeof(*stack));
	// alloc data
	stack->size = 1;
	stack->data = malloc(stack->size * sizeof(RpnInfo_t));
	stack->top = -1;

	return stack;
}

static void stack_ensure_space(stackinfo_t * stack,int add_len)
{
	if (stack == NULL || add_len == 0 || stack->top+1 < stack->size )
		return;

	// add a minimum is the incr
	if (add_len < stack_size_incr) {
		add_len = stack_size_incr;
	}

	stack->size += add_len;
	stack->data = realloc(stack, stack->size * sizeof(RpnInfo_t));
}

void stack_destroy(stackinfo_t * stack)
{
	if (stack == NULL) return;

	free(stack->data);
	free(stack);
}


int stack_isempty(stackinfo_t * stack) {
	if (stack->top == -1)
		return 1;
	else
		return 0;
}

int stack_len(stackinfo_t * stack) {
	return stack->top + 1;
}

RpnInfo_t * stack_peek(stackinfo_t * stack) {
	if (stack->top == -1) {
		return NULL;
	}
	else {
		return stack->data[stack->top];
	}
}

RpnInfo_t * stackinfo_pop(stackinfo_t * stack) {
	RpnInfo_t * data;

	if (!stack_isempty(stack)) {
		data = &stack->data[stack->top];
		stack->top = stack->top - 1;
		return data;
	}
	else {
		printf("Could not retrieve data, Stack is empty.\n");
		return NULL;
	}
}

void stackinfo_push(stackinfo_t * stack, char type, int start, int len) {

	stack_ensure_space(stack, 1);
	RpnInfo_t tinfo;
	tinfo.type = type;
	tinfo.start = start;
	tinfo.len = len;

	stack->top = stack->top + 1;
	*stack->data[stack->top] = tinfo;
}

