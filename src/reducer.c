void *reducer(void *argc){
    int idx = *(int *)argc;

    comm_buf_t *curr_buf = &comm_buf[idx];

    hash_map_t *topic_score_map = create_hashmap(INITIAL_CAPACITY);        
    bool flag = true;

    do{
        sem_wait(&curr_buf->empty); 
        pthread_mutex_lock(&curr_buf->mutex);

        while(curr_buf->out_buf_loc >= 0){

            tuple_t curr_tup = curr_buf->tuple_buf[curr_buf->out_buf_loc];
            curr_buf->out_buf_loc--;
            curr_buf->in_buf_loc--;
            if (!strcmp(curr_tup.topic,"\0")){ 
                hashmap_iterate(curr_buf->topic_score_map, idx);
                pthread_mutex_unlock(&curr_buf->mutex); //maybe not needed?
                return NULL;
            }
            else{
                hashmap_insert(curr_buf->topic_score_map, curr_tup.topic, &curr_tup.score);
            }
        }

        pthread_mutex_unlock(&curr_buf->mutex);
        sem_post(&curr_buf->full);
        
    } while(1);

    return NULL;
}
