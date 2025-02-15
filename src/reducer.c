void *reducer(void *argc){
    int idx = *(int *)argc;

    comm_buf_t *curr_buf = &comm_buf[idx];

    do{
        sem_wait(&curr_buf->full); 
        pthread_mutex_lock(&curr_buf->mutex);

	int indx = (curr_buf->out_buf_loc % num_slots);
        int sem_post_count = 0;
        while(curr_buf->in_buf_loc != curr_buf->out_buf_loc){

            tuple_t *curr_tup = &curr_buf->tuple_buf[indx];
            if (!strcmp(curr_tup->topic,"xeof")){ 
                hashmap_iterate(curr_buf->topic_score_map, curr_buf->userID);
                pthread_mutex_unlock(&curr_buf->mutex); //maybe not needed?
                return NULL;
            }
            else{
                hashmap_insert(curr_buf->topic_score_map, curr_tup->topic, curr_tup->score);
            }
            curr_buf->out_buf_loc++;
	    indx = (indx + 1) - ((indx + 1) >= num_slots) * num_slots;
            sem_post_count++;
        }

        pthread_mutex_unlock(&curr_buf->mutex);
	for (int i = 0; i < sem_post_count; i++) {
       	    sem_post(&curr_buf->empty);
        }
        
    } while(1);

    return NULL;
}
