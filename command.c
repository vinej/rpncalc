#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "command.h"

extern MYFLT data_stack[];
extern MYINT data_stack_ptr;
extern MYFLT reg[];
extern MYINT lbl[];

static void push(MYFLT a)
{
	if (data_stack_ptr >= DATA_STACK_SIZE) {
		printf("stack overflow\n");
		data_stack_ptr = 0;
	}
	data_stack[data_stack_ptr++] = a;
}

static void push_no_check(MYFLT a)
{
	data_stack[data_stack_ptr++] = a;
}

static MYFLT pop()
{
	return data_stack[--data_stack_ptr];
}

static void dropfunc()
{
	if (data_stack_ptr < 1) {
		printf("stack underflow! ");
		return;
	}
	--data_stack_ptr;
}

static void dupfunc()
{
	if (data_stack_ptr < 1) {
		printf("stack underflow! ");
		return;
	}
	MYFLT val = data_stack[data_stack_ptr - 1];
	push(val);
}

static void swapfunc()
{
	if (data_stack_ptr < 2) {
		printf("stack underflow! ");
		return;
	}
	MYFLT val1 = pop();
	MYFLT val2 = pop();
	push_no_check(val1);
	push_no_check(val2);
}

static void overfunc()
{
	if (data_stack_ptr < 2) {
		printf("stack underflow! ");
		return;
	}
	push(data_stack[data_stack_ptr - 2]);
}

static void rotfunc()
{
	if (data_stack_ptr < 3) {
		printf("stack underflow! ");
		return;
	}
	/* a b c -- b c a */
	MYFLT c = pop();
	MYFLT b = pop();
	MYFLT a = pop();
	push_no_check(b);
	push_no_check(c);
	push_no_check(a);
}

static void rotnegfunc()
{
	if (data_stack_ptr < 3) {
		printf("stack underflow! ");
		return;
	}
	/* a b c -- c a b */
	MYFLT c = pop();
	MYFLT b = pop();
	MYFLT a = pop();
	push_no_check(c);
	push_no_check(a);
	push_no_check(b);
}

static void nipfunc()
{
	if (data_stack_ptr < 2) {
		printf("stack underflow! ");
		return;
	}
	MYFLT b = pop();
	pop();
	push_no_check(b);
}

static void tuckfunc()
{
	if (data_stack_ptr < 2) {
		printf("stack underflow! ");
		return;
	}
	MYFLT val1 = pop();
	MYFLT val2 = pop();
	push_no_check(val1);
	push_no_check(val2);
	push(data_stack[data_stack_ptr - 2]);
}

static void drop2func()
{
	if (data_stack_ptr < 2) {
		printf("stack underflow! ");
		return;
	}
	--data_stack_ptr;
	--data_stack_ptr;
}

static void dup2func()
{
	if (data_stack_ptr < 2) {
		printf("stack underflow! ");
		return;
	}
	MYFLT val1 = data_stack[data_stack_ptr - 2];
	MYFLT val2 = data_stack[data_stack_ptr - 1];
	push(val1);
	push(val2);
}

static void swap2func()
{
	if (data_stack_ptr < 4) {
		printf("stack underflow! ");
		return;
	}
	MYFLT val1 = pop();
	MYFLT val2 = pop();
	MYFLT val3 = pop();
	MYFLT val4 = pop();
	push_no_check(val2);
	push_no_check(val1);
	push_no_check(val4);
	push_no_check(val3);
}

static void over2func()
{
	if (data_stack_ptr < 4) {
		printf("stack underflow! ");
		return;
	}
	push(data_stack[data_stack_ptr - 4]);
	push(data_stack[data_stack_ptr - 4]);
}

static void rot2func()
{
	// ( 6 5 4 3 2 1 -- 4 3 2 1 6 5 )
	if (data_stack_ptr < 6) {
		printf("stack underflow! ");
		return;
	}
	MYFLT val1 = pop();
	MYFLT val2 = pop();
	MYFLT val3 = pop();
	MYFLT val4 = pop();
	MYFLT val5 = pop();
	MYFLT val6 = pop();
	push_no_check(val4);
	push_no_check(val3);
	push_no_check(val2);
	push_no_check(val1);
	push_no_check(val6);
	push_no_check(val5);
}

static void addfunc()
{
	push(pop() + pop());
}

static void subfunc()
{
	MYFLT subtrahend = pop();
	push(pop() - subtrahend);
}

static void mulfunc()
{
	push(pop() * pop());
}


static void divfunc()
{
	MYFLT divisor = pop();
	push(pop() / divisor);

}

static void modfunc()
{
	MYFLT modulus = pop();
	push(fmod(pop(), modulus));
}

static void lshiftfunc()
{
	unsigned MYINT shift_amt = (unsigned MYINT) pop();
	unsigned MYINT base = (unsigned MYINT) pop();
	push(base << shift_amt);
}

