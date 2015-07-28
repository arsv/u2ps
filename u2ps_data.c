#include <stdlib.h>
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
	{ "FreeMono",	600 },
	{ NULL }
};

const struct fontvariant fontvariants[] = {
/*	     regular             bold             italic             bold italic */
	{ "FreeMono", {	"FreeMonoBold", "FreeMonoOblique", "FreeMonoBoldOblique" } },
	{ NULL }
};
