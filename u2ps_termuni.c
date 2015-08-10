#include "u2ps.h"
#include "u2ps_term.h"

const struct fontrange fontranges[] = {
	{ 0x0000, 0x0370, REGULAR },	/* fast-skip common Latin */
	{ 0x1E00, 0x1EF9, REGULAR },
	{ 0x0000, 0x0000, 0 }
};
