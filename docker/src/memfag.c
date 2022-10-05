#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

static void print_usage();
static void run();
static void alloc_mem(size_t size);
static void free_mem();
static void touch_mem(void * buf, size_t size);
static void print_ts();
static void print_rusage();

static size_t start, limit;
static unsigned int hold;

static void * mem_buf = NULL;

int
main(int argc, char* argv[])
{
	int start_alloc, limit_alloc, hold_for;

	if (argc != 4) {
		print_usage();
		return 0;
	}

	start_alloc = strtol(argv[1], NULL, 10);
	limit_alloc = strtol(argv[2], NULL, 10);
	hold_for = strtol(argv[3], NULL, 10);

	// Check for sanity:
	if (start_alloc < 1) {
		fprintf(stderr, "Invalid value of <start>: %d\n", start_alloc);
		return 1;
	}
	if (limit_alloc < 1 || limit_alloc <= start_alloc) {
		fprintf(stderr, "Invalid value of <limit>: %d\n", limit_alloc);
		return 1;
	}
	if (hold_for < 1) {
		fprintf(stderr, "Invalid value of <hold_for>: %d\n", hold_for);
		return 1;
	}

	// Turn MBs into Bytes:
	start = 1024L * 1024 * start_alloc;
	limit = 1024L * 1024 * limit_alloc;

	hold = hold_for;

	printf("start, B    : %ld\n", start);
	printf("limit, B    : %ld\n", limit);
	printf("hold for, s : %d\n", hold);

	for (;;) {
		run();
	}

	return 0;
}

static void
print_usage()
{
	fprintf(stderr, "Usage:\n  This program expects 3 arguments:\n"
"    <start> -- allocate <start> MB of RAM on start, double allocation\n"
"               every <hold_for> seconds;\n"
"    <limit> -- reset to <start> when memory allocation is over <limit> MB;\n"
"    <hold_for> -- how long to keep allocated memory, seconds.\n");
}


static void
run()
{
	time_t hold_till = time(NULL) + hold;

	for (size_t curr_alloc = start; curr_alloc < limit; curr_alloc *= 2) {

		alloc_mem(curr_alloc);
		while (time(NULL) < hold_till) {
			touch_mem(mem_buf, curr_alloc);
			sleep(1);
		}
		print_rusage();
		free_mem();

		hold_till += hold;
	}
}

static void
print_ts()
{
	time_t t = time(NULL);
	char t_str[32];
	ctime_r(&t, t_str);
	for (int i = 0; i < 32; ++i) {
		if (t_str[i] == '\n') {
			t_str[i] = '\0';
		}
	}
	printf("[%s]", t_str);
}

static void
alloc_mem(size_t size)
{
	//fprintf(stderr, "alloc_mem(%ld)\n", size);
	mem_buf = malloc(size);
	if (mem_buf == NULL) {
		fprintf(stderr, "malloc(%ld) failed\n", size);
		exit(1);
	}

	print_ts();
	printf(" %ld bytes of memory was allocated\n", size);
}

static void
free_mem()
{
	//fprintf(stderr, "free_mem()\n");
	free(mem_buf);
	mem_buf = NULL;
}

static void
touch_mem(void * buf, size_t size)
{
	char * base = (char *)buf;

	//fprintf(stderr, "touch_mem(%p, %ld)\n", buf, size);
	for (size_t idx = 0; idx < size; idx += 4096) {
		++base[idx];
	}
}

static void
print_rusage()
{
	struct rusage usage;

	int rs = getrusage(RUSAGE_SELF, &usage);
	if (rs < 0) {
		fprintf(stderr, "getrusage() failed\n");
		return;
	}

	print_ts();
	// man 2 getrusage
	printf(" Mem resource usage:");
	printf(" maxrss(%ld)", usage.ru_maxrss);
	printf(" ixrss(%ld)", usage.ru_ixrss);
	printf(" idrss(%ld)", usage.ru_idrss);
	printf(" isrss(%ld)", usage.ru_isrss);
	printf("\n");
}
