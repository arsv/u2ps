#include "u2ps.h"
#include "warn.h"

/* CSI mean Control Sequence Introducer, "\033[".
   See console_codes(4) for some background.

   The only kind of control sequences u2ps supports at present
   is CSI m which sets text attributes.

   The code is a bit hairy, it does a lot to avoid unnecessary
   PS commands in the output and it has to track the state across
   page and line boundaries which is pointless from terminal
   standpoint but necessary within PS. */

extern struct genopts genopts;

/* attr.flags */
#define BF (1<<0)	/* boldface */
#define HB (1<<1)	/* halfbright */
#define IT (1<<2)	/* italic */
#define UL (1<<3)	/* underlined */
#define BL (1<<4)	/* blinking */
#define RB (1<<5)	/* rapidly blinking */
#define RV (1<<6)	/* reversed */
#define IV (1<<7)	/* invisible */
#define SL (1<<8)	/* striked-out */

/* Positive color values are in fact colors
   (in a weird indexed palette) but there are also ps
   commands which go in place of color-setting, these
   are negative. */

/* attr.[fb]g */
#define UNDEFINED	-1
#define NOTSET		-2
/* attr.e[fb]g */
#define INVISIBLE	-3
#define HALFBRIGHT	-4
#define REVERSED	-5

static struct attr {
	int flags;
	int fg, efg;	/* foreground, effective foreground */
	int bg, ebg;	/* background, effective background */
	char* efs;	/* effective font style */
} curr, next;

extern void pscmd(const char* cmd, ...);

static void ansi_set(int key);
static void ansi_clr(int key);
static void ansi_fg(int color);
static void ansi_bg(int color);
static void ansi_reset(void);

static void put_ansi_diff(void);

void new_page_attr(void)
{
	next = curr;
	curr.flags &= ~(UL | SL);
	curr.fg = UNDEFINED;
	curr.bg = UNDEFINED;

	put_ansi_diff();
};

void new_line_attr(void)
{
	if(curr.flags & UL)
		pscmd("ul");
	if(curr.flags & SL)
		pscmd("sl");
}

void end_line_attr(void)
{
	if(curr.flags & UL)
		pscmd("ue");
	if(curr.flags & SL)
		pscmd("se");
}

void handle_csi(int cmd, int argn, int* args)
{
	int a;
	int i, j;

	if(cmd != 'm')
		return;

	for(i = 0; i < argn; i++) {
		switch(a = args[i]) {
			case 0: ansi_reset(); break;
			case   1 ...   9: ansi_set(a - 1); break;
			case  21 ...  29: ansi_clr(a - 21); break;
			case  30 ...  37: ansi_fg(a - 30); break;
			case  39: ansi_fg(NOTSET); break;
			case  40 ...  47: ansi_bg(a - 40); break;
			case  49: ansi_bg(NOTSET); break;
			case  90 ...  99: ansi_fg(a - 90 + 8); break;
			case 100 ... 109: ansi_bg(a - 100 + 8); break;
			/* 88-color sequences: ESC[38;5;Cm */
			case 48:
			case 38: if((j = i+2) >= argn || args[i+1] != 5)
					 break;
				 (a == 38 ? ansi_fg : ansi_bg)(args[j]);
				 i = j;
		}
	}

	put_ansi_diff();
}

void ansi_reset(void)
{
	next.flags = 0;
	next.fg = NOTSET;
	next.bg = NOTSET;
}

static void ansi_set(int key)
{
	next.flags |= (1<<key);
}

static void ansi_clr(int key)
{
	next.flags &= ~(1<<key);
}

static void ansi_fg(int color)
{
	next.fg = color;
}

static void ansi_bg(int color)
{
	next.bg = color;
}

static void ansi_set_next_evalues(void)
{
	if(next.fg >= 0) {
		next.efg = next.fg;
		if(next.efg <= 7 && (next.flags & BF))
			next.efg += 8;
	} else if(next.flags & IV)
		next.efg = INVISIBLE;
	else if(next.flags & HB)
		next.efg = HALFBRIGHT;
	else if(next.flags & RV)
		next.efg = REVERSED;
	else
		next.efg = NOTSET;

	if(next.bg >= 0)
		next.ebg = next.bg;
	else if(next.flags & RV)
		next.ebg = REVERSED;
	else
		next.ebg = NOTSET;

	int bold = (next.flags & BF) | (next.flags & BL) | (next.flags & RB);
	int italic = (next.flags & IT);
	if(bold && italic)
		next.efs = "fO";
	else if(bold)
		next.efs = "fB";
	else if(italic)
		next.efs = "fI";
	else
		next.efs = "fR";
}

static void put_ansi_diff(void)
{
	ansi_set_next_evalues();

	if(curr.efs != next.efs)
		pscmd("%s", next.efs);

	if(curr.efg != next.efg) {
		if(next.efg >= 0)
			pscmd("%i fg", next.efg);
		else switch(next.efg) {
			case NOTSET:	pscmd("nf"); break;
			case INVISIBLE: pscmd("vf"); break;
			case REVERSED:	pscmd("rf"); break;
			case HALFBRIGHT:pscmd("hf"); break;
		}
	}

	if(curr.ebg != next.ebg) {
		if(next.ebg >= 0)
			pscmd("%i bg", next.ebg);
		else switch(next.ebg) {
			case NOTSET:	pscmd("ng"); break;
			case INVISIBLE: pscmd("vg"); break;
			case REVERSED:	pscmd("rg"); break;
		}
	}

	int q;
	if((curr.flags & UL) != (q = next.flags & UL))
		pscmd(q ? "ul" : "ue");
	if((curr.flags & SL) != (q = next.flags & SL))
		pscmd(q ? "sl" : "se");
	
	curr = next;
}
