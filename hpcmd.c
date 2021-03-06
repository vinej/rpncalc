
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "str_builder.h"
#include "stack.h"
#include "fstack.h"
#include "hpcmd.h"
#include "command.h"
#include "command.c"

#define DEF "def"
#define END "end"
#define FNC "fnc"
#define LBL "lbl"
#define JMP "jmp"
#define BRK "brk"
#define JNZ "jnz"

#define INT_LEN (10)
#define HEX_LEN (8)
#define BIN_LEN (32)
#define OCT_LEN (11)

static char *  itoa ( int value, char * str, int base )
{
    int i,n =2,tmp;
    char buf[BIN_LEN+1];


    switch(base)
    {
        case 16:
            for(i = 0;i<HEX_LEN;++i)
            {
                if(value/base>0)
                {
                    n++;
                }
            }
            snprintf(str, n, "%x" ,value);
            break;
        case 10:
            for(i = 0;i<INT_LEN;++i)
            {
                if(value/base>0)
                {
                    n++;
                }
            }
            snprintf(str, n, "%d" ,value);
            break;
        case 8:
            for(i = 0;i<OCT_LEN;++i)
            {
                if(value/base>0)
                {
                    n++;
                }
            }
            snprintf(str, n, "%o" ,value);
            break;
        case 2:
            for(i = 0,tmp = value;i<BIN_LEN;++i)
            {
                if(tmp/base>0)
                {
                    n++;
                }
                tmp/=base;
            }
            for(i = 1 ,tmp = value; i<n;++i)
            {
                if(tmp%2 != 0)
                {
                    buf[n-i-1] ='1';
                }
                else
                {
                    buf[n-i-1] ='0';
                }
                tmp/=base;
            }
            buf[n-1] = '\0';
            strcpy(str,buf);
            break;
        default:
            return NULL;
    }
    return str;
}

static const long hextable[] = {
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1, 0,1,2,3,4,5,6,7,8,9,-1,-1,-1,-1,-1,-1,-1,10,11,12,13,14,15,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
};

long hexdec(unsigned const char *hex) {
	long ret = 0;
	while (*hex && ret >= 0) {
		ret = (ret << 4) | hextable[*hex++];
	}
	return ret;
}

int bin2int(const char *bin)
{
	int i=0, j=0;
	j = sizeof(int) * 8;
	while ((j--) && ((*bin == '0') || (*bin == '1'))) {
		i <<= 1;
		if (*bin == '1') i++;
		bin++;
	}
	return i;
}

MYFLT data_stack[DATA_STACK_SIZE];
MYINT data_stack_ptr = 0;
MYFLT reg[256];
char* lbl[256];
char* fnc[256];

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
	{ "<", ltfunc },
	{ ">", gtfunc },
	{ "=", eqfunc },
	{ "|", bitorfunc },
	{ "&", bitandfunc },
	{ "~", bitnotfunc },
	{ "@", bitxorfunc },
	//{ "#", binay nummer },
	//{ "$", hexa number },
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
	t_bool is_in_binary = false;
	t_bool is_in_hexa = false;
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

		// hexadecimal
		if (*token == '$' || is_in_hexa) {
			if (!is_in_hexa) {
				is_in_hexa = true;
				size++;
				token++;
				continue;
			}
			else {
				if (isdigit(*token) || (*token >= 65 && *token <= 70) || (*token >= 97 && *token <= 102)) {
					token++;
					size++;
					continue;
				}
				else
				{
					//convert hexa to decimal
					tokeninfo_fill(tokenInfo, currentTokenInfo, 'h', start, size, 0, '-');
					currentTokenInfo += 1;
					start = i;    // next token
					size = 0;     // reset size to zero
					is_in_hexa = false;
				}
			}
		}

		if (*token == '#' || is_in_binary) {
			if (!is_in_binary) {
				is_in_binary = true;
				token++;
				size++;
				continue;
			}
			else
			{
				if (*token == 48 || *token == 49) {
					token++;
					size++;
					continue;
				}
				else
				{
					//convert hexa to decimal
					tokeninfo_fill(tokenInfo, currentTokenInfo, 'b', start, size, 0, '-');
					currentTokenInfo += 1;
					start = i;    // next token
					size = 0;     // reset size to zero
					is_in_binary = false;
				}
			}
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
			case '~': tInfo->priority = 9; tInfo->assoc = 'r'; break;
			case '@': tInfo->priority = 9; tInfo->assoc = 'r'; break;

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
	else if (is_in_hexa) {
		tokeninfo_fill(tokenInfo, currentTokenInfo, 'h', start, size, 0, '-');
		currentTokenInfo += 1;
	}
	else if (is_in_binary) {
		tokeninfo_fill(tokenInfo, currentTokenInfo, 'b', start, size, 0, '-');
		currentTokenInfo += 1;
	}
	else if (is_in_number) {
		tokeninfo_fill(tokenInfo, currentTokenInfo, 'n', start, size, 0, '-');
		currentTokenInfo += 1;
	}
	return currentTokenInfo;
}

