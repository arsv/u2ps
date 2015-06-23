#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"
#include "warn.h"
#include "u2ps.h"

FILE* input;
FILE* output;

static int chunklen = 0;
static int chunkptr = 0;
static char chunk[CHUNKLEN];

static void open_input_output(void);
static void close_input_output(void);
extern void new_file();
extern void end_file();
static int read_chunk(char* chunk, int size, int ptr, int len);
static int exec_psfrem(void);

int main(int argc, char** argv)
{
	handle_args(argc, argv);
	open_input_output();

	new_file();
	while(1) {
		if((chunklen = read_chunk(chunk, CHUNKLEN, chunkptr, chunklen)) <= 0)
			break;
		if(chunklen < MAXTOKEN)
			break;
		if((chunkptr = print_chunk(chunk, 0, chunklen - MAXTOKEN)) <= 0)
			break;
	} if(chunkptr < chunklen)
		print_chunk(chunk, chunkptr, chunklen);
	end_file();

	if(runopts.skipfrem)
		return 0;

	close_input_output();
	return exec_psfrem();
}

char* preptemplate(const char* template)
{
	char* dir = getenv("TMPDIR");
	if(!dir) dir = "/tmp";

	int templen = strlen(template);
	int dirlen = strlen(dir);
	char* out = malloc(templen + dirlen + 2);

	if(!out) die("Cannot allocate memory: %m\n");

	strncpy(out, dir, dirlen);
	out[dirlen] = '/';
	strncpy(out + dirlen + 1, template, templen);
	out[dirlen + templen + 1] = '\0';

	return out;
}

FILE* fmkstemps(char* template, int suffixlen)
{
	int fd;

	if((fd = mkstemps(template, suffixlen)) < 0)
		return NULL;

	return fdopen(fd, "w");
}

char* resuffix(const char* name, const char* oldsuff, const char* newsuff)
{
	int namelen = strlen(name);
	int oldsufflen = strlen(oldsuff);
	int newsufflen = strlen(newsuff);
	char* newname = malloc(namelen + newsufflen + 2);

	strcpy(newname, name);
	if(namelen > oldsufflen && !strcmp(newname + namelen - oldsufflen, oldsuff))
		strcpy(newname + namelen - oldsufflen, newsuff);
	else
		strcpy(newname + namelen, newsuff);

	return newname;
}

void open_input_output(void)
{
	if(!inputname)
		input = stdin;
	else if(!(input = fopen(inputname, "r")))
		die("Cannot open %s: %m\n", inputname);

	if(inputname && !outputname && !runopts.stdout)
		outputname = resuffix(inputname, ".txt", ".ps");

	if(outputname) {
		if(!(output = fopen(outputname, "w")))
			die("Cannot open %s: %m\n", outputname);
	} else if(runopts.skipfrem) {
		output = stdout;
	} else {
		if(!tmpoutname)
			tmpoutname = preptemplate("u2ps.XXXXXXXX.ps");
		if(!(output = fmkstemps(tmpoutname, 3)))
			die("Cannot create temporary file: %m\n");
	}
}

void close_input_output(void)
{
	fclose(input);
	fclose(output);
}

int read_chunk(char* chunk, int size, int ptr, int len)
{
	int save = len - ptr;

	if(save > 0)
		memcpy(chunk, chunk + ptr, save);
	else
		save = 0;

	int read = fread(chunk + save, 1, size - save - 1, input);

	len = save + (read >= 0 ? read : 0);
	chunk[len] = '\0';

	return len;
}

int exec_psfrem(void)
{
	char** psfrem = malloc((passnum + 10)*sizeof(char*));
	char** p = psfrem;
	char** q = passopts;

	if(!psfrem) die("malloc: %m\n");

	*p++ = PATH "/psfrem.px";
	if(!runopts.skipfonts)
		*p++ = "-r";
	if(runopts.embedstdfonts)
		*p++ = "-A";
	if(runopts.skipunlink)
		*p++ = "-k";
	*p++ = "-I" BASE;
	if(q) while(*q) *p++ = *q++;

	if(outputname) {
		*p++ = "--";
		*p++ = outputname;
	} else {
		*p++ = "-i";
		*p++ = "--";
		*p++ = tmpoutname;
	}

	*p++ = NULL;

	execvp(*psfrem, psfrem);

	die("Can't execute %s: %m\n", *psfrem);

	return -1;
}
