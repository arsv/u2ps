#include <stdio.h>

extern int deutf(char* s, int* codepoint);

void test(char* file, int line, char* s, int ebt, int ecp)
{
	int rcp = -1;
	int rbt = deutf(s, &rcp);

	if(rbt != ebt)
		printf("%s:%i: FAIL taken %i != %i expected\n", file, line, rbt, ebt);
	else if(ecp == -1)
		printf("%s:%i: OK taken %i invalid\n", file, line, -rbt);
	else if(rcp != ecp)
		printf("%s:%i: FAIL codepoint 0x%X != 0x%X expected\n", file, line, rcp, ecp);
	else
		printf("%s:%i: OK taken %i codepoint 0x%X\n", file, line, rbt, rcp);
}

#define TEST(a, b, c) test(__FILE__, __LINE__, a, b, c)

int main(void)
{
	TEST("",	1, 0x00000000);
	TEST("abc",	1, 0x00000061);
	TEST("б",	2, 0x00000431);
	TEST("￥",	3, 0x0000FFE5);

	/* ￥ sequence is EF BF A5.
	   For invalid input, only the stuff that could have made a valid
	   sequnce should be consumed. */
	TEST("\xFF",		-1, -1);
	TEST("\xEF\xFF",	-1, -1);
	TEST("\xEF\xBF\xFE",	-2, -1);

	return 0;
}
