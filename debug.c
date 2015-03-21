#include "debug.h"

#include <stdlib.h>
#include <stdio.h>
#ifdef __MINGW32__
#else
#include <execinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#endif

void segv_handler(int sig)
{
#ifdef __MINGW32__
    //TODO: http://stackoverflow.com/a/5699483
#else
	void *stackptrs[10];
	size_t num = backtrace(stackptrs, 10);
	FILE *btfile = fopen("backtraces.log", "a");
	fprintf(btfile, "\n");
	backtrace_symbols_fd(stackptrs, num, fileno(btfile));
	fclose(btfile);
	printf("==== CRASH (signal %d) ====\n", sig);
	backtrace_symbols_fd(stackptrs, num, STDERR_FILENO);
	exit(1);
#endif
}


void debug_install_handler()
{
#ifdef __MINGW32__
#else
	signal(SIGSEGV, segv_handler);
#endif
}
