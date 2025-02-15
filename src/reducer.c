void *reducer(void *argc){
    printf("In the reducer\n");
    int idx = *(int *)argc;

    printf("%d\n", idx);
    comm_buf_t *curr_buf = &comm_buf[idx];
    printf("In buf loc %d\n", curr_buf->in_buf_loc);

    do{
        sem_wait(&curr_buf->full); 
        pthread_mutex_lock(&curr_buf->mutex);

        while(curr_buf->in_buf_loc >= 0){

            tuple_t *curr_tup = &curr_buf->tuple_buf[curr_buf->in_buf_loc];
	    printf("Locked the buffer in this state %s, %d for idx %d\n", curr_tup->topic, curr_tup->score, idx);
            if (!strcmp(curr_tup->topic,"endoffile")){ 
                hashmap_iterate(curr_buf->topic_score_map, idx + 1);
                pthread_mutex_unlock(&curr_buf->mutex); //maybe not needed?
                return NULL;
            }
            else{
                hashmap_insert(curr_buf->topic_score_map, curr_tup->topic, curr_tup->score);
            }
            curr_buf->in_buf_loc--;
       	    sem_post(&curr_buf->empty);
        }
	curr_buf->in_buf_loc = 0;

        pthread_mutex_unlock(&curr_buf->mutex);
        
    } while(1);

    return NULL;
}
