#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <Windows.h>
#include <math.h>

#include "str_builder.h"
#include "stack.h"
#include "fstack.h"
#include "hpcmd.h"
#include "command.h"
#include "command.c"

MYFLT data_stack[DATA_STACK_SIZE];
MYINT data_stack_ptr = 0;
MYFLT reg[256];
MYINT lbl[256];
MYINT fnc[256];

struct primitive {
	const char *name;
	void(*function) ();
}; 

static const struct primitive opprimitives[] = {
	{ "+", addfunc },
	{ "-", subfunc },
	{ "*", mulfunc },
	{ "/", divfunc },
	{ "^", powfunc },
	{ "%", modfunc },
	{ 0, 0 }
};

static const struct primitive primitives[] = {
{ "sto", stofunc },
{ "rcl", rclfunc },
{ "stc", stcfunc },
{ "neg", negfunc },
{ "abs", absfunc },
{ "min", minfunc },
{ "max", maxfunc },
{ "inc", incfunc },
{ "dec", decfunc },
{ "<<", lshiftfunc },
{ ">>", rshiftfunc },
{ "ifeq", ifeqfunc },
{ "ifne", ifnefunc },
{ "iflt", ifltfunc },
{ "ifgt", ifgtfunc },
{ "ifle", iflefunc },
{ "ifge", ifgefunc },
{ "and", andfunc },
{ "or", orfunc },
{ "not", notfunc },
{ "xor", xorfunc },
{ "round", roundfunc },
{ "ceil", ceilfunc },
{ "floor", floorfunc },
{ "pow", powfunc },
{ "sqrt", sqrtfunc },
{ "log", logfunc },
{ "log2", log2func },
{ "log10", log10func },
{ "pi", pifunc },
{ "ex", efunc },
{ "sin", sinfunc },
{ "cos", cosfunc },
{ "tan", tanfunc },
{ "asin", asinfunc },
{ "acos", acosfunc },
{ "atan", atanfunc },
{ "sinh", sinhfunc },
{ "cosh", coshfunc },
{ "tanh", tanhfunc },
{ "rand", randfunc },
{ "drop", dropfunc },
{ "dup", dupfunc },
{ "swap", swapfunc },
{ "over", overfunc },
{ "rot", rotfunc },
{ "-rot", rotnegfunc },
{ "nip", nipfunc },
{ "tuck", tuckfunc },
{ "2drop", drop2func },
{ "2dup", dup2func },
{ "2swap", swap2func },
{ "2over", over2func },
{ "2rot", rot2func },
{ 0, 0 }
};

void tokeninfo_fill(TokenInfo_t *tokenInfo, int pos, char type, int start, int len, int priority, char assoc) {
	TokenInfo_t * tInfo = &tokenInfo[pos];
	tInfo->type = type;
	tInfo->start = start;
	tInfo->len = len;
	tInfo->priority = 0;
	tInfo->assoc = assoc;
}

