#include "restaurant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>

// // declaring mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
sem_t mutex;

int* vacancy_tables[5];
struct Queue* queues[5];
int count_tables[5];
int counter = 0;

// A structure to represent a queue
struct Queue {
    unsigned capacity;
    unsigned head, tail, size;
    pthread_cond_t** array;
    int* array2;
};

struct Queue* createQueue()
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = 1000;
    queue->head = queue->size = 0;
 
    // This is important, see the enqueue
    queue->tail = queue->capacity - 1;
    queue->array = (pthread_cond_t**)malloc(
        queue->capacity * sizeof(pthread_cond_t*));
    queue->array2 = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}

void enqueue(struct Queue* queue, pthread_cond_t* item, int i)
{
    if (queue->size == queue->capacity)
        return;
    queue->tail = (queue->tail + 1)
                  % queue->capacity;
    queue->array[queue->tail] = item;
    queue->array2[queue->tail] = i;
    queue->size = queue->size + 1;
}
 
pthread_cond_t* dequeue(struct Queue* queue)
{
    if (queue->size == 0)
        return NULL;
    pthread_cond_t* item = queue->array[queue->head];
    queue->head = (queue->head + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

pthread_cond_t* peek(struct Queue* queue)
{
    if (queue->size == 0)
        return NULL;
    pthread_cond_t* item = queue->array[queue->head];
    return item;
}

int peekIndex(struct Queue* queue)
{
    if (queue->size == 0)
        return INT_MAX;
    int index = queue->array2[queue->head];
    return index;
}

int getTableSize(int table_id) {
    for (int i = 0; i < 5; i++) {
        table_id -= count_tables[i];
        if (table_id <= 0) {
            return i + 1;
        }
    }
}

int getTableId(int num_people, int index) {
    int res = 0;
    for (int i = 0; i < num_people - 1; i++) {
        res += count_tables[i];
    }
    return res + index;
}

int getGroup(struct Queue** arr, int groupSize) {
    int currFront = INT_MAX;
    int res = INT_MAX;
    for (int i = 0; i < groupSize; i++) {
        // printf("size: %i , index %i at queue %i \n", groupSize, peekIndex(arr[i]), i);
        if (peekIndex(arr[i]) < currFront) {
            currFront = peekIndex(arr[i]);
            res = i;
        }
        // printf("currFront %i \n", currFront);
    }
    return res;
}

int reserveTable(int num_people) {
    int index = num_people - 1;
    int counter = 0;
    for (int j = 0; j < index; j++) {
        counter += count_tables[j];
        // printf("\nstarting for size: %i: ", count_tables[j]);
        // for (int k = 0; k < count_tables[j]; k++) {
        //     printf("%i,", vacancy_tables[j][k]);
        // }
    }
    for (int i = index; i < 5; i++) {
        for (int j = 0; j < count_tables[i]; j++) {
            if (vacancy_tables[i][j] == 1) {
                vacancy_tables[i][j] = 0;
                // printf("\nReserving %i, %i for id %i", i+1, j+1, counter+j);
                return counter + j;
            }
        }
        counter += count_tables[i];
    }
    return -1;
}

void unReserveTable(int table_id) {
    // printf("original %i", table_id);
    if (table_id < count_tables[0]) {
        vacancy_tables[0][table_id] = 1;
        // printf("\nReleasing 1, %i", table_id);
        return; 
    }

    for (int i = 1; i < 5; i++) {
        table_id -= count_tables[i - 1];
        if (table_id < count_tables[i]) {
            vacancy_tables[i][table_id] = 1;
            // printf("\nReleasing %i, %i", i+1, table_id);
            return;
        }
    }
}

void restaurant_init(int num_tables[5]) {
    sem_init(&mutex, 0, 1);
    for (int i = 0; i < 5; i++) {
        count_tables[i] = num_tables[i];
        vacancy_tables[i] = (int*) malloc(num_tables[i] * sizeof(int));
        queues[i] = createQueue();
        for (int j = 0; j <num_tables[i]; j++) {
            vacancy_tables[i][j] = 1;
        }
    }
}

void restaurant_destroy(void) {
    for (int i = 4; i >= 0; i--) {
        free(queues[i] -> array);
        free(queues[i]);
        free(vacancy_tables[i]);
    }
    pthread_cond_destroy(&cond1);
    pthread_mutex_destroy(&lock);
    sem_destroy(&mutex);
}

int request_for_table(group_state *state, int num_people) {
    int res;
    // store info into state
    state->group_size = num_people;
    // retrieve mutex and conds of tablesize
    int index = num_people - 1;
    // pthread_mutex_t lock = mutex_tables[index];
    pthread_mutex_lock(&lock);
    if (queues[index]->size == 0) {
        res = reserveTable(num_people);
    } else {
        res = -1;
    }
        
    on_enqueue();
    if (res == -1) {
        pthread_cond_t* cond = &cond1;
        sem_wait(&mutex);
        enqueue(queues[index], cond, counter);
        counter++;
        sem_post(&mutex);
        // printf("cond address a: %p \n", cond);
        pthread_cond_wait(cond, &lock);
        res = reserveTable(num_people);
        dequeue(queues[index]);
        // printf("cond address c: %p \n", cond);
    }
    state->table_id = res;
    pthread_mutex_unlock(&lock);
    return res;
}

void leave_table(group_state *state) {
    // int index = state->group_size - 1;
    // pthread_mutex_t lock = mutex_tables[index];

    pthread_mutex_lock(&lock);
    unReserveTable(state->table_id);

    // TODO: Wake a group with smaller group size than your release table
    int index = getGroup(queues, getTableSize(state->table_id) + 1);
    if (index != INT_MAX) {
        pthread_cond_t* cond = peek(queues[index]);
        if (cond != 0) {
            pthread_cond_signal(cond);
            // pthread_cond_destroy(cond);
        }
    }
    pthread_mutex_unlock(&lock);
}

// #include "restaurant.h"
// #include <stdio.h>
// #include <stdlib.h>
// #include <pthread.h>
// #include <unistd.h>
// #include <limits.h>
// #include <semaphore.h>


// // A structure to represent a queue
// struct Queue {
//     unsigned front, rear, size;
//     unsigned capacity;
//     pthread_cond_t** array;
//     int* array2;
// };

// struct Queue* createQueue()
// {
//     struct Queue* queue = (struct Queue*)malloc(
//         sizeof(struct Queue));
//     queue->capacity = 1000;
//     queue->front = queue->size = 0;
 
//     // This is important, see the enqueue
//     queue->rear = queue->capacity - 1;
//     queue->array = (pthread_cond_t**)malloc(
//         queue->capacity * sizeof(pthread_cond_t*));
//     queue->array2 = (int*)malloc(
//         queue->capacity * sizeof(int));
//     return queue;
// }

// int isFull(struct Queue* queue)
// {
//     return (queue->size == queue->capacity);
// }
 
// int isEmpty(struct Queue* queue)
// {
//     return (queue->size == 0);
// }

// void enqueue(struct Queue* queue, pthread_cond_t* item, int i)
// {
//     if (isFull(queue))
//         return;
//     queue->rear = (queue->rear + 1)
//                   % queue->capacity;
//     queue->array[queue->rear] = item;
//     queue->array2[queue->rear] = i;
//     queue->size = queue->size + 1;
// }
 
// pthread_cond_t* dequeue(struct Queue* queue)
// {
//     if (isEmpty(queue))
//         return NULL;
//     pthread_cond_t* item = queue->array[queue->front];
//     queue->front = (queue->front + 1)
//                    % queue->capacity;
//     queue->size = queue->size - 1;
//     return item;
// }

// pthread_cond_t* peek(struct Queue* queue)
// {
//     if (isEmpty(queue))
//         return NULL;
//     pthread_cond_t* item = queue->array[queue->front];
//     return item;
// }

// int peekIndex(struct Queue* queue)
// {
//     if (isEmpty(queue))
//         return INT_MAX;
//     int index = queue->array2[queue->front];
//     return index;
// }

// int getGroup(struct Queue** arr, int groupSize) {
//     int currFront = INT_MAX;
//     int res = INT_MAX;
//     for (int i = 0; i < groupSize; i++) {
//         // printf("size: %i , index %i at queue %i \n", groupSize, peekIndex(arr[i]), i);
//         if (peekIndex(arr[i]) < currFront) {
//             currFront = peekIndex(arr[i]);
//             res = i;
//         }
//         // printf("currFront %i \n", currFront);
//     }
//     return res;
// }

// // // declaring mutex
// pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// int *vacant1, *vacant2,*vacant3,*vacant4, *vacant5 = 0;
// int* vacancy_tables[5];
// struct Queue* queues[5];
// int count_tables[5];
// int counter = 0;
// sem_t mutex;


// int getTableSize(int table_id) {
//     for (int i = 0; i < 5; i++) {
//         table_id -= count_tables[i];
//         if (table_id <= 0) {
//             return i + 1;
//         }
//     }
// }

// int getTableId(int num_people, int index) {
//     int res = 0;
//     for (int i = 0; i < num_people - 1; i++) {
//         res += count_tables[i];
//     }
//     return res + index;
// }

// int reserveVacantTable(int num_people) {
//     int index = num_people - 1;
//     int counter = 0;
//     for (int j = 0; j < index; j++) {
//         counter += count_tables[j];
//         // printf("\nstarting for size: %i: ", count_tables[j]);
//         // for (int k = 0; k < count_tables[j]; k++) {
//         //     printf("%i,", vacancy_tables[j][k]);
//         // }
//     }
//     for (int i = index; i < 5; i++) {
//         for (int j = 0; j < count_tables[i]; j++) {
//             if (vacancy_tables[i][j] == 1) {
//                 vacancy_tables[i][j] = 0;
//                 // printf("\nReserving %i, %i for id %i", i+1, j+1, counter+j);
//                 return counter + j;
//             }
//         }
//         counter += count_tables[i];
//     }
//     return -1;
// }

// void releaseTable(int table_id) {
//     // printf("original %i", table_id);
//     if (table_id < count_tables[0]) {
//         vacancy_tables[0][table_id] = 1;
//         // printf("\nReleasing 1, %i", table_id);
//         return; 
//     }

//     for (int i = 1; i < 5; i++) {
//         table_id -= count_tables[i - 1];
//         if (table_id < count_tables[i]) {
//             vacancy_tables[i][table_id] = 1;
//             // printf("\nReleasing %i, %i", i+1, table_id);
//             return;
//         }
//     }
// }

// void restaurant_init(int num_tables[5]) {
//     vacancy_tables[0] = vacant1;
//     vacancy_tables[1] = vacant2;
//     vacancy_tables[2] = vacant3;
//     vacancy_tables[3] = vacant3;
//     vacancy_tables[4] = vacant3;
//     if ( sem_init(&mutex, 0, 1) != 0 ) {
//         printf("Error initiating s\n");
//     }

//     for (int i = 0; i < 5; i++) {
//         count_tables[i] = num_tables[i];
//         vacancy_tables[i] = (int*) calloc(num_tables[i], sizeof(int));
//         queues[i] = createQueue();
//         for (int j = 0; j <num_tables[i]; j++) {
//             vacancy_tables[i][j] = 1;
//         }
//     }
// }

// void restaurant_destroy(void) {
//     for (int i = 4; i >= 0; i--) {
//         free(queues[i] -> array);
//         free(queues[i]);
//         free(vacancy_tables[i]);
//     }
//     pthread_mutex_destroy(&lock);
//     sem_destroy(&mutex);
// }

// int request_for_table(group_state *state, int num_people) {
//     int res;
//     // store info into state
//     state->group_size = num_people;
//     // retrieve mutex and conds of tablesize
//     int index = num_people - 1;
//     // pthread_mutex_t lock = mutex_tables[index];
//     pthread_mutex_lock(&lock);
//     if (isEmpty(queues[index])) {
//         res = reserveVacantTable(num_people);
//     } else {
//         res = -1;
//     }
        
//     on_enqueue();
//     if (res == -1) {
//         pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
//         pthread_cond_t* cond = &cond1;
//         sem_wait(&mutex);
//         enqueue(queues[index], cond, counter);
//         counter += 1;
//         sem_post(&mutex);
//         // printf("cond address a: %p \n", cond);
//         pthread_cond_wait(cond, &lock);
//         res = reserveVacantTable(num_people);
//         dequeue(queues[index]);
//         // printf("cond address c: %p \n", cond);
//     }
//     state->table_id = res;
//     pthread_mutex_unlock(&lock);
//     return res;
// }

// void leave_table(group_state *state) {
//     // int index = state->group_size - 1;
//     // pthread_mutex_t lock = mutex_tables[index];

//     pthread_mutex_lock(&lock);
//     releaseTable(state->table_id);

//     int index = getGroup(queues, getTableSize(state->table_id) + 1);
//     // printf("%i\n", index);
//     if (index != INT_MAX) {
//         pthread_cond_t* cond = peek(queues[index]);
//         if (cond != 0) {
//             pthread_cond_signal(cond);
//             pthread_cond_destroy(cond);
//         }
//     }

//     pthread_mutex_unlock(&lock);
// }