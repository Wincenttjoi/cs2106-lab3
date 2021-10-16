#include "restaurant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// A structure to represent a queue
struct Queue {
    unsigned front, rear, size;
    unsigned capacity;
    pthread_cond_t** array;
};

struct Queue* createQueue()
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = 1000;
    queue->front = queue->size = 0;
 
    queue->rear = queue->capacity - 1;
    queue->array = (pthread_cond_t**)malloc(
        queue->capacity * sizeof(pthread_cond_t*));
    return queue;
}
 
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}

void enqueue(struct Queue* queue, pthread_cond_t* item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    // printf("%d enqueued to queue\n", item);
}
 
pthread_cond_t* dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return NULL;
    pthread_cond_t* item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}

// Function to remove an item from queue.
// It changes front and size
pthread_cond_t* peek(struct Queue* queue)
{
    if (isEmpty(queue))
        return NULL;
    pthread_cond_t* item = queue->array[queue->front];
    return item;
}



// You can declare global variables here

// // Declaration of thread condition variable
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond4 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond5 = PTHREAD_COND_INITIALIZER;

// // declaring mutex
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock5 = PTHREAD_MUTEX_INITIALIZER;

int *vacant1, *vacant2,*vacant3,*vacant4,*vacant5 = 0;
int* vacancy_tables[5];
struct Queue* queues[5];
int count_tables[5];
pthread_mutex_t mutex_tables[5];


int reserveVacantTable(int num_people) {
    int index = num_people - 1;
    int counter = 0;
    for (int j = 0; j < num_people - 1; j++) {
        counter += count_tables[j];
    }
    for (int i = 0; i < count_tables[i]; i++) {
        if (vacancy_tables[index][i] == 1) {
            vacancy_tables[index][i] = 0;
            return counter + i;
        }
    }
    return -1;
}

void releaseTable(int table_id) {
    if (table_id < count_tables[0]) {
        vacancy_tables[0][table_id] = 1;
        return; 
    }

    for (int i = 1; i < 5; i++) {
        table_id -= count_tables[i - 1];
        if (table_id < count_tables[i]) {
            vacancy_tables[i][table_id] = 1;
            return;
        }
    }
}

void restaurant_init(int num_tables[5]) {
    // Write initialization code here (called once at the start of the program).
    // It is guaranteed that num_tables is an array of length 5.
    // TODO
    vacancy_tables[0] = vacant1;
    vacancy_tables[1] = vacant2;
    vacancy_tables[2] = vacant3;
    vacancy_tables[3] = vacant3;
    vacancy_tables[4] = vacant3;

    mutex_tables[0] = lock1;
    mutex_tables[1] = lock2;
    mutex_tables[2] = lock3;
    mutex_tables[3] = lock4;
    mutex_tables[4] = lock5;


    for (int i = 0; i < 5; i++) {
        count_tables[i] = num_tables[i];
        vacancy_tables[i] = (int*) calloc(num_tables[i], sizeof(int));
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
    pthread_cond_destroy(&cond2);
    pthread_cond_destroy(&cond3);
    pthread_cond_destroy(&cond4);
    pthread_cond_destroy(&cond5);

    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    pthread_mutex_destroy(&lock3);
    pthread_mutex_destroy(&lock4);
    pthread_mutex_destroy(&lock5);
}

int request_for_table(group_state *state, int num_people) {
    int res;
    // store info into state
    state->group_size = num_people;
    // retrieve mutex and conds of tablesize
    int index = num_people - 1;
    pthread_mutex_t lock = mutex_tables[index];
    pthread_mutex_lock(&lock);
    res = reserveVacantTable(num_people);
        
    on_enqueue();
    if (res == -1) {
        pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
        pthread_cond_t* cond = &cond1;
        enqueue(queues[index], cond);
        pthread_cond_wait(cond, &lock);
        res = reserveVacantTable(num_people);
    }
    state->table_id = res;
    pthread_mutex_unlock(&lock);
    return res;
}

void leave_table(group_state *state) {
    int index = state->group_size - 1;
    pthread_mutex_t lock = mutex_tables[index];
    pthread_mutex_lock(&lock);

    pthread_cond_t* cond = dequeue(queues[index]);
    releaseTable(state->table_id);

    if (cond != 0) {
        pthread_cond_signal(cond);
    }
    pthread_mutex_unlock(&lock);
}