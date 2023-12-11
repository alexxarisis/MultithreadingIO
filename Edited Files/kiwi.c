#include "bench.h"
#include "../engine/variant.h"

void _write_test(long int count, int r)
{
	int i;
	long long start,end;
	Variant sk, sv;

	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);
	start = get_ustime_sec();
	for (i = 0; i < count; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	end = get_ustime_sec();

	pthread_mutex_lock(&write_lock);
	totalWriteTime += end - start;
	writeCount += count;
	pthread_mutex_unlock(&write_lock);	
}

void _read_test(long int count, int r)
{
	int i;
	int ret;
	int found = 0;
	long long start,end;
	Variant sk;
	Variant sv;
	char key[KSIZE + 1];

	start = get_ustime_sec();
	for (i = 0; i < count; i++) {
		memset(key, 0, KSIZE + 1);

		/* if you want to test random write, use the following */
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		ret = db_get(db, &sk, &sv);
		if (ret) {
			//db_free_data(sv.mem);
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}

		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}

	end = get_ustime_sec();

	pthread_mutex_lock(&read_lock);
	totalReadTime += end - start;
	readCount += count;
	totalFound += found;
	pthread_mutex_unlock(&read_lock);
}
