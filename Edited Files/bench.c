#include "bench.h"

#define DATAS ("testdb")

struct data
{
    long int count;
    int r;
};

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}


// pername ta orismata 
void * passWriteParams(void * arg)
{
	struct data *d = (struct data *) arg;
	_write_test(d->count, d->r);
	return 0;
}

void * passReadParams(void * arg)
{
	struct data *d = (struct data *) arg;
	_read_test(d->count, d->r);
	return 0;
}

int main(int argc,char** argv)
{
	long int count;

	// Initializing the counters
	totalFound = 0;
	totalReadTime = 0;
	totalWriteTime = 0;
	readCount = 0;
	writeCount = 0;

	// Initializing the mutex'es
	pthread_mutex_init(&read_lock, NULL);
	pthread_mutex_init(&write_lock, NULL);

	srand(time(NULL));
	if (argc < 3) {
		fprintf(stderr,"Usage: db-bench <write | read | read-write> <count> --optionals: <threadCount> <readWritePercentages(only for read-write)> <random>\n");
		exit(1);
	}
	
	if (strcmp(argv[1], "write") == 0) {
		int r = 0;
		int threadCount = 1;
		struct data dataWrite;

		count = atoi(argv[2]);

		_print_header(count);
		_print_environment();

		if (argc >= 4)
			threadCount = atoi(argv[3]);
		// Initialize thread array
		pthread_t threads[threadCount];
		
		if (argc == 5)
			r = 1;

		dataWrite.count = count / threadCount;
		dataWrite.r = r;

		db = db_open(DATAS);

		int i;
		for (i = 0; i < threadCount; i++)
			pthread_create(&threads[i], NULL, passWriteParams, (void * ) &dataWrite);
		for (i = 0; i < threadCount; i++)
			pthread_join(threads[i], NULL);

		db_close(db);

		printf(LINE);
		printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
			,writeCount
			,(double)(totalWriteTime / writeCount / threadCount) 
			,(double)(writeCount / totalWriteTime * threadCount)
			,totalWriteTime / threadCount);	
	} 
	else if (strcmp(argv[1], "read") == 0) {
		int r = 0;
		int threadCount = 1;
		struct data dataRead;

		count = atoi(argv[2]);

		_print_header(count);
		_print_environment();

		if (argc >= 4)
			threadCount = atoi(argv[3]);
		// Initialize thread array
		pthread_t threads[threadCount];

		if (argc == 5)
			r = 1;
		
		dataRead.count = count / threadCount;
		dataRead.r = r;

		db = db_open(DATAS);

		int i;
		for (i = 0; i < threadCount; i++)
			pthread_create(&threads[i], NULL, passReadParams, (void * ) &dataRead);
		for (i = 0; i < threadCount; i++)
			pthread_join(threads[i], NULL);

		db_close(db);

		printf(LINE);
		printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
			readCount, totalFound,
			(double)(totalReadTime /  readCount / threadCount),
			(double)( readCount / totalReadTime * threadCount),
			totalReadTime / threadCount);
	}
	else if (strcmp(argv[1], "readwrite") == 0) {
		int r = 0;
		// default threads = 2, one for each method
		long int totalThreads = 2;
		// To calculate the threads for each method
		long int readThreads;
		long int writeThreads;
		// To calculate the count for each method
		long int localWriteCount;
		long int localReadCount;

		struct data dataRead;
		struct data dataWrite;

		count = atoi(argv[2]);

		_print_header(count);
		_print_environment();

		if (argc >= 4)
			totalThreads = (long int) atoi(argv[3]);

		//default values 50/50 percentage read-write
		localReadCount = count / 2;
		localWriteCount = count - localReadCount;
		// default threads 50/50 as well
		readThreads = totalThreads / 2;
		writeThreads = totalThreads - readThreads;

		//Check to see if user has input % to divide the count & threads, ex: 10/90
		//cannot be single digit on the first, if threads < 100
		if (argc >= 5)
		{		
			int num;
			char str[2];
			strncpy(str, argv[4], 2);
			if (str[2] == '/')
			{
				strncpy(str, argv[4], 1);
				num = atoi(str);
			} 
			else
				num = atoi(str);
			
			// divide count accordingly
			localReadCount = num * count / 100;
			localWriteCount = count - localReadCount;

			// divide threads accordingly
			readThreads = num * totalThreads / 100;
			writeThreads = totalThreads - readThreads;
		}
		// Initialize thread arrays
		pthread_t read_threads[writeThreads], write_threads[readThreads];

		if (argc == 6)
			r = 1;

		db = db_open(DATAS);
		// For the read method
		dataRead.count = localReadCount /  readThreads;
		dataRead.r = r;
		// For the write method
		dataWrite.count = localWriteCount /  writeThreads;
		dataWrite.r = r;

		int i;
		for (i = 0; i < writeThreads; i++)
			pthread_create(&write_threads[i], NULL, passWriteParams, (void * ) &dataWrite);
		for (i = 0; i < readThreads; i++)
			pthread_create(&read_threads[i], NULL, passReadParams, (void * ) &dataRead);

		for (i = 0; i < writeThreads; i++)
			pthread_join(write_threads[i], NULL);
		for (i = 0; i < readThreads; i++)
			pthread_join(read_threads[i], NULL);
		
		db_close(db);

		// Read print
		printf(LINE);
		printf("|Random-Read	(done:%ld, found:%d): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
			readCount, totalFound,
			(double)(totalReadTime /  readCount / readThreads),
			(double)( readCount / totalReadTime * readThreads),
			totalReadTime / readThreads);

		// Write print
		printf(LINE);
		printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
			,writeCount
			,(double)(totalWriteTime / writeCount / writeThreads) 
			,(double)(writeCount / totalWriteTime * writeThreads)
			,totalWriteTime / writeThreads);
	} else {
		fprintf(stderr,"Usage: db-bench <write | read> <count> <random>\n");
		exit(1);
	}

	return 1;
}
