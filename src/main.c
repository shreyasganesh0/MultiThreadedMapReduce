#include <stdlib.h>
#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif
#include <pthread.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "combiner.h"
#include "hashmap.c"
#include "mapper.c"
#include "reducer.c"


int main(int argc, char *argv[]){

    if (argc < 3){
        printf("Too few arguments\n");
        return -1;
    }

    int num_slots = atoi(argv[1]);
    int num_users = atoi(argv[2]);

    comm_buf = (comm_buf_t *)malloc(num_users * sizeof(comm_buf_t));

    pthread_t mapper_tid;
    int *red_idxs = (int *)malloc(num_users * sizeof(int));
    pthread_t *reducer_tids = (pthread_t *)malloc(num_users * sizeof(pthread_t));
    if (red_idxs == NULL || reducer_tids == NULL){
        printf("Failure to allocate space for buffer\n");
        return -1;
    }


    for (int i = 0; i < num_users; i++){
        sem_init(&comm_buf[i].empty, 0, 0);
        sem_init(&comm_buf[i].full, 0, num_slots);
        comm_buf[i].tuple_buf = (tuple_t *)malloc(num_slots * sizeof(tuple_t));
        if (comm_buf[i].tuple_buf == NULL){
            printf("Failure to allocate space for buffer\n");
            return -1;
        }
        pthread_mutex_init(&comm_buf[i].mutex, NULL);
        comm_buf[i].in_buf_loc = 0;
        comm_buf[i].out_buf_loc = 0;
        comm_buf[i].capacity = num_slots;
        comm_buf[i].topic_score_map = create_hashmap(INITIAL_CAPACITY);

        red_idxs[i] = i;
        int pred_err = pthread_create(&reducer_tids[i], NULL, reducer, &red_idxs[i]);
        if (pred_err != 0){
            printf("Thread creation failure\n");
            return -1;
        }
    }

    int pmap_err = pthread_create(&mapper_tid, NULL, mapper, NULL);
    if (pmap_err != 0){
        printf("Thread creation failure\n");
        return -1;
    }

    pthread_join(mapper_tid, NULL);

    for (int i = 0; i < num_users; i++){
        pthread_join(reducer_tids[i], NULL);
        pthread_mutex_destroy(&comm_buf[i].mutex);
        sem_destroy(&comm_buf[i].empty);
        sem_destroy(&comm_buf[i].full);
        free(comm_buf[i].tuple_buf);
    }

    //free(reducer_tids);
    //free(red_idxs);
    return 0;

    
}

