#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "../engine/db.h"

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char* key, int length);

pthread_mutex_t write_lock;
pthread_mutex_t read_lock;

// Global file
DB* db;
// Globals to find the total time spent 
double totalReadTime;
double totalWriteTime;

// Globals to calculate times for the prints
// write
long int writeCount;

// read
int totalFound;
long int readCount;