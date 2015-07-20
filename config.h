/* Default Ghostscript command, used to run font/resource reduction scripts */
#define GS "gs"

/* Path to psfrem executable, used in u2ps */
#define PATH "./"

/* Path to postscript library directory; psfrem and ttf2pt42 will use -I$BASE
   when running $GS above */
#define BASE "res"

/* Input will be read in block this large: */
#define CHUNKLEN 1024

/* A token is either a (utf-8) character, or a control sequence
   like \033[38;5;200m. u2ps does not attempt to maintain a proper
   state machine, and depends on the assumption that any valid token
   will not be longer than this value. */
#define MAXTOKEN 100
/* The value itself must be less than CHUNKLEN */


/* PostScript output lines will be (softly) wrapped at specified column. */
#define PSLINE 100
