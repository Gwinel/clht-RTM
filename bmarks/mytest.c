/*
 *	Filename: test.c
 *	Author: (Aimlab)Zhiwen Chan
 *  Description: design this file as a test module 
 *				 for concurrent hashtables.
 *	
 *
 */

# include <stdio.h>
# include <sys/time.h>
# include <getopt.h>
# include <pthread.h>
# include <time.h>
# include <malloc.h>
# include <unistd.h>
# include <stdlib.h>
# include "utils.h"
# include "clht.h"

/*---------------------------------------------------------------------
			GLOBAL VALUES
---------------------------------------------------------------------*/
clht_hashtable_t** hashtable;
int duration =  1000;
int num_buckets = 1024;
int num_elements = 2048;
int num_threads = 1;
double filling_rate = 0.5;
double update_rate = 0.1;
double put_rate = 0.1;
double get_rate = 0.1;
size_t key;
int rand_max;
int *putting_count ;
int *getting_count ;
int *removing_count;
unsigned long *seeds;
unsigned long phys_id;
struct timeval start, end, t1;
//struct timespec timeout;
//timeout.tv_sec = duration / 1000;
//timeout.tv_nsec = (duration % 1000) * 1000000;

static volatile int stop;
typedef struct thread_data{
	int id;
	clht_t* ht;
}thread_data_t;

/*###########################################
 * BARRIER
 *##########################################*/
typedef struct barrier{
	pthread_cond_t complete;
	pthread_mutex_t mutex;
	int count;
	int crossing;
}barrier_t;

void barrier_init(barrier_t *b, int n){
	pthread_cond_init(&b->complete, NULL);
	pthread_mutex_init(&b->mutex, NULL);
	b->count = n;
	b->crossing = 0;
}

void barrier_cross(barrier_t *b){
	pthread_mutex_lock(&b->mutex);
	b->crossing++;
	if(b->crossing < b->count)
		pthread_cond_wait(&b->complete, &b->mutex);
	else{
		pthread_cond_broadcast(&b->mutex);
		b->crossing = 0;
	}
	pthread_mutex_unlock(&b->mutex);
}

//pthread_barrier_t b, b2;

/*++++++++++++++++++++++++++++++++++++++++++++++++
 * test()
 *++++++++++++++++++++++++++++++++++++++++++++++++*/
void* test(void *arg){
	uint64_t key;
	uint64_t num_elems_thread = (uint64_t) (num_elements * filling_rate)/num_threads;
	int rem_threshold = put_rate*100;
	int put_threshold = update_rate*100;

	thread_data_t *data = (thread_data_t *)arg;
	int ID = data->id;
	phys_id = the_cores[ID];
	set_cpu(phys_id);

	clht_t* hashtable = data->ht;
	printf("Test thread id: %d\n", ID);

	int my_putting_count = 0;
	int my_getting_count = 0;
	int my_removing_count = 0;
	int i;
		
	srand((unsigned int) time(NULL));
	//char* obj = NULL;
	while(stop == 0){
		int number = rand()%100+1;
		key = (my_random(&(seeds[0]), &(seeds[1]), &(seeds[2]))) & rand_max;
		//printf("rand_num = %d\t",number);
		if(number <= rem_threshold){
			int removed ;
			removed = clht_remove(hashtable, key);
			//printf("removed = %d\n",removed);	
			my_removing_count++;
		}
		
		else if(number <= put_threshold){
			int res;
			res = clht_put(hashtable, key, key+1);
			//printf("res = %d\n");
			my_putting_count++;
		}
		
		else{
			clht_val_t res;
			res = clht_get(hashtable->ht, key);
			my_getting_count++;
		}
	}

	putting_count[ID] += my_putting_count;
	getting_count[ID] += my_getting_count;
	removing_count[ID] += my_removing_count;

	pthread_exit(NULL);
}

/*
int is_power_of_two(unsigned int x){
	return ((x != 0 ) && !(x & (x - 1)));
}
*/

