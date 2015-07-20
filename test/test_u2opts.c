#include "test.h"

extern char* inputname;
extern char* outputname;

extern void handle_args(int argc, char** argv);

/* handle(...) calls handle_opts(...), all the fuss is about turning "..." into a char*[] */

#define argsname(key) args ## key
#define handle__(key, ...) \
	char* argsname(key)[] = { "./u2ps", __VA_ARGS__, NULL };\
	handle_args(sizeof(argsname(key))/sizeof(char*), argsname(key))
#define handle(...) handle__(__LINE__, __VA_ARGS__)

int main(void)
{
	handle("--");
	testnull(inputname);
	testnull(outputname);

	handle("-E", "input");
	teststr(inputname, "input");
	testnull(outputname);
	
	return 0;
}
