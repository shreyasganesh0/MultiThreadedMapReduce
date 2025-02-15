void *mapper(void *argc){
    printf("In the mapper\n");

    while(1){
        
        mapper_t mapper_inp; 

        char buf[30];

        if (fgets(buf, sizeof(buf), stdin) != NULL) {
            size_t buf_len = strlen(buf);
            if (buf_len > 0 && buf[buf_len - 1] == '\n'){
                buf[buf_len - 1] = '\0';
            }
            if (strlen(buf) > 4+1+15+5){
                printf("Input too strong\n");
                continue;
            }

            if(!strcmp(buf, "exit")){
                printf("exit\n");
                return 0;
            }
            if(sscanf(buf, "(%[^,],%[^,],%[^)])", mapper_inp.userID, &mapper_inp.action, mapper_inp.topic) == 0) {
                printf("Unable to parse input\n");
                continue;
            }

            int score = 0;
            switch(mapper_inp.action){
                case 'D':
                    {
                        score = -10;
                        break;
                    }
                case 'L':
                    {
                        score = 20;
                        break;
                    }
                case 'C':
                    {
                        score = 30;
                        break;
                    }
                case 'S':
                    {
                        score = 40;
                        break;
                    }
                case 'P':
                    {
                        score = 50;
                        break;
                    }
                default:
                    {
                        printf("Invalid entry to action\n");
                        continue;
                    }
            }

            int idx = atoi(mapper_inp.userID) - 1; // prefetch a few and buffer?
            comm_buf_t *curr_buf = &comm_buf[idx];


            sem_wait(&curr_buf->empty);
            pthread_mutex_lock(&curr_buf->mutex);
	    
	    int indx = curr_buf->in_buf_loc % num_slots;
	    if (indx >= num_slots) {
		printf("Out of bounds check\n");
		break;
            }

            strcpy(curr_buf->tuple_buf[indx].topic, mapper_inp.topic); 
            curr_buf->tuple_buf[indx].score = score; 
            curr_buf->in_buf_loc++;

            pthread_mutex_unlock(&curr_buf->mutex);
            sem_post(&curr_buf->full);
                
        }
        else{
            if (feof(stdin)){
                for (int i = 0; i < num_users; i++) { // maybe have a check for number for illegal access comm_buf[0]
                    comm_buf_t *curr_buf = &comm_buf[i];
                    sem_wait(&curr_buf->empty);
                    pthread_mutex_lock(&curr_buf->mutex);

	            int indx = curr_buf->in_buf_loc % num_slots;
                    strcpy(curr_buf->tuple_buf[indx].topic, "endoffile"); 
                    curr_buf->tuple_buf[indx].score = -1; 
                    curr_buf->in_buf_loc++;

                    pthread_mutex_unlock(&curr_buf->mutex);
                    sem_post(&curr_buf->full);
                }
            }
            else if (ferror(stdin)) {
                perror("Error reading input");
            } else {
                printf("Unknown error occurred.\n");
            }
            return NULL;
        }
    }
    return NULL;
}
