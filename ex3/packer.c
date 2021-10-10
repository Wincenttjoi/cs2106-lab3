#include "packer.h"
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>

// You can declare global variables here
int N = 2;
sem_t s1;
sem_t s2;
sem_t s3;
sem_t s1_mutex;
sem_t s2_mutex;
sem_t s3_mutex;
sem_t s1_count;
sem_t s2_count;
sem_t s3_count;

int* id_s1;
int* id_s2;
int* id_s3;

int count_1, count_2, count_3 = 0;

void packer_init(int balls_per_pack) {
    N = balls_per_pack;
    id_s1 = (int*) calloc (N,sizeof(int));
    id_s2 = (int*) calloc (N,sizeof(int));
    id_s3 = (int*) calloc (N,sizeof(int));
    // Write initialization code here (called once at the start of the program).
    if ( sem_init(&s1, 0, 0) != 0 ) {
        printf("Error initiating s1\n");
    }
    if ( sem_init(&s2, 0, 0) != 0 ) {
        printf("Error initiating s2\n");
    }
    if ( sem_init(&s3, 0, 0) != 0 ) {
        printf("Error initiating s3\n");
    }
    if ( sem_init(&s1_mutex, 0, 1) != 0 ) {
        printf("Error initiating s1\n");
    }
    if ( sem_init(&s2_mutex, 0, 1) != 0 ) {
        printf("Error initiating s2\n");
    }
    if ( sem_init(&s3_mutex, 0, 1) != 0 ) {
        printf("Error initiating s3\n");
    }
    if ( sem_init(&s1_count, 0, balls_per_pack) != 0 ) {
        printf("Error initiating s1\n");
    }
    if ( sem_init(&s2_count, 0, balls_per_pack) != 0 ) {
        printf("Error initiating s2\n");
    }
    if ( sem_init(&s3_count, 0, balls_per_pack) != 0 ) {
        printf("Error initiating s3\n");
    }
}

void getOthers(int* arr, int id, int* addr) {
    int k = 0;
    for (int j = 0; j < N; j++) {
        if (arr[j] != id) {
            addr[k] = arr[j];
            k++;
        }
    }
}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    sem_destroy(&s1_count);
    sem_destroy(&s2_count);
    sem_destroy(&s3_count);
    sem_destroy(&s1);
    sem_destroy(&s2);
    sem_destroy(&s3);
    sem_destroy(&s1_mutex);
    sem_destroy(&s2_mutex);
    sem_destroy(&s3_mutex);
    free(id_s1);
    free(id_s2);
    free(id_s3);
}

void pack_ball(int colour, int id, int *other_ids) {
    // Write your code here.
    switch(colour) {
        case 1: ;
        sem_wait(&s1_count);
        sem_wait(&s1_mutex);
        id_s1[count_1] = id;
        count_1++;
        sem_post(&s1_mutex);

        if (count_1 == N) {
            sem_post(&s1);
        }
        sem_wait(&s1);
        sem_wait(&s1_mutex);
        count_1--;
        if (count_1 != 0) {
            sem_post(&s1);
        }
        getOthers(id_s1, id, other_ids);
        if (count_1 == 0) {
            for (int i = 0; i < N; i++) {
                sem_post(&s1_count);
            }
        }
        sem_post(&s1_mutex);
        break;

        case 2: ;
        sem_wait(&s2_count);
        sem_wait(&s2_mutex);
        id_s2[count_2] = id;
        count_2++;
        sem_post(&s2_mutex);

        if (count_2 == N) {
            sem_post(&s2);
        }
        sem_wait(&s2);
        sem_wait(&s2_mutex);
        count_2--;
        if (count_2 != 0) {
            sem_post(&s2);
        }
        getOthers(id_s1, id, other_ids);
        if (count_2 == 0) {
            for (int i = 0; i < N; i++) {
                sem_post(&s2_count);
            }
        }
        sem_post(&s2_mutex);
        break;

        case 3: ;
        sem_wait(&s3_count);

        sem_wait(&s3_mutex);
        id_s3[count_3] = id;
        count_3++;
        sem_post(&s3_mutex);

        if (count_3 == N) {
            sem_post(&s3);
        }
        sem_wait(&s3);

        sem_wait(&s3_mutex);
        count_3--;
        if (count_3 != 0) {
            sem_post(&s3);
        }
        getOthers(id_s1, id, other_ids);
        if (count_3 == 0) {
            for (int i = 0; i < N; i++) {
                sem_post(&s3_count);
            }
        }
        sem_post(&s3_mutex);
        break;

        default:
            printf("Unidentified colour");
    }
}