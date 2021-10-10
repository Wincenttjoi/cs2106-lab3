#include "restaurant.h"
#include <pthread.h>
#include <semaphore.h>

// You can declare global variables here
int tables_for_size[6];
int *table_one_occupancy;
int *table_two_occupancy;
int *table_three_occupancy;
int *table_four_occupancy;
int *table_five_occupancy;

pthread_mutex_t lock_one;
pthread_mutex_t lock_two;
pthread_mutex_t lock_three;
pthread_mutex_t lock_four;
pthread_mutex_t lock_five;

sem_t table_one;
sem_t table_two;
sem_t table_three;
sem_t table_four;
sem_t table_five;

int *queue;
int queue_counter;

int next_ticket = 0;
int ticket_counter = 0;

void restaurant_init(int num_tables[5]) {
    // Write initialization code here (called once at the start of the program).
    // It is guaranteed that num_tables is an array of length 5.
    // TODO
    // 0 2 3 1 4 2: Indexes follow the table size
    tables_for_size[0] = 0;
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        tables_for_size[i + 1] = num_tables[i];
    }
    // 1 1
    table_one_occupancy = (int*) malloc(tables_for_size[1] * sizeof(int));
    table_two_occupancy = (int*) malloc(tables_for_size[2] * sizeof(int));
    table_three_occupancy = (int*) malloc(tables_for_size[3] * sizeof(int));
    table_four_occupancy = (int*) malloc(tables_for_size[4] * sizeof(int));
    table_five_occupancy = (int*) malloc(tables_for_size[5] * sizeof(int));

    pthread_mutex_init(&lock_one, NULL);
    pthread_mutex_init(&lock_two, NULL);
    pthread_mutex_init(&lock_three, NULL);
    pthread_mutex_init(&lock_four, NULL);
    pthread_mutex_init(&lock_five, NULL);

    sem_init(&table_one, 0, tables_for_size[1]);
    sem_init(&table_two, 0, tables_for_size[2]);
    sem_init(&table_three, 0, tables_for_size[3]);
    sem_init(&table_four, 0, tables_for_size[4]);
    sem_init(&table_five, 0, tables_for_size[5]);

    queue = (group_state*) malloc (sizeof(group_state*));

}

void restaurant_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    // TODO
    free(table_one_occupancy);
    free(table_two_occupancy);
    free(table_three_occupancy);
    free(table_four_occupancy);
    free(table_five_occupancy);

    pthread_mutex_destroy(&lock_one);
    pthread_mutex_destroy(&lock_two);
    pthread_mutex_destroy(&lock_three);
    pthread_mutex_destroy(&lock_four);
    pthread_mutex_destroy(&lock_five);

    sem_destroy(&table_one);
    sem_destroy(&table_two);
    sem_destroy(&table_three);
    sem_destroy(&table_four);
    sem_destroy(&table_five);

    free(queue);
}

int request_for_table(group_state *state, int num_people) {
    // Write your code here.
    // Return the id of the table you want this group to sit at.
    // TODO
    on_enqueue();
    waitMutexForTable(num_people);
    int tableChoped = chopeTableOccupancy(&state, num_people);
    return tableChoped;
}

// 1 1

void leave_table(group_state *state) {
    // Write your code here.
    // TODO
    vacate_table(state, state->num_people);
    postMutexForTable(state->num_people);
}

void waitMutexForTable(int num_people) {
    if (num_people == 1) {
        sem_wait(&table_one);
    } else if (num_people == 2) {
        sem_wait(&table_two);
    } else if (num_people == 3) {
        sem_wait(&table_three);
    } else if (num_people == 4) {
        sem_wait(&table_four);
    } else if (num_people == 5) {
        sem_wait(&table_five);
    }
}

void postMutexForTable(int num_people) {
    if (num_people == 1) {
        sem_post(&table_one);
    } else if (num_people == 2) {
        sem_post(&table_two);
    } else if (num_people == 3) {
        sem_post(&table_three);
    } else if (num_people == 4) {
        sem_post(&table_four);
    } else if (num_people == 5) {
        sem_post(&table_five);
    }
}

void vacate_table(group_state* state, int num_people) {
    if (num_people == 1) {
        table_one_occupancy[state->table_number] = 0;
    } else if (num_people == 2) {
        table_two_occupancy[state->table_number] = 0;
    } else if (num_people == 3) {
        table_three_occupancy[state->table_number] = 0;
    } else if (num_people == 4) {
        table_four_occupancy[state->table_number] = 0;
    } else if (num_people == 5) {
        table_five_occupancy[state->table_number] = 0;
    }
    // checkQueue(state->num_people);
}

// void checkQueue(int num_people) {
//     for (int i = 0; i < queue_counter - 1; i++) {
//         group_state* state = queue[i];
//         if (state->num_people == num_people) {
//             request_for_table(state, state->num_people);
//         }
//     }
// }

int chopeTableOccupancy(group_state* state, int num_people) {
    if (num_people == 1) {
        for (int i = 0; i < tables_for_size[1]; i++) {
            if (table_one_occupancy[i] == 0) {
                table_one_occupancy[i] = 1;
                state->table_number = i;
                return i;
            }
        }
    } else if (num_people == 2) {
        for (int i = 0; i < tables_for_size[2]; i++) {
            if (table_two_occupancy[i] == 0) {
                table_two_occupancy[i] = 1;
                state->table_number = i;
                return tables_for_size[1] + i;
            }
        }
    } else if (num_people == 3) {
        for (int i = 0; i < tables_for_size[3]; i++) {
            if (table_three_occupancy[i] == 0) {
                table_three_occupancy[i] = 1;
                state->table_number = i;
                return tables_for_size[1] + tables_for_size[2] + i;
            }
        }
    } else if (num_people == 4) {
        for (int i = 0; i < tables_for_size[4]; i++) {
            if (table_four_occupancy[i] == 0) {
                table_four_occupancy[i] = 1;
                state->table_number = i;
                return tables_for_size[1] + tables_for_size[2] + tables_for_size[3] + i;
            }
        }
    } else if (num_people == 5) {
        for (int i = 0; i < tables_for_size[5]; i++) {
            if (table_five_occupancy[i] == 0) {
                table_five_occupancy[i] = 1;
                state->table_number = i;
                return tables_for_size[1] + tables_for_size[2] + tables_for_size[3] + tables_for_size[4] + i;
            }
        }
    }

    // Needs to queue
    state->num_people = num_people;
    state->queue_counter = queue_counter;
    queue[queue_counter] = state;
    queue_counter++;
}

