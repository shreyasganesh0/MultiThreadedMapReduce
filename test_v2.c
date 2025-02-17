#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <unistd.h>

// GLOBAL VOLATILE FLAG FOR MAPPER... lets reducers know that if mapper is done, and they're buffer is empty then they can safely terminate/join
volatile int mapper_done = 0;

// Struct to store our data in dynamic array
struct Tuple {
    unsigned id;
    int weight;
    char topic[16];
};

// Buffer struct
struct Buffer {
    int id;
    int in;
    int out;
    int capacity;
    int em_flag;
    struct Tuple *tuple_array;
    sem_t mutex;
    sem_t full;
    sem_t empty;
};

// Arguments for mapper
struct MapperArgs {
    struct Buffer *buffers;
    int num_buffs;
    int num_slots;
};

// /* PRODUCER/MAPPER THREAD*/
void *mapper(void *arg) {
    // Extract all the data from struct and put it into usable variables
    struct MapperArgs *args = (struct MapperArgs *)arg;
    struct Buffer *buffs = args->buffers;
    int num_buffs = args->num_buffs;
    int num_slots = args->num_slots;

   char input[24];
    
    // Poll for new lines available
    while (fgets(input, sizeof(input), stdin) != NULL) {
        // Remove newline character
        input[strcspn(input, "\n")] = '\0';

        unsigned int id;
        char weight;
        char topic[16];     // Need 1 more char in order for null char from sscanf
        int weight_num = 0;

        // Extract text from input
        if (sscanf(input, "(%u,%c,%15[^)])", &id, &weight, topic) == 3) {
            // Map char to weight
            if (weight == 'P'){
                weight_num = 50;
            }
            else if (weight == 'L') {
                weight_num = 20;
            }
            else if (weight == 'D') {
                weight_num = -10;
            }
            else if (weight == 'C'){
                weight_num = 30;
            }
            else if (weight == 'S') {
                weight_num = 40;
            }
            else {
                printf("Invalid Weight.");
                break;
            }


            // Print the userID in format
            //printf("(%04u,%s,%d)\n", id, topic, weight_num);

            /* INSERT INTO BUFFER */

            // Bc each buffer already has a thread assigned to it we need to go through each one first and look for a matching id
            // If no matching ID is found we enter into the first available slot ID = -1. This also takes care of 1st processed ID and new ID's

            // Search for slot to insert
            int found = -1;
            for (int i = 0; i < num_buffs; i++) {
                if (buffs[i].id == id) {
                    found = i;
                    break;
                }
            }

            // This means there is a found thing
            if (found != -1) {
                // Ensure buffer is empty
                sem_wait(&buffs[found].empty);
                // Lock buffer
                sem_wait(&buffs[found].mutex);
                // Create a new tuple to store in the array
                struct Tuple newTuple;
                newTuple.id = id;
                newTuple.weight = weight_num;
                
                // Copy the topic ensuring null termination
                strncpy(newTuple.topic, topic, sizeof(newTuple.topic) - 1);
                newTuple.topic[sizeof(newTuple.topic) - 1] = '\0';
                
                // Now insert new tuple into in position in buffer(tuple_array)
                buffs[found].tuple_array[buffs[found].in] = newTuple;
                // Print added tuple
                printf("Mapper Added: (%04u,%s,%d)\n", buffs[found].tuple_array[buffs[found].in].id, buffs[found].tuple_array[buffs[found].in].topic,buffs[found].tuple_array[buffs[found].in].weight);
                
                // Update in index
                buffs[found].in = (buffs[found].in + 1) % buffs[found].capacity;
		buffs[found].em_flag = 0;


                // Unlock buffer
                sem_post(&buffs[found].mutex);
                
                // Signal new item available for consumption
                sem_post(&buffs[found].full);
            }
            else {
                // Now that we know what slot is available simply insert
                // If no slot was available then slot will still be -1 meaning this is the first processd ID and we can insert into the first buffer
                for (int i = 0; i < num_buffs; i++) {
                    // If no matching ID then insert into first available buffer and break
                    if (buffs[i].id == -1) {

                        sem_wait(&buffs[i].empty);
                        sem_wait(&buffs[i].mutex);

                        buffs[i].id = id;

                        // Create a new tuple to store in the array
                        struct Tuple newTuple;
                        newTuple.id = id;
                        newTuple.weight = weight_num;

                        // Copy the topic ensuring null termination
                        strncpy(newTuple.topic, topic, sizeof(newTuple.topic) - 1);
                        newTuple.topic[sizeof(newTuple.topic) - 1] = '\0';

                        // Now insert new tuple into in position in buffer(tuple_array)
                        buffs[i].tuple_array[buffs[i].in] = newTuple;

                        // Print added tuple
                        printf("Mapper Added:(%04u,%s,%d)\n", buffs[i].tuple_array[buffs[i].in].id, buffs[i].tuple_array[buffs[i].in].topic,buffs[i].tuple_array[buffs[i].in].weight);

                        // Update in index
                        buffs[i].in = (buffs[i].in + 1) % buffs[i].capacity;
		        buffs[i].em_flag = 0;
                        // ATTEMPT TO FIX
                        sem_post(&buffs[i].mutex);
                        sem_post(&buffs[i].full);
                        break;
                    }
                }
            }

            
        }
        else {
            printf("Invalid input format.\n");
        }
    }
    //printf("Mapper done\n");

    mapper_done = 1;
    free(args);
    return NULL;
}

