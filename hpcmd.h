struct TokenInfo;
typedef struct TokenInfo TokenInfo_t;

struct TokenInfo {
	char type;
	int start ; // start of the token into the buffer
	int len; // len of the token into the buffer
	int priority; 
	char assoc;
};

typedef enum {
	false = 0,
	true
} t_bool;
