#include "packer.h"
#include <stdio.h>
#include <semaphore.h>

// You can declare global variables here
int N = 2;
sem_t collector1;
sem_t collector2;
sem_t collector3;
// sem_t waitForFlush1;
// sem_t waitForFlush2;
// sem_t waitForFlush3;

int id1[2];
int id2[2];
int id3[2];
int counter1 = 0;
int counter2 = 0;
int counter3 = 0;

int getOther(int* arr, int my_id) {
    for (int i = 0; i < N; i++) {
        if (arr[i] != my_id) {
            return arr[i];
        }
    }
}

void packer_init(void) {
    // Write initialization code here (called once at the start of the program).
    sem_init(&collector1, 0, 1);
    sem_init(&collector2, 0, 1);
    sem_init(&collector3, 0, 1);
    // sem_init(&waitForFlush1, 0, 0);
    // sem_init(&waitForFlush2, 0, 0);
    // sem_init(&waitForFlush3, 0, 0);
}

void packer_destroy(void) {
    // Write deinitialization code here (called once at the end of the program).
    sem_destroy(&collector1);
    sem_destroy(&collector2);
    sem_destroy(&collector3);
    // sem_destroy(&waitForFlush1);
    // sem_destroy(&waitForFlush2);
    // sem_destroy(&waitForFlush3);
}

int pack_ball(int colour, int id) {
    // Write your code here.
    int otherBallId;

    if (colour == 1) {
        if (counter1 + 1 > N) {
            printf("Error");
        }
        sem_wait(&collector1);
        id1[counter1] = id;
        counter1++;
        sem_post(&collector1);
        if (counter1 < N) {

        }
        otherBallId = getOther(id1, id);
        counter1 = 0;
    }

    if (colour == 2) {
        if (counter2 + 1 > N) {
            printf("Error");
        }
        sem_wait(&collector2);
        id1[counter2] = id;
        counter2++;
        sem_post(&collector2);
        if (counter2 < N) {
        }
            otherBallId = getOther(id2, id);
            counter2 = 0;
    }

    if (colour == 3) {
        if (counter3 + 1 > N) {
            printf("Error");
        }
        sem_wait(&collector3);
        id1[counter3] = id;
        counter3++;
        sem_post(&collector3);
        if (counter3 < N) {
        }
            otherBallId = getOther(id3, id);
            counter3 = 0;
    }
    // printf("Ball %d was matched with ball %d\n", id, otherBallId);

    return otherBallId;
}

