unsigned long hash_function(const char *key){
    unsigned long hash = 5381; // defined in the DJB2
    int c;
    while((c = *key++)){
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

hash_map_t *create_hashmap(int capacity){
    hash_map_t *hash_map = malloc(sizeof(hash_map_t));
    hash_map->capacity = capacity;
    hash_map->size = 0;
    hash_map->nodes = calloc(capacity, sizeof(hash_node_t));
    return hash_map;
}

void hashmap_iterate(hash_map_t *map, char *userID){
    if (map == NULL){
        printf("Empty Map\n");
        return;
    }

    hash_node_t *node = map->nodes;
    for (int i = 0; i < map->capacity; i++){
        if (node[i].is_occupied == 1){
            printf("(%4s,%s,%d)\n", userID, node[i].key, node[i].value);
            node[i].value = -1;
            node[i].is_occupied = 0;
            map->size -= 1;
        }
    }
    return;
}

void hashmap_insert(hash_map_t *map, const char *key, int value) {
    if ((float)map->size / map->capacity > LOAD_FACTOR) {
        // TODO: resize hashmap 
    }

    unsigned long hash = hash_function(key);
    int index = hash % map->capacity;

    while (map->nodes[index].is_occupied) {
        // If the key already exists, update the value
        if (strcmp(map->nodes[index].key, key) == 0) {
            map->nodes[index].value += value;
            map->nodes[index].is_occupied = 1;
            map->size++;
            return;
        }
        index = (index + 1) % map->capacity;  // Linear probing
    }

    map->nodes[index].key = strdup(key);
    map->nodes[index].value = value;
    map->nodes[index].is_occupied = 1;
    map->size++;
}

int hashmap_get(hash_map_t *map, const char *key) {
    unsigned long hash = hash_function(key);
    int index = hash % map->capacity;

    while (map->nodes[index].is_occupied) {
        
        if (strcmp(map->nodes[index].key, key) == 0) {
            return map->nodes[index].value; 
        }
        index = (index + 1) % map->capacity;  // Linear probing
    }

    return -1;  // Key not found
}

void hashmap_delete(hash_map_t *map, const char *key) {
    unsigned long hash = hash_function(key);
    int index = hash % map->capacity;

    while (map->nodes[index].is_occupied) {
        if (strcmp(map->nodes[index].key, key) == 0) {
            free(map->nodes[index].key);
            map->nodes[index].key = NULL;
            map->nodes[index].value = -1;
            map->nodes[index].is_occupied = 0;
            map->size--;
            return;
        }
        index = (index + 1) % map->capacity;
    }
}

void free_hashmap(hash_map_t *map) {
    for (int i = 0; i < map->capacity; i++) {
        if (map->nodes[i].is_occupied) {
            free(map->nodes[i].key);
        }
    }
    free(map->nodes);
    free(map);
}