int tokenize(char * tokens, TokenInfo_t *tokenInfo) {
	t_bool is_in_function = false;
	t_bool is_in_number = false;
	t_bool is_in_string = false;
	t_bool is_in_back_slash = false;
	int currentTokenInfo = 0;
	int tokenInfoSize = 0;
	char *token = &tokens[0];

	int len = (int)strlen(tokens);
	int start = 0;
	int size = 0;
	int i = 0;
	for (i = 0; i < len; i++) 
	{
		if (is_in_string || *token == '"')
		{
			if (is_in_string && *token == '"' && !is_in_back_slash) {
				size++;
				tokeninfo_fill(tokenInfo, currentTokenInfo, 's', start, size, 0, '-');
				currentTokenInfo += 1;
				is_in_string = false;
				start = i + 1;  // next token
				size = 0;       // reset size to zero
			}
			else
			{
				is_in_string = true;
				if (is_in_back_slash || *token == '\\') {
					if (is_in_back_slash) {
						size++;
						is_in_back_slash = false;
						token++;
						continue;
					}
					else
					{
						is_in_back_slash = true;
						token++;
						continue;
					}
				}
				size++;
			}
			token++;
			continue;
		}

		if (*token == ' ')
		{
			// if we begin with a space, we can start at the next tkoen
			if (size == 0) {
				start = i + 1;
			}
			token++;
			continue;
		}

		if (is_in_function || isalpha(*token)) {
			if (isalnum(*token))
			{
				is_in_function = true;
				size++;
				token++;
				continue;
			}
			else {
				// add the token into the stack
				tokeninfo_fill(tokenInfo, currentTokenInfo, 'f', start, size, 0, '-');
				currentTokenInfo += 1;
				start = i;  // next token
				size = 0;       // reset size to zero
				is_in_function = false;
			}
		}

		if (is_in_number || isdigit(*token) || *token == '.') {
			if ((isdigit(*token) || *token == '.') && (*token != '-')) {
				is_in_number = true;
				size++;
				token++;
				continue;
			}
			else
			{
				// add the token into the stack
				tokeninfo_fill(tokenInfo, currentTokenInfo, 'n', start, size, 0, '-');
				currentTokenInfo += 1;
				start = i;    // next token
				size = 0;     // reset size to zero
				is_in_number = false;
			}
		}

		if (*token == '(' || *token == ')') {
			// add the token into the stack
			//size++; //?????????
			tokeninfo_fill(tokenInfo, currentTokenInfo, *token, start, size, 0, '-');
			currentTokenInfo += 1;
			start = i + 1;  // next token
			size = 0;       // reset size to zero
			is_in_number = false;
			is_in_function = false;
			token++;
			continue;
		}

		size++;
		TokenInfo_t * tInfo = &tokenInfo[currentTokenInfo];
		if (*token == ',') {
			tInfo->type = ',';
		}
		else {
			tInfo->type = 'o'; 
		}
		tInfo->start = start;
		tInfo->len = size;
		switch (*token) {
			case '(': tInfo->priority = 10; tInfo->assoc = 'l'; break;
			
			case '!': tInfo->priority = 9; tInfo->assoc = 'r'; break;

			case '^': tInfo->priority = 8; tInfo->assoc = 'r'; break;

			case '%': tInfo->priority = 7; tInfo->assoc = 'l'; break;
			case '*': tInfo->priority = 7; tInfo->assoc = 'l'; break;
			case '/': tInfo->priority = 7; tInfo->assoc = 'l'; break;

			case '+': tInfo->priority = 6; tInfo->assoc = 'l'; break;
			case '-': tInfo->priority = 6; tInfo->assoc = 'l'; break;
			case ')': tInfo->priority = 6; tInfo->assoc = 'l'; break;

			case '<': tInfo->priority = 5; tInfo->assoc = 'l'; break;
			case '>': tInfo->priority = 5; tInfo->assoc = 'l'; break;

			case '&': tInfo->priority = 4; tInfo->assoc = 'l'; break;

			case '|': tInfo->priority = 3; tInfo->assoc = 'l'; break;

			case ',': tInfo->priority = 2; tInfo->assoc = 'l'; break;

			case '=': tInfo->priority = 1; tInfo->assoc = 'l'; break;

			case ';': tInfo->priority = 0; tInfo->assoc = 'r'; break;
		}
		currentTokenInfo += 1;
		start = i + 1;  // next token
		size = 0;   // reset size to zero
		is_in_number = false;
		token++;
		continue;
	}

	if (is_in_function) {
		tokeninfo_fill(tokenInfo, currentTokenInfo, 'f', start, size, 0, '-');
		currentTokenInfo += 1;
	}
	else if (is_in_number) {
		tokeninfo_fill(tokenInfo, currentTokenInfo, 'n', start, size, 0, '-');
		currentTokenInfo += 1;
	}
	return currentTokenInfo;
}

