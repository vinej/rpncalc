
struct RpnInfo;
typedef struct RpnInfo RpnInfo_t;

struct RpnInfo {
	char type;
	int start; // start of the token into the buffer
	int len; // len of the token into the buffer
};

struct stackinfo;
typedef struct stackinfo stackinfo_t;

stackinfo_t * stack_create(int);
void stackinfo_destroy(stackinfo_t *);
int stackinfo_isempty(stackinfo_t *);
int stackinfo_isfull(stackinfo_t *);
int stackinfo_len(stackinfo_t *);
void * stackinfo_peek(stackinfo_t *);
void * stackinfo_pop(stackinfo_t *);
void stackinfo_push(stackinfo_t *, char, int, int);


