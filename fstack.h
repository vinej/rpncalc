#include <stdio.h>

struct fstack;
typedef struct fstack fstack_t;

fstack_t * fstack_create(int size);
void fstack_destroy(fstack_t *);
int fstack_isempty(fstack_t *);
int fstack_isfull(fstack_t *);
int fstack_len(fstack_t *);
double fstack_peek(fstack_t *);
double fstack_pop(fstack_t *);
void fstack_push_str(fstack_t *, char *);
void fstack_push_dbl(fstack_t *, double);

