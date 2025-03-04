#ifndef COMBINER_H
#define COMBINER_H

#define BUCKET_COUNT 6
#define LOAD_FACTOR 0.75
#define INITIAL_CAPACITY 2000

typedef struct {
    char userID[5];
    char action;
    char topic[16];
} mapper_t;

typedef struct {
    char topic[16];
    int score;
} tuple_t;

typedef struct {
    char *key;
    int value;
    int is_occupied;
} hash_node_t;

typedef struct {
    hash_node_t *nodes;
    int capacity;
    int size;
} hash_map_t;

typedef struct {
    sem_t empty;
    sem_t full;
    pthread_mutex_t mutex;
    hash_map_t *topic_score_map;
    tuple_t *tuple_buf;
    char userID[5];
    int in_buf_loc;
    int out_buf_loc;
    int taken;
} comm_buf_t;


comm_buf_t *comm_buf;
int num_users;
int num_slots;

unsigned long hash_function(const char *key);

hash_map_t *create_hashmap(int capacity);

void hashmap_iterate(hash_map_t *map, char *userID);

void hashmap_insert(hash_map_t *map, const char *key, int value);

int hashmap_get(hash_map_t *map, const char *key);

void hashmap_delete(hash_map_t *map, const char *key);

void free_hashmap(hash_map_t *map);

void *mapper(void *arg);

void *reducer(void *arg);

#endif
