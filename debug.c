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
#else
	void *stackptrs[10];
	size_t num = backtrace(stackptrs, 10);
	int btfile = open("backtrace.log", O_WRONLY | O_APPEND | O_CREAT, 0666);
	write(btfile, "\n", 1);
	backtrace_symbols_fd(stackptrs, num, btfile);
	close(btfile);
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