void alg2rpn(char * in, char * result) {
	TokenInfo_t * token = NULL;
	stack_t * stack = NULL;
	TokenInfo_t * tokenInfo = NULL;
	str_builder_t * out = NULL;
    // the TokenInfo array won't be greater that the len of the input string
    // count contains the number of TokenInfo into the string
    tokenInfo = malloc(sizeof(TokenInfo_t) * strlen(in));
    int count = tokenize(in, tokenInfo);
    out = str_builder_create();
    stack = stack_create(count);

    int len_stack = 0;

    token = tokenInfo;
    for (int i = 0; i < count; i++) {
        switch (token->type)
        {
        case 'n':
            str_builder_add_str(out, in + token->start, token->len);
            str_builder_add_char(out, ' ');
            token++;
            break;
        case 'h':
        {
            char temp[200];
            strncpy(temp, in + token->start + 1, token->len - 1);
            temp[token->len - 1] = 0;
            long val = hexdec(temp);
            char temp2[200];
            itoa(val, temp2, 10);
            str_builder_add_str(out, temp2, strlen(temp2));
            str_builder_add_char(out, ' ');
            token++;
            break;
        }
        case 'b':
        {
            char temp3[200];
            strncpy(temp3, in + (token->start + 1), token->len - 1);
            temp3[token->len - 1] = 0;
            int val = bin2int(temp3);
            char temp4[200];
            itoa(val, temp4, 10);
            str_builder_add_str(out, temp4, strlen(temp4));
            str_builder_add_char(out, ' ');
            token++;
            break;
        }
        case 'f':
            if (token->len == 1) {
                char buffer[4];
                int val = (int)(*(in + token->start));
                itoa(val, buffer, 10);
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
    free(tokenInfo); tokenInfo = NULL;
    str_builder_destroy(out); out = NULL;
    stack_destroy(stack); stack = NULL;

    if (tokenInfo != NULL) free(tokenInfo);
    if (out != NULL) str_builder_destroy(out);
    if (stack != NULL) stack_destroy(stack);
}

char * findAfterToken(char * token, char * find) {
	char * start = token;
	while(1) {
		while (*token != 0) {
			token++;
		}
		if (*(token + 1) == 0) {
			printf("%s", "Erreur nonrtn");
		}

		if (strcmp(start, find) == 0) {
			return ++token;
		}
		token++;
		start = token;
	}
	printf("no RTN token)");
	exit(1);
}

double evaluate(char * tokens) {
	// the TokenInfo array won't be greater that the len of the input string
	char * token = tokens;
	char * startToken = token;
	t_bool isEnd = false;
	while(!isEnd) {
		startToken = token;
		// move to the end of the current token
		while (*token != 0) {
			token++;
		}
		if (*(token + 1) == 0) isEnd = true;

		// function
		if (isalpha(*startToken)) {

			// Manage the RTN function. We only return to the calling code
			// the stack contains the calculated values
			if (strcmp(startToken, END) == 0) {
				// end if function
				return 1;
			}

			if (strcmp(startToken, BRK) == 0) {
				MYFLT argv1 = pop();
				MYFLT argv2 = pop();
				if (argv1 == reg[(int)argv2]) {
					token = findAfterToken(token, JMP);
					continue;
				}
			}

			// If it a function, we do not execute the code. We simply keep the position of the function
			// find the end of the function and continue the execution
			if (strcmp(startToken, DEF) == 0) {
				MYINT vname = (MYINT)pop();
				// place the function just after the fnc definition
				fnc[(int)vname] = ++token;
				// continue after the RTN function.
				// A function is not evaluate the first time is parsed
				token = findAfterToken(token, END);
				continue;
			}

			// a call an existing function. The execution will restart at the beginning
			// of the function.
			if (strcmp(startToken, FNC) == 0) {
				MYFLT vname = pop();
				evaluate(fnc[(int)vname]);
				token++;
				continue;
			}

			if (strcmp(startToken, LBL) == 0) {
				MYFLT vreg = pop();
				token++;
				lbl[(int)vreg] = token; // next token
				continue;
			}

			if (strcmp(startToken, JMP) == 0) {
				MYFLT vlbl = pop();
				token = lbl[(int)vlbl];
				continue;
			}

			if (strcmp(startToken, JNZ) == 0) {
				MYFLT vlbl = pop();
				MYFLT vreg = pop();
				reg[((int)vreg)] -= 1;
				if (reg[((int)vreg)] != 0) {
					token = lbl[(int)vlbl];
					continue;
				}
				token++;
				continue;
			}

			const struct primitive *pr = primitives;
			while (pr->name != 0) {
				if (strcmp(pr->name, startToken) == 0) {
					(*(pr->function)) ();
					break;
				}
				pr++;
			}
			token++;
			continue;
		}
		// number
		else if (isdigit(*startToken) || *startToken == '.') {
			push(atof(startToken));
			token++;
			continue;
		}
		//op
		else
		{
			const struct primitive *opr = opprimitives;
			while (opr->name != 0) {
				if (strcmp(opr->name, startToken) == 0) {
					(*(opr->function)) ();
					break;
				}
				opr++;
			}
			token++;
			continue;
		}
	}
	double result = pop();
	return result;
}

int setTokenSep(char * tokens) {
	char * token = tokens;
	int i = 0;
	while (*token != 0) {
		while (*token != ' ') {
			token++;
			i++;
		}
		*token = 0;
		token++;
		i++;
	}
	return i;
}

int main()
{
	int max = 1;
	double result = 0;
	//long start = now();
	for (int i = 0; i < max; i++) {
		char * out = malloc(1000);
		//char * infix = "sto(i,2) ; sto(c,neg(1)); lbl(k) ; rcl(c)+ifeq(12*12/4+rcl(i),36,323+rcl(i),423) ; stc(c) ; dec(i,1) ; brk(i,0); jmp(k); rcl(c)"; // == 845  , 89msec
		//char * infix = "sto(i,2) ; sto(c,neg(1)); lbl(k) ; rcl(c)+ifeq(12*12/4+rcl(i),36,323+rcl(i),423) ; stc(c) ; jnz(i,k); rcl(c)"; // == 845  , 70msec
 	    //char * infix = "sto(i,2) ; def(b); 100/50 ; end; def(a); 100/50*5 ; end; 2*3*fnc(a)*fnc(b)"; // == 120  , 40msec
		char * infix = "sto(i,2) ; def(b); 100/50 ; end; def(a); 100/50*5 ; end; 2*3*fnc(a)*fnc(b)/fnc(b)+12345"; // == 12405  , 45msec
		//char * infix = "5 % 2";
		//char * infix = "cos(34)";
		//char * infix = "5 | 2";
		//char * infix = "5 & 5";
		//char * infix = "5 @ 5";
		//char * infix = "~5";
		//char * infix = "$ff + $1";
		//char * infix = "cos(34)";
		//char * infix = "23 / 23 * 2 * sin(cos(34)) + 5 + 23 / 23 * 2 * sin(cos(34)) + 5 + 23 / 23 * 2 * sin(cos(34)) + 5 + 23 / 23 * 2 * sin(cos(34)) + 5"; //13.997312
		// 12 12 * 4 / 48 323 423 ifeq
		// 323+2+323+1
		/* 23 23 / 2 * 34 cos sin * 5 + 23 23 / 2 * 34 cos sin * + 5 + 23 23 / 2 * 34 cos sin * + 5 + 23 23 / 2 * 34 cos sin * + 5 + */
		alg2rpn(infix, out);
		//printf("\nrpn = %s\n", out);
		int len = setTokenSep(out);
		result = evaluate(out);
		free(out);
	}
	//long end = now();
	printf("\nTolal of millisecond for %i : %f", max, result);
}
