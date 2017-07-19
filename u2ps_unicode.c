#include "u2ps.h"

static struct utfmask {
	unsigned char mask;
	unsigned char test;
	unsigned char take;
} utfmask[] = {
	{ 0x80 /* 1000 0000 */, 0x00 /* 0000 0000 */, 0x7F /* 0111 1111 */ },
	{ 0xC0 /* 1100 0000 */, 0x80 /* 1000 0000 */, 0x3F /* 0011 1111 */ },
	{ 0xE0 /* 1110 0000 */, 0xC0 /* 1100 0000 */, 0x1F /* 0001 1111 */ },
	{ 0xF0 /* 1111 0000 */, 0xE0 /* 1110 0000 */, 0x0F /* 0000 1111 */ },
	{ 0xF8 /* 1111 1000 */, 0xF0 /* 1111 0000 */, 0x07 /* 0000 0111 */ },
	{ 0xFC /* 1111 1100 */, 0xF8 /* 1111 1000 */, 0x03 /* 0000 0011 */ },
	{ 0xFE /* 1111 1110 */, 0xFC /* 1111 1100 */, 0x01 /* 0000 0001 */ },
	{ 0x00, 0x00, 0x00 }
};

/* Entry [1] from the above array */
#define CONT_MASK 0xC0
#define CONT_TEST 0x80
#define CONT_TAKE 0x3F

#define valid(bytestaken)    (bytestaken)
#define invalid(bytestaken) -(bytestaken)

#define endof(array) (array + sizeof(array)/sizeof(*array))

int deutf(unsigned char* s, int* codepoint)
{
	struct utfmask* p;
	int seq;
	int cnt;

	for(p = utfmask; p < endof(utfmask); p++)
		if((p->mask & *s) == p->test)
			break;
	switch(seq = (p - utfmask)) {
		default:
			return invalid(1);
		case 0:
			*codepoint = *s;
			return valid(1);
		case 2 ... 6:
			*codepoint = *s++ & p->take;
	} for(cnt = 1; *s && cnt < seq; cnt++) {
		if((*s & CONT_MASK) != CONT_TEST)
			break;
		*codepoint = (*codepoint << 6) | (*s++ & CONT_TAKE);
	};

	return (cnt < seq) ? invalid(cnt) : valid(cnt);
}

/* Assumed glyph width in monospace terminal font. That's 1 for all regular
   characters, 2 for double-width characters and 0 for combining characters
   and control code.

   The return of this function should match usubstitute from the ps code.
   Even better, it should match the actual font used, but that kind of data
   is not available within u2ps.

   Either way, a mistake here will only affect line wrapping, causing early
   or late wraps, and that's it. */

int uniwidth(int codepoint)
{
	switch(codepoint) {
		// ASCII stuff and generic Latin
		case 0x0000 ... 0x001F: return 0;
		case 0x0020 ... 0x02FF: return 1;
		// generic combining stuff
		case 0x0300 ... 0x036F:
		case 0x20D0 ... 0x20EF: return 0;
		// Greek and Coptic
		case 0x0374 ... 0x03F3: return 1;
		// Cyrillic
		case 0x0400 ... 0x0482: return 1;
		case 0x0483 ... 0x0489:	return 0;  // cyrillic combining marks
		case 0x048C ... 0x04F9: return 1;
		// Armenian
		case 0x0500 ... 0x058A: return 1;
		// Hebrew
		case 0x0591 ... 0x05BD: return 0;
		case 0x05BE: return 1;
		case 0x05BF: return 0;
		case 0x05C0: return 1;
		case 0x05C1 ... 0x05C4: return 0;
		case 0x05D0 ... 0x05F4: return 1;
		// Georgian
		case 0x10A0 ... 0x10FB: return 1;
		// Hangul double-width
		case 0x1100 ... 0x115F: return 2;
		// Hangul combining
		case 0x1160 ... 0x11F9: return 0;
		// Combining arrowheads
		case 0x1DFE:
		case 0x1DFF: return 0;
		// CJK double-width characters
		case 0x2E80 ... 0x3098: return 2;
		case 0x309D ... 0x4DB5:
		case 0x4E00 ... 0x9FC3:
		case 0xA000 ... 0xA4C6: return 2;
		// katakana-hiragana combining voice marks
		case 0x3099:
		case 0x309A: return 0;
		// combining ligatures
		case 0xFE20:
		case 0xFE21:
		case 0xFE22:
		case 0xFE23: return 0;
		// BOM? or just invalid
		case 0xFFFF:
		case 0xFFFE: return 0;
		// Thai combining characters
		case 0x0E31:
		case 0x0E34:
		case 0x0E35:
		case 0x0E36:
		case 0x0E37:
		case 0x0E38:
		case 0x0E39:
		case 0x0E3A:
		case 0x0E47:
		case 0x0E48:
		case 0x0E49:
		case 0x0E4A:
		case 0x0E4B:
		case 0x0E4C:
		case 0x0E4D:
		case 0x0E4E: return 0;
		// variation selectors (?)
		case 0xFE00 ... 0xFE0F: return 0;
		// fullwidth currency
		case 0xFFE0 ... 0xFFE6: return 2;

		case 0x1D300 ... 0x1D371: return 2;
		case 0x1F100 ... 0x1F1FF: return 2;
		case 0x1F030 ... 0x1F061: return 2;
		case 0xE0000 ... 0xE01FF: return 2;

		// Everything else is single-width
		default: return 1;
	}
}
