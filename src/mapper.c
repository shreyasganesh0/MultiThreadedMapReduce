void *mapper(void *argc){
    printf("In the mapper\n");

    while(1){
        
        mapper_t mapper_inp; 

        char buf[30];

        if (fgets(buf, sizeof(buf), stdin) != NULL){
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
            if(sscanf(buf, "(%[^,],%[^,],%[^)])", mapper_inp.userID, &mapper_inp.action, mapper_inp.topic) == 0){
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

            int idx = atoi(mapper_inp.userID) - 1;
            comm_buf_t *curr_buf = &comm_buf[idx];

            sem_wait(&curr_buf->full);
            pthread_mutex_lock(&curr_buf->mutex);

            strcpy(curr_buf->tuple_buf[curr_buf->in_buf_loc].topic, mapper_inp.topic); 
            curr_buf->tuple_buf[curr_buf->in_buf_loc].score = score; 
            curr_buf->in_buf_loc++;

            pthread_mutex_unlock(&curr_buf->mutex);
            sem_post(&curr_buf->empty);
                
        }
        else{
            if (feof(stdin)){
		printf("EOF found\n");
                for (int i = 0; i < comm_buf[0].capacity; i++){ // maybe have a check for number for illegal access comm_buf[0]
                    comm_buf_t *curr_buf = &comm_buf[i];
                    sem_wait(&curr_buf->full);
                    pthread_mutex_lock(&curr_buf->mutex);

                    curr_buf->tuple_buf[curr_buf->in_buf_loc].topic[0] = '\0'; 
                    curr_buf->tuple_buf[curr_buf->in_buf_loc].score = -1; 
                    curr_buf->in_buf_loc++;
                    curr_buf->out_buf_loc++;

                    pthread_mutex_unlock(&curr_buf->mutex);
                    sem_post(&curr_buf->empty);
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