/// Round up to next higher power of 2 (return x if it's already a power
/// of 2) for 32-bit numbers
/*
static long int pow2roundup(long int x){
	if(x == 0) return 1;
	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x |= x >> 32;
	return x+1;
}
*/

/* main function */
int main(int argc, char **argv){
	set_cpu(the_cores[0]);
 	struct timespec timeout;
	timeout.tv_sec = duration / 1000;
	timeout.tv_nsec = (duration % 1000) * 1000000;
	
	struct option long_options[] = {
		// These options don't set a flag
		{"help",			no_argument,		NULL, 'h'},
		{"duration",		required_argument,	NULL, 'd'},
		{"initial-size",	required_argument,	NULL, 'i'},
		{"load-factor",		required_argument,	NULL, 'l'},
		{"update-rate",		required_argument,	NULL, 'u'},
		{"range",		required_argument,	NULL, 'r'},
		{"num-buckets",		required_argument,	NULL, 'b'},
		{"num-threads",		required_argument,	NULL, 't'},
		{"put-rate",		required_argument,	NULL, 'p'},
		{"filling-rate",	required_argument,	NULL, 'f'},
		{NULL, 0, NULL, 0}
	};

	size_t initial = 1024, range = 2048, update = 20, load = 50, put = 10; 
	double load_factor = 2;
	int put_explicit = 0;

	int i, c;
	while(1)
	{
		i = 0;
		c = getopt_long(argc, argv, "hAf:d:i:l:u:r:b:n:p:f", long_options, &i);

		if(c == -1)
			break;

		if(c == 0 && long_options[i].flag == 0)
			c = long_options[i].val;

		switch(c)
		{
			case 0:
				break;
			case 'h':
				printf("Usage:\n"
					   "  intset [options...]\n"
					   "\n"
					   "Options:\n"
					   "	-h, --help\n"
					   "			Print this message\n"
					   "	-d, --duration\n"
					   "			Test duration in millisecond\n"
					   "	-i, --initial-size\n"
					   "			Number of elements to insert before test\n"
					   "	-l, --load-factor\n"
					   "			Load factor of initial hash table\n"
					   "	-u, --update-rate\n"
					   "			The ratio of update operation to total operations\n"
					   "	-r, --range\n"
					   "			Range of integer value inserted in the table\n"
					   "	-b, --num-bucktes\n"
					   "			Number of initial buckets\n"
					   "	-n, --num-threads\n"
					   "			Number of threads\n"
					   "	-p, --put-rate\n"
					   "			The ratio of insert operation to total operation\n"
					   "	-f, --filling-rate\n"
					   "			The filling rate of hash table(0<filling_rate<=1)\n"
					   "!!! Watch out: each argument shoud be power of 2 !!!\n");
				exit(0);
				case 'd':
					duration = atoi(optarg);
					break;
				case 'i':
					initial = atoi(optarg);
					break;
				case 'l':
					load_factor = atof(optarg);
					break;
				case 'u':
					update = atoi(optarg);
					break;
				case 'r':
					range = atoi(optarg);
					break;
				case 'n':
					num_threads = atoi(optarg);
					break;
				case 'p':
					put_explicit = 1;
					put = atoi(optarg);
					break;
				case 'f':
					filling_rate = atof(optarg);
					break;
				case '?':
				default:
					printf("Use -h or --help for help.\n");
					exit(1);

		}
	}
	
	if(range < initial)
		range = 2 * initial ;

	if(!is_power_of_two(initial)){
		size_t initial_pow2 = pow2roundup(initial);
		printf("** rounding up initial (to make it power of 2): old: %d / new: %d\n", initial, initial_pow2);
		initial = initial_pow2;
	}

	if(!is_power_of_two(range)){
		size_t range_pow2 = pow2roundup(range);
		printf("** rounding up range (to make it power of 2): old: %d / new: %d\n", range, range_pow2);
		range = range_pow2 ;
	}
	
	num_buckets = initial / 2;
	if(!is_power_of_two(num_buckets)){
		size_t num_buckets_pow2 = pow2roundup(num_buckets);
		printf("** rounding up num_buckets (to make it power of 2): old: %d / new: %d\n", num_buckets, num_buckets_pow2);
		num_buckets = num_buckets_pow2 ;
	}

	if(put > update)
		put = update;

	num_elements = (int)(initial / filling_rate) ;
	rand_max = num_elements - 1;
	//load_factor = load / 100.0 ;
	update_rate = update / 100.0;
	if(put_explicit)
	{
		put_rate = put / 100.0;
	}
	else
	{
		put_rate = update_rate / 2;
	}
	get_rate = 1 - update_rate ;

	printf("num_elements = %d\n", num_elements);
	printf("filling_rate = %f\n", filling_rate);
	printf("initial = %d\n", initial);
	printf("range = %d\n", range);
	printf("load_factor = %f\n\n", load_factor);
	/* create a clht hashtable*/
	clht_t* hashtable = clht_create(num_buckets);
	assert(hashtable != NULL);

	int putting_count_total = 0;
	int getting_count_total = 0;
	int removing_count_total = 0;
	int total_operations = 0;
	putting_count = (int*) calloc(num_threads, sizeof(int));
	getting_count = (int*) calloc(num_threads, sizeof(int));
	removing_count = (int*) calloc(num_threads, sizeof(int));
	seeds = (unsigned long*) calloc(3, sizeof(unsigned long));
	seeds = seed_rand();

	/* Create threads to do sth. */
	pthread_t tid[num_threads];
	pthread_attr_t attr;
	int  j, rc;

	/* Initial the hashtable in main thread */
	int num_init = num_elements * filling_rate ;
	for(j = 0; j < num_init; j++){
		key = my_random(&(seeds[0]), &(seeds[1]), &(seeds[2])) % rand_max ;
		if(!clht_put(hashtable, key, (clht_val_t)key)){
			j--;
		}		
	}

	//pthread_barrier_init(&b2, NULL, num_threads + 1);
	//pthread_barrier_init(&b, NULL,  num_threads);

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	thread_data_t* tds = (thread_data_t*)malloc(num_threads * sizeof(thread_data_t));
	gettimeofday(&start, NULL);
	
	stop = 0;
	for(j = 0; j < num_threads; j++){
		tds[j].id = j;
		tds[j].ht = hashtable;
		rc = pthread_create(&tid[j], &attr, test, &tds[j]);
		if(rc){
			printf("thread create failed...\n");
			exit(-1);
		}

	}
	//pthread_barrier_wait(&b);
	pthread_attr_destroy(&attr);	
	//barrier_cross(&barrier_global);
	sleep(duration/1000);
	stop = 1;
	for(j = 0; j < num_threads; j++){
		pthread_join(tid[j], NULL);
	}

	gettimeofday(&end, NULL);
	
	for(j = 0; j < num_threads; j++){
		putting_count_total += putting_count[j];
		getting_count_total += getting_count[j];
		removing_count_total += removing_count[j];
		total_operations += (putting_count[j] + getting_count[j] + removing_count[j]);
	}
	
	//total_operations = putting_count_total + getting_count_total + removing_count_total;
	double throughput = (total_operations*1000.0)/duration;	
	duration = (end.tv_sec*1000 + end.tv_usec/1000) - (start.tv_sec*1000 + start.tv_usec/1000);
	printf("Number of insert: %d\n", putting_count_total);
	printf("Number of remove: %d\n", removing_count_total);
	printf("Number of lookup: %d\n", getting_count_total);
	printf("total throughput: %.2f(Mops/s)\n", throughput/ 1e6);
	printf("time elapsed of multiple threads is : %d (ms).\n", duration);
        
	//clht_gc_destory(hashtable);	
	pthread_exit(NULL);
	return 0;
}