void alg2rpn(char * in, char * result) {

	// the TokenInfo array won't be greater that the len of the input string
	TokenInfo_t * tokenInfo = malloc(sizeof(TokenInfo_t) * strlen(in));
	// count contains the number of TokenInfo into the string
	int count = tokenize(in, tokenInfo);

	str_builder_t * out = str_builder_create();
	stack_t * stack = stack_create(count);

	int len_stack = 0;

	TokenInfo_t * token = tokenInfo;
	for (int i = 0; i < count; i++) {
		switch (token->type)
		{
		case 'n':
			str_builder_add_str(out, in + token->start, token->len);
			str_builder_add_char(out, ' ');
			token++;
			break;
		case 's':
			str_builder_add_str(out, in + token->start, token->len);
			str_builder_add_char(out, ' ');
			token++;
			break;
		case 'f':
			if (token->len == 1) {
				char buffer[4];
				int val = (int)(*(in + token->start));
				_itoa(val, buffer, 10);
				str_builder_add_str(out, buffer, strlen(buffer));
				str_builder_add_char(out, ' ');
			}
			else {
				// if gsb, pass all token until rtn
				stack_push(stack, token);
			}
			token++;
			break;
		case 'o':
		case ',':
		{
			TokenInfo_t * topToken = (TokenInfo_t *)stack_peek(stack);
			while (stack_len(stack) != 0) {
				if (
					((topToken->type == 'f') || (topToken->type == 'o' &&
						topToken->priority > token->priority) ||
						(topToken->priority == token->priority && topToken->assoc == 'l')) &&
						(topToken->type != '(')
					) {
					topToken = ((TokenInfo_t *)stack_pop(stack));
					str_builder_add_str(out, in + topToken->start, topToken->len);
					str_builder_add_char(out, ' ');
					topToken = (TokenInfo_t *)stack_peek(stack);
				}
				else {
					break;
				}
			}
			if (token->type != ',') {
				stack_push(stack, token);
			}
			token++;
			break;
		}
		case '(':
			stack_push(stack, token);
			token++;
			break;
		case ')':
		{
			TokenInfo_t * topToken = (TokenInfo_t *)stack_peek(stack);
			while (stack_len(stack) != 0 && topToken->type != '(')
			{
				topToken = ((TokenInfo_t *)stack_pop(stack));
				str_builder_add_str(out, in + topToken->start, topToken->len);
				str_builder_add_char(out, ' ');
				topToken = (TokenInfo_t *)stack_peek(stack);
			}
			topToken = ((TokenInfo_t *)stack_pop(stack));
			token++;
			break;
		}
		}
	}

	while (stack_len(stack) != 0)
	{
		TokenInfo_t * topToken = ((TokenInfo_t *)stack_pop(stack));
		if (topToken != NULL && in[topToken->start] != ';') {
			str_builder_add_str(out, in + topToken->start, topToken->len);
			str_builder_add_char(out, ' ');
		}
	}

	strcpy(result, str_builder_peek(out));
	free(tokenInfo);
	str_builder_destroy(out);
	stack_destroy(stack);
}

double evaluate(char * tokens) {
	// the TokenInfo array won't be greater that the len of the input string

	int len = (int)strlen(tokens);

	int start = 0;
	int i = 0;
	int size = 0;
	char * token = tokens;
	int isPushStack = 1;
	for (i = 0; i < len; i++) {
		if (*token != ' ') {
			token++;
			size++;
			continue;
		}
		// function
		if (isalpha(tokens[start])) {
			if (strcmp(&tokens[start], "rtn") == 0) {
				// end if function
				return 1;
			}

			if (strcmp(&tokens[start], "fnc") == 0) {
				MYINT vname = pop();
				fnc[(int)vname] = i; // next token
				start = i + 1;
				size = 0;
				token++;
				while (1) {
					while (*token != ' ') {
						size++;	token++; i++;
					}
					*token = 0;
					if (strcmp(&tokens[start], "rtn") == 0) {
						break;
					}
					*token = ' ';
					token++;
					start = i + 2;
					++i;
					size = 0;
				}
				start = i + 2;
				size = 0;
				i++;
				*token = ' ';
				token++;
				continue;
			}

			if (strcmp(&tokens[start], "gsb") == 0) {
				MYFLT vname = pop();
				int oldi = i;
				i = fnc[(int)vname];
				start = i + 1;
				size = 0;
				token = &tokens[i + 1];
				evaluate(token);
				i = oldi;
				start = i + 1;
				size = 0;
				token = &tokens[i + 1];
				continue;
			}

			if (strcmp(&tokens[start], "lbl") == 0) {
				MYFLT vreg = pop();
				lbl[(int)vreg] = i; // next token
				start = i + 1;
				size = 0;
				*token = ' ';
				token++;
				continue;
			}

			if (strcmp(&tokens[start], "jnz") == 0) {
				*token = ' ';
				MYFLT vreg = pop();
				MYFLT vlbl = pop();
				if (reg[(int)vreg] != 0.0) {
					i = lbl[(int)vlbl];
					start = i + 1;
					size = 0;
					token = &tokens[i+1];
					continue;
				}
				else {
					start = i + 1;
					size = 0;
					token++;
					continue;
				}
			}

			const struct primitive *pr = primitives;
			while (pr->name != 0) {
				if (strcmp(pr->name, &tokens[start]) == 0) {
					(*(pr->function)) ();
					break;
				}
				pr++;
			}
			start = i + 1;
			size = 0;
			*token = ' ';
			token++;
			continue;
		}
		// number
		else if (isdigit(tokens[start]) || tokens[start] == '.') {
			*token = 0;
			push(atof(&tokens[start]));
			*token = ' ';
			start = i + 1;
			size = 0;
			token++;
			continue;
		}
		//op
		else
		{
			const struct primitive *opr = opprimitives;
			*token = 0;
			while (opr->name != 0) {
				if (strcmp(opr->name, &tokens[start]) == 0) {
					(*(opr->function)) ();
					break;
				}
				opr++;
			}
			start = i + 1;
			size = 0;
			*token = ' ';
			token++;
			continue;
		}
	}
	double result = pop();
	return result;
}

