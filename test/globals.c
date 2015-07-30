#include <stdio.h>
#include "u2ps.h"

FILE* output;
char* inputname;

struct genopts genopts;
struct pagelayout pagelayout;
struct headings headings;

int fontsize;
int fontaspect;
int auxsize;
const char* auxfont;
struct font fonts[nFONTS];
