#include <stdlib.h>
#include "u2ps.h"
#include "u2ps_data.h"

const struct papersize papersize[] = {
/*	  name            pw      ph     mv      mh */
	{ "a0",		2384,	3370,	100,	 0 },
	{ "a1",		1684,	2384,	 90,	 0 },
	{ "a2",		1190,	1684,	 80,	 0 },
	{ "a3",		 842,	1191,	 70,	 0 },
	{ "a4",		 595,	 842,	 55,	57 },
	{ "a5",		 420,	 595,	 50,	 0 },
	{ "a6",		 298,	 420,	 15,	 0 },
	{ "a7",		 210,	 298,	 10,	 0 },
	{ "b0",		2835,	4008,	  0,	 0 },
	{ "b1",		2004,	2835,	  0,	 0 },
	{ "b2",		1417,	2004,	  0,	 0 },
	{ "b3",		1001,	1427,	  0,	 0 },
	{ "b4",		 709,	1001,	  0,	 0 },
	{ "b5",		 499,	 709,	  0,	 0 },
	{ "b6",		 354,	 499,	  0,	 0 },
	{ "c0",		1837,	 578,	  0,	 0 },
	{ "c1",		 919,	 649,	  0,	 0 },
	{ "c2",		 459,	 323,	  0,	 0 },
	{ "letter",	 612,	 792,	  0,	 0 },
	{ "ledger",	 792,	1224,	  0,	 0 },
	{ "executive",	 522,	 756,	  0,	 0 },
	{ NULL }
};

const struct fontaspect fontaspects[] = {
	{ "DejaVuSansMono",	602 },
	{ "MesloLGM",		602 },
	{ "EnvyCodeR",		537 },
	{ "Iosevka",		500 },
	{ "UnifontMedium",	500 },
	{ "SawarabiGothic",	500 },
	{ NULL,			600 }
};

const char fontkeys[nFONTS+1] = {
	[REGULAR] = 'R',
	[BOLD] = 'B',
	[ITALIC] = 'I',
	[BOLDITALIC] = 'O',
	[CJK] = 'C',
	[THAI] = 'T',
	[nFONTS] = '\0'
};

/* The code below does static initialization of a const char* f[][].
   See comments in u2ps_data.i on why this is needed. */

#define defont(name, ...) \
	static const char* name[] = { __VA_ARGS__, NULL };
#include "u2ps_data.i"
#undef defont

const struct fontvariant fontvariants[] = {
#define defont(name, ...) \
	{ #name, name },
#include "u2ps_data.i"
	{ NULL }
};

/* At present the assumption is that a single font is used for CJK. */

const struct fontrange fontranges[] = {
	{ 0x0000, 0x0370, REGULAR },	/* fast-skip common Latin */
	{ 0x1E00, 0x1EF9, REGULAR },
	{ 0x0E00, 0x0E5B, THAI },
	{ 0x1100, 0x11FF, CJK },	/* Hangul */
	{ 0x2E80, 0x3400, CJK },	/* aux CJK glyphs */
	{ 0x3400, 0x4DFF, CJK },	/* rare ideographs */
	{ 0x4E00, 0x9FFF, CJK },	/* common ideographs */
	{ 0xF900, 0xFAFF, CJK },	/* duplicates, unifiable, corporate characters */
	{ 0xAC00, 0xD7A3, CJK },	/* Hangul */
	{ 0x0000, 0x0000, 0 }
};
