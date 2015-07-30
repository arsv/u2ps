#include <string.h>

#include "u2ps.h"
#include "ct.h"

/* Basic font-name setting check. Proper fontset names should
   set all relevant fonts[] entries, and it should be possible
   to override entries via keyed font specifications. */

extern void set_font(char* opt);

void reset(void) { memset(fonts, 0, nFONTS*sizeof(struct font)); };

int main(void)
{
	set_font("R:Blah");
	teststr(fonts[REGULAR].name, "Blah");
	testnull(fonts[BOLD].name);

	reset();
	set_font("FreeMono");
	teststr(fonts[REGULAR].name, "FreeMono");
	teststr(fonts[BOLD].name, "FreeMonoBold");
	teststr(fonts[ITALIC].name, "FreeMonoOblique");
	teststr(fonts[BOLDITALIC].name, "FreeMonoBoldOblique");

	set_font("I:SomeItalic");
	teststr(fonts[REGULAR].name, "FreeMono");
	teststr(fonts[BOLD].name, "FreeMonoBold");
	teststr(fonts[ITALIC].name, "SomeItalic");
	teststr(fonts[BOLDITALIC].name, "FreeMonoBoldOblique");

	return 0;
}
