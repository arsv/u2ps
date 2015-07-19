#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "config.h"
#include "resuffix.h"
#include "warn.h"
#include "u2ps.h"

/* There's a bunch of globals defined and set in u2ps_opts.c
   via handle_args call below. Those are not important here;
   the code in this file assumes they have been set to some
   sane values.

   What really happens here is piping input to output, with
   some filtering in-between, and possibly psfrem call at the end. */

FILE* input;
FILE* output;

/* The input stream is processed in tokens, not bytes, with each token
   being either a control character, a (printable) unicode character,
   or a full escape sequence. The stream is read in chunks however, with
   no guarantees that each chunk will be aligned at (variable) token
   boundary.

   The code below assumes no token can be longer than MAXTOKEN bytes,
   and uses that to keep the tokenizer simple and stateless.
   Instead of letting the tokenizer scan the full chunklen, it is told
   there are only (chunklen - MAXTOKEN) bytes there, and allowed
   to overshot the boundary slightly. MAXTOKEN thus acts as a sort
   of low-water-mark, prompting read_chunk to shift the unprocessed
   bytes to the start and read in some more.

   The real hard boundary for tokenizer to stop at is '\0'.

   This structure follows perl code, which naturally used regexp as
   a tokenizer. It might have been better to use a state machine to
   handle the stream bytewise, decoding utf and csi-commands at the
   same time. */

static int chunklen = 0;
static int chunkptr = 0;
static char chunk[CHUNKLEN];

static void open_input_output(void);
static void close_input_output(void);
extern void new_file();
extern void end_file();
static int read_chunk(char* chunk, int size, int ptr, int len);
static int run_psfrem(void);

int main(int argc, char** argv)
{
	handle_args(argc, argv);
	open_input_output();

	new_file();
	while(1) {
		if((chunklen = read_chunk(chunk, CHUNKLEN, chunkptr, chunklen)) > 0)
			chunkptr = 0;
		else
			break;
		if(chunklen <= MAXTOKEN)
			break;
		if((chunkptr = print_chunk(chunk, chunklen - MAXTOKEN)) <= 0)
			break;
	} if(chunkptr < chunklen) {
		print_chunk(chunk, chunklen);
	}
	end_file();

	if(runopts.skipfrem)
		return 0;

	close_input_output();
	return run_psfrem();
}

int read_chunk(char* chunk, int size, int ptr, int len)
{
	int save = len - ptr;

	if(save > 0)
		memmove(chunk, chunk + ptr, save);
	else
		save = 0;

	int read = fread(chunk + save, 1, size - save - 1, input);

	len = save + (read >= 0 ? read : 0);
	chunk[len] = '\0';

	return len;
}

/* u2ps itself is a simple filter and can do stdin-to-stdout
   without much effort, but psfrem does two-pass processing,
   and needs a real file to work on as well as a temporary file
   to store glyph stats.

   Without psfrem, we write directly to the output.

   When psfrem is expected, tmpoutname is created, u2ps writes
   there, closes it and runs psfrem tmpoutname outputname.

   Temp file name is based on outputname whenever possible,
   and /tmp fallback is only used when there is no outputname.
   It might have been better to try /tmp unless told otherwise,
   but on the other hand, it might have not, and having all the
   temp files together makes debugging easier. */

void open_input_output(void)
{
	if(!inputname)
		input = stdin;
	else if(!(input = fopen(inputname, "r")))
		die("Cannot open %s: %m\n", inputname);

	if(inputname && !outputname && !runopts.stdout)
		outputname = resuffix(inputname, ".txt", ".ps");

	if(runopts.skipfrem) {
		if(!output)
			output = stdout;
		else if(!(output = fopen(outputname, "w")))
			die("Cannot open %s: %m\n", outputname);
	} else {
		if(!outputname) {
			if(!tmpoutname)
				tmpoutname = preptemplate("u2ps.XXXXXXXX.ps");
			if(!(output = fmkstemps(tmpoutname, 3)))
				die("Cannot create temporary file: %m\n");
		} else {
			tmpoutname = resuffix(outputname, ".ps", ".tps");
			if(!(output = fopen(tmpoutname, "w")))
				die("Cannot create %s: %m\n", tmpoutname);
		}
	}
}

/* Got to flush buffers before letting psfrem work on the files. */

void close_input_output(void)
{
	fclose(input);
	fclose(output);
}

/* With psfrem there is always tmpoutname which should be removed
   afterwards, so it's fork-wait and then unlink.

   In case of errors, we leave all temp files as is. */

int spawn(char *const argv[])
{
	pid_t pid;
	int status;

	pid = fork();
	if(pid < 0)
		die("Cannot fork: %m\n");
	if(!pid) {
		execvp(*argv, argv);
		die("Can't execute %s: %m\n", *argv);
	} else {
		if(waitpid(pid, &status, 0) < 0)
			die("wait failed: %m\n");
		return status;
	}
}

int run_psfrem(void)
{
	char** psfrem = malloc((passnum + 10)*sizeof(char*));
	char** p = psfrem;
	char** q = passopts;

	if(!psfrem) die("malloc: %m\n");

	*p++ = PATH "/psfrem";
	if(!runopts.skipfonts)
		*p++ = "-r";
	if(runopts.embedstdfonts)
		*p++ = "-A";
	if(runopts.keeptemp)
		*p++ = "-k";
	*p++ = "-I" BASE;
	if(q) while(*q) *p++ = *q++;

	if(outputname) {
		*p++ = "--";
		*p++ = tmpoutname;
		*p++ = outputname;
	} else {
		*p++ = "--";
		*p++ = tmpoutname;
		/* psfrem dumps output to stdout */
	}

	*p++ = NULL;

	spawn(psfrem);

	if(!runopts.keeptemp)
		unlink(tmpoutname);

	return 0;
}