long now() {
	// struct to request the time
	SYSTEMTIME str_t;
	GetSystemTime(&str_t);

	printf("Year:%d\nMonth:%d\nDate:%d\nHour:%d\nMin:%d\nSecond:% d\nMill:%d\n"
		, str_t.wYear, str_t.wMonth, str_t.wDay
		, str_t.wHour, str_t.wMinute, str_t.wSecond, str_t.wMilliseconds);

	return str_t.wMilliseconds;
}

int setTokenSep(char * tokens) {
	char * token = tokens;
	int i;
	while (*token != 0) {
		while (*token != ' ') {
			token++;
		}
		*token = 0;
		token++;
	}
	return token - tokens;
}

int main()
{
	int max = 10000;
	double result = 0;
	long start = now();
	for (int i = 0; i < max; i++) {
		char * out = malloc(1000);
		//char * infix = "sto(i,2) ; sto(c,neg(1)); lbl(k) ; rcl(c)+ifeq(12*12/4+rcl(i),36,323+rcl(i),423) ; stc(c) ; dec(i,1) ; jnz(k,i); rcl(c)"; // == 845  , 70msec
		char * infix = "sto(i,2) ; fnc(b); 100/50 ; rtn; fnc(a); 100/50*5 ; rtn; 2*3*gsb(a)*gsb(b)"; // == 845  , 70msec
																																				  //char * infix = "sto(a,12);sto(b,23); rcl(a) * rcl(b)"; // == 845  , 70msec
																																				  //char * infix = "cos(34)";
		//char * infix = "23 / 23 * 2 * sin(cos(34)) + 5 + 23 / 23 * 2 * sin(cos(34)) + 5 + 23 / 23 * 2 * sin(cos(34)) + 5 + 23 / 23 * 2 * sin(cos(34)) + 5";
		//char * infix = "sto(10,1);sto(2,0);lbl(1);rcl(2)+ifeq(12*12/4,36,323*rcl(1),423);stc(2);dec(1,1);jnz(1,1)";
		// 12 12 * 4 / 48 323 423 ifeq
		// 323+2+323+1
		/* 23 23 / 2 * 34 cos sin * 5 + 23 23 / 2 * 34 cos sin * + 5 + 23 23 / 2 * 34 cos sin * + 5 + 23 23 / 2 * 34 cos sin * + 5 + */
		alg2rpn(infix, out);
		//printf("\nrpn = %s\n", out);
		//int len = setTokenSep(out);
		result = evaluate(out);
		free(out);
	}
	long end = now();
	printf("\nTolal of millisecond for 10,000 :  %d , %f", end - start, result);

	getchar();
}