/* CONSUMER/REDUCER THREAD*/
void *reducer(void *arg) {
    // Extract the buffer that was passed in
    struct Buffer *buff = (struct Buffer *)arg;

    // Create the dynamic array to hold entries
    struct Tuple *tuples = NULL;
    int num_tuples = 0;

    // Start processing from buffer
    // Because we want the reducer to always be consuming use a while loop and set up a condition within to break when buffer is empty and mapper is done
    int token = 0;
    while (1) {
	    
        /*      IN THE CASE THE BUFFER IS NOT EMPTY PROCESS NEXT TUPLE       */ 
        // Wait for new entry in buffer
        sem_wait(&buff->full);

        // Now that there is a new entry lock the buffer so we can access tuple safely
        sem_wait(&buff->mutex);

	    int isEmpty = (buff->in == buff->out);
	    if (mapper_done && isEmpty && buff->em_flag) {
		printf("Hit reducer end\n");
		break;
	    }	    

        //printf("Mutex grabbed in reducer.\n");

        // Extract the tuple from the out slot from the buffer (View PThreads.pdf slide 52 for refresher)
	struct Tuple newTuple;
	newTuple.id = buff->tuple_array[buff->out].id;
	strcpy(newTuple.topic, buff->tuple_array[buff->out].topic);
	newTuple.weight = buff->tuple_array[buff->out].weight;

	printf("Extracted Tuple.\n  ID: %04u\n    Topic: %s\n   Weight: %d\n", newTuple.id, newTuple.topic, newTuple.weight);

	if (buff->out == buff->in) {
		buff->em_flag = 1;
	}	
// Update out slot counter for circular buffer
	buff->out = (buff->out + 1) % buff->capacity;
        // Unlock the buffer (increment the semaphor lock counter)
        sem_post(&buff->mutex);

        // Signal that a tuple has been removed from buffer
	sem_post(&buff->empty);
        

	int slot = -1;
	for (int i = 0; i < num_tuples; i++) {
	    // Have to use string compare, will return 0 if the same
	    if (strcmp(newTuple.topic, tuples[i].topic) == 0) {
		slot = i;
		break;
	    }
	}

	//printf("Slot value %d.\n",slot);

	// If a matching topic was found
	if (slot != -1) {
	    tuples[slot].weight += newTuple.weight;
	    //printf("Slot found and new slot weight is %d.\n", tuples[slot].weight);
	}
	// If no matching topic was found
	else {
	    // Increment number of tuples and reallocate memory with new size
	    num_tuples++;
	    printf("Here in %d reducer for num_tuples %d\n", newTuple.id, num_tuples);
	    struct Tuple *temp = realloc(tuples, num_tuples * sizeof(struct Tuple));
	    // Copy back to tuples
	    tuples = temp;
	    // Enter new tuple
	    tuples[num_tuples - 1].id = newTuple.id;
	    tuples[num_tuples - 1].weight = newTuple.weight;
	    strcpy(tuples[num_tuples - 1].topic, newTuple.topic);

	    //printf("Slot NOT found entering new slot into array.\n  ID: %04u\n    Topic: %s\n   Weight: %d\n", tuples[num_tuples - 1].id, tuples[num_tuples - 1].topic, tuples[num_tuples - 1].weight);
	}
        token++;
    }

    printf("Num tuples is %d\n", num_tuples);
    for (int i = 0; i < num_tuples; i++) {
        printf("(%04u,%s,%d)\n", tuples[i].id, tuples[i].topic, tuples[i].weight);

    }
    
    free(tuples);
    return NULL;
}

