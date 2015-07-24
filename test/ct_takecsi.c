#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "u2ps.h"

#define MAX 20

int called;
int retcmd;
int retargn;
int retargs[MAX];

extern int take_csi(char* ptr);

void handle_csi(int cmd, int argn, int* args)
{
	called = 1;
	retcmd = cmd;
	if(argn >= MAX) {
		retargn = -1;
		return;
	} else {
		retargn = argn;
		memcpy(retargs, args, argn*sizeof(int));
	}
}

int match(int arg1n, int* arg1s, int arg2n, int* arg2s)
{
	int i;

	if(arg1n != arg2n)
		return 0;

	for(i = 0; i < arg1n && i < arg2n; i++)
		if(arg1s[i] != arg2s[i])
			return 0;

	return 1;
}

void dump(int argn, int* args)
{
	int i;

	printf("{ ");
	for(i = 0; i < argn; i++)
		printf("%i ", args[i]);
	printf("}");
}

void test(char* file, int line, char* seq, int tst, int tstcmd, int tstargn, ...)
{
	int tstargs[tstargn];
	va_list ap;
	int i;

	va_start(ap, tstargn);
	for(i = 0; i < tstargn; i++)
		tstargs[i] = va_arg(ap, int);
	va_end(ap);

	called = 0;
	retcmd = 0;
	retargn = 0;
	memset(retargs, 0, sizeof(retargs));

	int ret = take_csi(seq);

	if(ret != tst) {
		printf("%s:%i: FAIL ret %i expected %i\n", file, line, ret, tst);
		return;
	}

	if(retcmd != tstcmd) {
		printf("%s:%i: FAIL cmd %i expected %i\n", file, line, retcmd, tstcmd);
		return;
	}

	if(!match(retargn, retargs, tstargn, tstargs)) {
		printf("%s:%i: FAIL return ", file, line);
		dump(retargn, retargs);
		printf(" expected ");
		dump(tstargn, tstargs);
		printf("\n");
		return;
	}

	printf("%s:%i: OK ", file, line); dump(retargn, retargs); printf("\n");
}

#define TEST(seq, ...) test(__FILE__, __LINE__, seq, __VA_ARGS__)

int main(void)
{
	TEST("\033[m",        3, 'm', 1, 0);
	TEST("\033[0m",       4, 'm', 1, 0);
	TEST("\033[1m",       4, 'm', 1, 1);
	TEST("\033[1;2m",     6, 'm', 2, 1, 2);
	TEST("\033[12;34m",   8, 'm', 2, 12, 34);

	return 0;
}
