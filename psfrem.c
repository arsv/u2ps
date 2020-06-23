#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "psfrem.h"
#include "config.h"
#include "warn.h"

extern char* inputname;
extern char* outputname;
extern char* statsname;
extern int keeptemp;

extern struct dynlist passopt;
extern struct dynlist excludefonts;
extern struct dynlist includefonts;

extern void handle_options(int argc, char** argv);
static void run_reduce(char* inputname, char* statsname);
extern void filter_embed(char* outputname, char* inputname, char* statsname);

static FILE* spawn_pipe(char** cmd, char* output, int* pid);

int main(int argc, char** argv)
{
	handle_options(argc, argv);

	if(statsname)
		run_reduce(inputname, statsname);

	filter_embed(outputname, inputname, statsname);

	if(statsname && !keeptemp)
		unlink(statsname);

	return 0;
}

void run_reduce(char* inputname, char* statsname)
{
	char** gs = malloc((passopt.ptr + 11)*sizeof(char*));
	char** p = gs;
	char** q = passopt.list;

	*(p++) = GS;			/* 0 */
	*(p++) = "-dBATCH";		/* 1 */
	*(p++) = "-dNOSAFER";		/* 2 */
	*(p++) = "-dNOPAUSE";		/* 3 */
	*(p++) = "-dWRITESYSTEMDICT";	/* 4 */
	*(p++) = "-dQUIET";		/* 5 */
	*(p++) = "-sDEVICE=nullpage";	/* 6 */
	if(q) while(*q) *(p++) = *(q++);/* 6 + passptr */
	*(p++) = BASE "/fstat.ps";	/* 7 + passptr */
	*(p++) = inputname;		/* 8 + passptr */
	*(p++) = BASE "/reduce.ps";	/* 9 + passptr */
	*(p++) = NULL;			/* 10 + passptr */

	int pid;
	FILE* ctrl;

	ctrl = spawn_pipe(gs, statsname, &pid);

	if((p = excludefonts.list)) while(*p)
		fprintf(ctrl, "/%s false mark-font\n", *p++);
	if((p = includefonts.list)) while(*p)
		fprintf(ctrl, "/%s true mark-font\n", *p++);
	fprintf(ctrl, "reduce-fonts\n");
	fprintf(ctrl, "reduce-rgl\n");
	fclose(ctrl);

	int status = 0;
	if(waitpid(pid, &status, 0) < 0)
		die("wait failed: %m\n");
	if(status)
		die("gs failed, aborting\n");
}

FILE* spawn_pipe(char** cmd, char* out, int* outpid)
{
	int pfd[2];
	int pid;

	if(pipe(pfd) < 0)
		die("Cannot create pipe: %m\n");

	if((pid = fork()) < 0)
		die("Cannot fork: %m\n");
	else if(pid) {
		close(pfd[0]);
		*outpid = pid;
		return fdopen(pfd[1], "w");
	} else {
		int ofd = open(out, O_WRONLY | O_CREAT | O_TRUNC, 0644);

		if(ofd < 0)
			die("Cannot create %s: %m\n", out);
		if(dup2(ofd, 1) < 0)
			die("Cannot dup to stdout: %m\n");

		close(pfd[1]);
		if(dup2(pfd[0], 0) < 0)
			die("Cannot dup to stdin: %m\n");
		execvp(*cmd, cmd);
		die("Cannot exec %s: %m\n", *cmd);
	}
}