int main(int argc, char *argv[]) {
    // Err checking for input args
    if (argc != 3) {
        fprintf(stderr,"Incorrect number of arguments.");
        return 1;
    }

    // Extract number of buffers and how many slots per buffer
    int num_slots = atoi(argv[1]);
    int num_buffs = atoi(argv[2]);
    
    printf("Found slots %d, and buff %d \n", num_slots, num_buffs);
    // Allocate enough memory for an array of buffers
    struct Buffer *buffer_array = malloc(num_buffs * sizeof(struct Buffer));

    // Initialize each buffer in buffer array
    for (int i = 0; i < num_buffs; i++) {
        buffer_array[i].id = -1;        // Use -1 to show unused buffer. bc ID never will be negative
        buffer_array[i].in = 0;
        buffer_array[i].out = 0;
        buffer_array[i].em_flag = 0;
        buffer_array[i].capacity = num_slots;
        buffer_array[i].tuple_array = malloc(num_slots * sizeof(struct Tuple));
        sem_init(&buffer_array[i].mutex, 0, 1);
        sem_init(&buffer_array[i].full, 0, 0);
        sem_init(&buffer_array[i].empty, 0, num_slots);
    }

    // Move the buffer array we just created into a struct for easy passing to mapper thread
    struct MapperArgs *mapper_args = malloc(sizeof(struct MapperArgs));
    mapper_args->buffers = buffer_array;
    mapper_args->num_buffs = num_buffs;
    mapper_args->num_slots = num_slots;

    // Create the mapper thread (FIXED: pass mapper_args instead of NULL)
    pthread_t mapper_thread;
    if (pthread_create(&mapper_thread, NULL, mapper, mapper_args) != 0) {
        perror("Error creating mapper thread.");
        exit(1);
    }
    
    // Create reducer threads an give each thread its own buffer
    pthread_t reducers[num_buffs];
    // int num_threads = 0;
    for (int i = 0; i < num_buffs; i++)
    {
        pthread_create(&reducers[i], NULL, reducer, &buffer_array[i]);
        // num_threads++;
    }
    // printf("%d\n",num_threads);

    // Wait for the mapper thread to finish
    if (pthread_join(mapper_thread, NULL) != 0) {
        perror("Error joining mapper thread.");
        exit(1);
    }

    // Wait for reducers to finish
    for (int i = 0; i < num_buffs; i++) {
        pthread_join(reducers[i], NULL);
    }

    // Cleanup
    for (int i = 0; i < num_buffs; i++) {
        free(buffer_array[i].tuple_array);
        sem_destroy(&buffer_array[i].mutex);
        sem_destroy(&buffer_array[i].full);
        sem_destroy(&buffer_array[i].empty);
    }
    free(buffer_array);

    return 0;
}
