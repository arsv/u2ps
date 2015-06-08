#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "config.h"
#include "warn.h"

void openout(const char* fname)
{
	int fd = open(fname, O_WRONLY | O_CREAT, 0644);

	if(fd < 0)
		die("Can't open %s: %m", fname);

	if(dup2(fd, 1) < 0)
		die("Can't dup fd: %m");
}

void openoutresuffix(const char* fname, const char* suff, const char* repl)
{
	int slen = strlen(suff);
	int flen = strlen(fname);
	int rlen = strlen(repl);
	char buf[flen + rlen + 1];

	strcpy(buf, fname);

	if(flen > slen && !strcmp(fname + flen - slen, suff))
		strcpy(buf + flen - slen, repl);
	buf[flen + rlen] = '\0';

	openout(buf);
}

int main(int argc, char** argv)
{
	int gsn = argc+5+1;
	int gsi = 0;
	char* gs[gsn];
	int i;

	gs[gsi++] = GS;
	gs[gsi++] = "-dBATCH";
	gs[gsi++] = "-sDEVICE=nullpage";
	gs[gsi++] = "-dQUIET";
	gs[gsi++] = "-dNOPAUSE";
	gs[gsi++] = "-I" BASE "/ps";

	for(i = 1; i < argc; i++)
		if(argv[i][0] != '-')
			break;
		else switch(argv[i][1]) {
			case 'I':
			case 'd':
				gs[gsi++] = argv[i];
				break;
			case '-':
				if(argv[i][2])
					die("Long options are not supported\n");
				i++;
				break;
			default:
				die("Unknown option %s", argv[i]);
		}
	if(i >= argc)
		die("Input file name required\n");
	if(i < argc - 2)
		die("Extra arguments\n");

	/* Now there's a kind of asymmetry here, ttf2pt42.ps needs ttf to be
	   a file*name*, but the output is expected to be a stream (stdout). */

	char* ttf = argv[i++];

	if(i < argc)
		openout(argv[i++]);
	else
		openoutresuffix(ttf, ".ttf", ".pfa");

	gs[gsi++] = "--";
	gs[gsi++] = BASE "/ps/t2p42.ps";
	gs[gsi++] = ttf;
	gs[gsi++] = NULL;

	execvp(*gs, gs);

	die("cannot execute %s: %m\n", *gs);
}
