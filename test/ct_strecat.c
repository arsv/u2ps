#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char* strecat(const char* str, ...);

#define test(res, ...) \
	r = strecat(__VA_ARGS__, NULL); \
	if(!strcmp(r, res)) \
		printf("%s:%i: OK %s\n", __FILE__, __LINE__, res);\
	else\
		printf("%s:%i: FAIL %s\n", __FILE__, __LINE__, r);\
	free(r);

int main(void)
{
	char* r;

	test("a", "a");
	test("abbb", "a", "bbb");
	test("aa/cc", "aa", "/", "cc");

	return 0;
}
