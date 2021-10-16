#include "packer.h"
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>

// You can declare global variables here
int N = 2;
sem_t s1;
sem_t s2;
sem_t s3;
sem_t s1_signal;
sem_t s2_signal;
sem_t s3_signal;
sem_t s1_count;
sem_t s2_count;
sem_t s3_count;

int* id_s1;
int* id_s2;
int* id_s3;

int count_1, count_2, count_3 = 0;

void packer_init(int balls_per_pack) {
    N = balls_per_pack;
    id_s1 = (int*) malloc (N * sizeof(int));
    id_s2 = (int*) malloc (N * sizeof(int));
    id_s3 = (int*) malloc (N * sizeof(int));
    sem_init(&s1, 0, 0);
    sem_init(&s2, 0, 0);
    sem_init(&s3, 0, 0);
    sem_init(&s1_signal, 0, 1);
    sem_init(&s2_signal, 0, 1);
    sem_init(&s3_signal, 0, 1);
    sem_init(&s1_count, 0, balls_per_pack);
    sem_init(&s2_count, 0, balls_per_pack);
    sem_init(&s3_count, 0, balls_per_pack);
}

void getOtherIds(int* arr, int id, int* addr) {
    int counter = 0;
    for (int j = 0; j < N; j++) {
        if (arr[j] != id) {
            addr[counter] = arr[j];
            counter++;
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
    sem_destroy(&s1_signal);
    sem_destroy(&s2_signal);
    sem_destroy(&s3_signal);
    free(id_s1);
    free(id_s2);
    free(id_s3);
}

void pack_ball(int colour, int id, int *other_ids) {
    // Write your code here.
    if (colour == 1) {
        return handleColourOne(id, other_ids);
    } else if (colour == 2) {
        return handleColourTwo(id, other_ids);
    } else if (colour == 3) {
        return handleColourThree(id, other_ids);
    } else {
        printf("Error, invalid colour!");
    }
}

int handleColourOne(int id, int *other_ids) {
    sem_wait(&s1_count);

    sem_wait(&s1_signal);
    id_s1[count_1] = id;
    count_1++;
    sem_post(&s1_signal);

    if (count_1 == N) {
        sem_post(&s1);
    }
    sem_wait(&s1);


    sem_wait(&s1_signal);
    count_1--;
    if (count_1 != 0) {
        sem_post(&s1);
    }
    getOtherIds(id_s1, id, other_ids);
    if (count_1 == 0) {
        for (int i = 0; i < N; i++) {
            sem_post(&s1_count);
        }
    }
    sem_post(&s1_signal);
}

int handleColourTwo(int id, int *other_ids) {
    int res;
    sem_wait(&s2_count);

    sem_wait(&s2_signal);
    id_s2[count_2] = id;
    count_2++;
    sem_post(&s2_signal);

    if (count_2 == N) {
        sem_post(&s2);
    }
    sem_wait(&s2);


    sem_wait(&s2_signal);
    count_2--;
    if (count_2 != 0) {
        sem_post(&s2);
    }
    getOtherIds(id_s2, id, other_ids);
    if (count_2 == 0) {
        for (int i = 0; i < N; i++) {
            sem_post(&s2_count);
        }
    }
    sem_post(&s2_signal);
}

int handleColourThree(int id, int *other_ids) {
    sem_wait(&s3_count);

    sem_wait(&s3_signal);
    id_s3[count_3] = id;
    count_3++;
    sem_post(&s3_signal);

    if (count_3 == N) {
        sem_post(&s3);
    }
    sem_wait(&s3);


    sem_wait(&s3_signal);
    count_3--;
    if (count_3 != 0) {
        sem_post(&s3);
    }
    getOtherIds(id_s3, id, other_ids);
    if (count_3 == 0) {
        for (int i = 0; i < N; i++) {
            sem_post(&s3_count);
        }
    }
    sem_post(&s3_signal);
}