#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define DEFAULT 5
#define ITR_COUNT 2

//global variables
int product = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t c_mutex = PTHREAD_MUTEX_INITIALIZER;
int count = 0;
int demand = 0;

//prototypes:
int producer();
int consumer();

int main(int argc, char *argv[]){
    srand(time(NULL));
    int p_count = 1;
	int c_count = DEFAULT;
    int arg;
	
    if(argc > 1)
	{
        while((arg = getopt(argc, argv, "p:c:")) != -1)
		{
            switch(arg)
			{
                case 'p':
                    p_count = atoi(optarg);
                break;
				
                case 'c':
                    c_count = atoi(optarg);
                break;
				
                case '?':
                    abort();
                default:
                break;
            }
        }
    }
    pthread_mutex_lock(&c_mutex);
    printf("Producer(s): %d\n", p_count);
    printf("Consumer(s): %d\n", c_count);
	
    pthread_t *ptID = (pthread_t *) malloc(sizeof(pthread_t) * p_count);
    pthread_t *ctID = (pthread_t *) malloc(sizeof(pthread_t) * c_count);
	
	int p,c,i,j;
	
   for(p = 0; p < p_count; p++)
	{
        if(pthread_create(ptID+p, NULL, (void *) &producer, NULL) != 0)
            perror("producer thread");
    }

	
    for(c = 0; c < c_count; c++)
	{
        if(pthread_create(ctID+c, NULL, (void *) &consumer, NULL) != 0)
            perror("consumer thread");
    }

    for(i = 0; i < p_count; i++)
	{
        pthread_join(ptID[i], NULL);
    }
	
    for(j = 0; j < c_count; j++)
	{
        pthread_join(ctID[j], NULL);
    }
	
    return 0;
}

int producer(){
    int produce = 1 + ((double)rand() / (double) RAND_MAX) * 20;
	
    pthread_mutex_lock(&mutex);
	product += produce; // Add what we've produced to the stockpile.
	printf("Producer has stocked %d product.\n", produce);
	
	/* If there is demand for a product or if the surplus out weighs the demand, 
	 * then unlock the consumer lock. Let it consume. */
	if(demand > 0 && product > demand)
	{
		pthread_mutex_unlock(&c_mutex);
		//printf("Unlock Consumer lock.\n");
	}
	
	else
	{
		pthread_mutex_unlock(&mutex);
		//printf("Unlock mutex. \n");
	}
			
    return 0;
}

int consumer(){
    int take = ((double)rand() / (double) RAND_MAX) * 20;
	
    pthread_mutex_lock(&mutex_2);
	printf("TOTAL PRODUCT: %d\n", product);
	pthread_mutex_lock(&mutex);
	
	/* If there are enough things to consume, then consume them. */
	if(product >= take)
	{
		product -= take;
		printf("Consumer has removed %d product.\n", take);
		pthread_mutex_unlock(&mutex);
		pthread_mutex_unlock(&mutex_2);
	}
	
	/* We need to lock the consumer and unlock the producer. */
	else
	{
		printf("Need more product. \n");
		demand = take; // What we need.
		
		printf("Unlock producer. \n");
		pthread_mutex_unlock(&mutex);
		
		printf("Lock consumer. \n");
		pthread_mutex_lock(&c_mutex);
		
		printf("We are taking what we wanted earlier and set demand to zero. \n");
		product -= take;
		demand = 0;
		
		printf("[ELSE]Consumer has removed %d product.\n", take);
		pthread_mutex_unlock(&mutex);
		pthread_mutex_unlock(&mutex_2);
	}
			
    return 0;
}