static void rshiftfunc()
{
	unsigned MYINT shift_amt = (unsigned MYINT) pop();
	unsigned MYINT base = (unsigned MYINT) pop();
	push(base >> shift_amt);
}

static void absfunc()
{
	push(fabs(pop()));
}

static void minfunc()
{
	MYFLT a = pop();
	MYFLT b = pop();
	MYFLT c = (a < b) ? a : b;
	push(c);
}

static void maxfunc()
{
	MYFLT a = pop();
	MYFLT b = pop();
	MYFLT c = (a > b) ? a : b;
	push(c);
}

static void roundfunc()
{
	push((MYINT)round(pop()));

}

static void floorfunc()
{
	push((MYINT)floor(pop()));
}

static void ceilfunc()
{
	push((MYINT)ceil(pop()));
}


static void powfunc()
{
	MYFLT raise = pop();
	push(pow(pop(), raise));

}

static void sqrtfunc()
{
	push(sqrt(pop()));
}

static void logfunc()
{
	push(log(pop()));
}

static void log2func()
{
	push(log2(pop()));
}

static void log10func()
{
	push(log10(pop()));
}

static void efunc()
{
	push(CONST_E);
}

static void pifunc()
{
	push(CONST_PI);
}

static void sinfunc()
{
	push(sin(pop()));
}

static void cosfunc()
{
	push(cos(pop()));
}

static void tanfunc()
{
	push(tan(pop()));
}

static void sinhfunc()
{
	push(sinh(pop()));
}

static void coshfunc()
{
	push(cosh(pop()));
}

static void tanhfunc()
{
	push(tanh(pop()));
}

static void asinfunc()
{
	push(asin(pop()));
}

static void acosfunc()
{
	push(acos(pop()));
}

static void atanfunc()
{
	push(atan(pop()));
}

static void randfunc()
{
	push((double)rand() / (double)RAND_MAX);
}

static void andfunc()
{
	push((unsigned MYINT) pop() & (unsigned MYINT) pop());
}

static void orfunc()
{
	push((unsigned MYINT) pop() | (unsigned MYINT) pop());
}

static void xorfunc()
{
	push((unsigned MYINT) pop() ^ (unsigned MYINT) pop());
}

static void notfunc()
{
	push(~(unsigned MYINT) pop());
}

/*
static void eqfunc()
{
	MYFLT num2 = pop();
	push(pop() == num2);
}

static void noteqfunc()
{
	MYFLT num2 = pop();
	push(pop() != num2);
}

static void gtfunc()
{
	MYFLT num2 = pop();
	push(pop() > num2);
}

static void ltfunc()
{
	MYFLT num2 = pop();
	push(pop() < num2);
}

static void gtefunc()
{
	MYFLT num2 = pop();
	push(pop() >= num2);
}

static void ltefunc()
{
	MYFLT num2 = pop();
	push(pop() <= num2);
}
*/

static void rclfunc()
{
	push(reg[(int)pop()]);
}

static void stofunc()
{
	MYFLT val = pop();
	MYFLT vreg = pop();
	reg[(int)vreg] = val;
}

static void negfunc()
{
	push(pop() * -1);
}

static void stcfunc()
{
	MYFLT vreg = pop();
	reg[(int)vreg] = pop();
}

static void ifeqfunc() {
	MYFLT val2 = pop();
	MYFLT val1 = pop();
	MYFLT expr2 = pop();
	MYFLT expr1 = pop();
	push(expr1 == expr2 ? val1 : val2);
}

static void ifltfunc() {
	MYFLT val2 = pop();
	MYFLT val1 = pop();
	MYFLT expr2 = pop();
	MYFLT expr1 = pop();
	push(expr1 < expr2 ? val1 : val2);
}

static void iflefunc() {
	MYFLT val2 = pop();
	MYFLT val1 = pop();
	MYFLT expr2 = pop();
	MYFLT expr1 = pop();
	push(expr1 <= expr2 ? val1 : val2);
}

static void ifgtfunc() {
	MYFLT val2 = pop();
	MYFLT val1 = pop();
	MYFLT expr2 = pop();
	MYFLT expr1 = pop();
	push(expr1 > expr2 ? val1 : val2);
}

static void ifgefunc() {
	MYFLT val2 = pop();
	MYFLT val1 = pop();
	MYFLT expr2 = pop();
	MYFLT expr1 = pop();
	push(expr1 >= expr2 ? val1 : val2);
}

static void ifnefunc() {
	MYFLT val2 = pop();
	MYFLT val1 = pop();
	MYFLT expr2 = pop();
	MYFLT expr1 = pop();
	push(expr1 != expr2 ? val1 : val2);
}

static void decfunc() {
	MYFLT val = pop();
	MYFLT vreg = pop();
	reg[(int)vreg] -= val;
}

static void incfunc() {
	MYFLT val = pop();
	MYFLT vreg = pop();
	reg[(int)vreg] += val;
}

