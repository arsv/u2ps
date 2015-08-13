#include <string.h>

#include "u2ps.h"
#include "ct.h"

/* Font scaling check.
   set_font_aspects should be able to find aspect values,
   and set_font_sizes is expected to bring all fonts to the same width. */

extern void set_font(char* opt);
extern void set_font_aspects(void);
extern void set_font_sizes(void);

void reset(void) { memset(fonts, 0, nFONTS*sizeof(struct font)); };

int main(void)
{
	fontsize = 1000; /* 10pt */
	fontaspect = 0;	 /* not set */

	/* Default case */
	set_font("FreeMono");
	fontaspect = 0;
	set_font_aspects();
	testeqi(fonts[REGULAR].aspect, 600);
	testeqi(fonts[REGULAR].xscale, 0);
	testeqi(fonts[BOLD].aspect, 600);
	testeqi(fonts[BOLD].xscale, 0);
	/* unset fontaspect should default to REGULAR's */
	testeqi(fontaspect, fonts[REGULAR].aspect);

	/* with no special requests, the fonts should retain their
	   natural aspect, which means xscale 1000mil = 1.0 */
	set_font_sizes();
	testeqi(fonts[REGULAR].aspect, 600);
	testeqi(fonts[REGULAR].xscale, 1000);
	testeqi(fonts[BOLD].aspect, 600);
	testeqi(fonts[BOLD].xscale, 1000);
	testeqi(fontaspect, fonts[REGULAR].aspect);

	/* now let's request 0.3 font; this should result in 500mil
	   xscale, shrinking the font down from its natual 0.6 aspect */
	reset();
	fontaspect = 300;
	set_font("FreeMono");
	set_font_aspects();
	set_font_sizes();
	testeqi(fonts[REGULAR].xscale, 500);
	testeqi(fonts[BOLD].xscale, 500);

	/* in case one of the fonts is of different size, it must be scaled */
	reset();
	fontaspect = 0;
	set_font("FreeMono");
	set_font_aspects();
	fonts[BOLD].aspect = 300;
	set_font_sizes();
	testeqi(fonts[REGULAR].xscale, 1000);
	testeqi(fonts[BOLD].xscale, 2000);

	return 0;
}
