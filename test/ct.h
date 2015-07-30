#include <stdio.h>
#include <string.h>

#define testlog(fmt, ...) printf("# " fmt "\n", ## __VA_ARGS__);

/* test if success (zero) */
#define testzero(a) {\
	int r = a;\
	if(r)\
		printf("%s:%i: FAIL(%i) %s\n", __FILE__, __LINE__, r, #a);\
	else\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #a);\
}

/* assert (test if non-zero) */
#define test(a) \
	if(a)\
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, #a);\
	else\
		printf("%s:%i: FAIL %s\n", __FILE__, __LINE__, #a);

/* compare strings */
#define teststr(a,b) {\
	const char* r = a;\
	if(b == NULL && r == NULL)\
		printf("%s:%i: OK %s = NULL\n", __FILE__, __LINE__, #a);\
	else if(b == NULL) \
		printf("%s:%i: FAIL %s = \"%s\" not NULL\n", __FILE__, __LINE__, #a, r);\
	else if(r == NULL) \
		printf("%s:%i: FAIL %s = NULL\n", __FILE__, __LINE__, #a);\
	else if(strcmp(r, b)) \
		printf("%s:%i: FAIL %s = \"%s\" not \"%s\"\n", __FILE__, __LINE__, #a, r, b);\
	else\
		printf("%s:%i: OK %s = \"%s\"\n", __FILE__, __LINE__, #a, b);\
}

#define testeq(val, exp, fmt) {\
	if((val) == (exp)) \
		printf("%s:%i: OK %s == %s\n", __FILE__, __LINE__, #val, #exp);\
	else \
		printf("%s:%i: FAIL %s = " fmt " != %s\n", __FILE__, __LINE__, #val, val, #exp);\
}

#define testeqi(val, exp) testeq(val, exp, "%i")

#define testnull(exp) {\
	if((exp) == NULL) \
		printf("%s:%i: OK %s is NULL\n", __FILE__, __LINE__, #exp);\
	else\
		printf("%s:%i FAIL %s is not NULL\n", __FILE__, __LINE__, #exp);\
}

void die(const char* fmt, ...) __attribute__((noreturn));

int nocall(const char* f);
#define NOCALL(f) void f() { nocall(#f); }
