#include "restaurant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// You can declare global variables here

// // declaring mutex
pthread_mutex_t lock1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock4 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock5 = PTHREAD_MUTEX_INITIALIZER;

int *vacancy_tables[5];
int count_tables[5];
pthread_mutex_t mutex_tables[5];
struct Queue *queues[5];

// A structure to represent a queue
struct Queue
{
  unsigned capacity;
  unsigned head, tail, size;
  pthread_cond_t **array;
};

struct Queue *createQueue()
{
  struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
  queue->capacity = 100;
  queue->head = queue->size = 0;

  queue->tail = queue->capacity - 1;
  queue->array = (pthread_cond_t **)malloc(queue->capacity * sizeof(pthread_cond_t *));
  return queue;
}

void enqueue(struct Queue *queue, pthread_cond_t *item)
{
  if (queue->size == queue->capacity)
    return;
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->array[queue->tail] = item;
  queue->size = queue->size + 1;
  // printf("%d enqueued to queue\n", item);
}

pthread_cond_t *dequeue(struct Queue *queue)
{
  if (queue->size == 0)
    return NULL;
  pthread_cond_t *item = queue->array[queue->head];
  queue->head = (queue->head + 1) % queue->capacity;
  queue->size = queue->size - 1;
  return item;
}

int reserveTable(int num_people)
{
  int index = num_people - 1;
  int counter = 0;
  for (int j = 0; j < num_people - 1; j++)
  {
    counter += count_tables[j];
  }
  for (int i = 0; i < count_tables[index]; i++)
  {
    if (vacancy_tables[index][i] == 1)
    {
      vacancy_tables[index][i] = 0;
      return counter + i;
    }
  }
  return -1;
}

void unReserveTable(int table_id)
{
  if (table_id < count_tables[0])
  {
    vacancy_tables[0][table_id] = 1;
    return;
  }

  for (int i = 1; i < 5; i++)
  {
    table_id -= count_tables[i - 1];
    if (table_id < count_tables[i])
    {
      vacancy_tables[i][table_id] = 1;
      return;
    }
  }
}

pthread_cond_t *peek(struct Queue *queue)
{
  if (queue->size == 0)
    return NULL;
  pthread_cond_t *item = queue->array[queue->head];
  return item;
}

int isEmpty(struct Queue *queue)
{
  return (queue->size == 0);
}

void restaurant_init(int num_tables[5])
{
  for (int i = 0; i < 5; i++)
  {
    vacancy_tables[i] = 0;
    count_tables[i] = num_tables[i];
    vacancy_tables[i] = (int *)malloc(num_tables[i] * sizeof(int));
    queues[i] = createQueue();
    for (int j = 0; j < num_tables[i]; j++)
    {
      vacancy_tables[i][j] = 1;
    }
  }
  mutex_tables[0] = lock1;
  mutex_tables[1] = lock2;
  mutex_tables[2] = lock3;
  mutex_tables[3] = lock4;
  mutex_tables[4] = lock5;
}

void restaurant_destroy(void)
{
  for (int i = 4; i >= 0; i--)
  {
    free(queues[i]->array);
    free(queues[i]);
    free(vacancy_tables[i]);
  }

  pthread_mutex_destroy(&lock1);
  pthread_mutex_destroy(&lock2);
  pthread_mutex_destroy(&lock3);
  pthread_mutex_destroy(&lock4);
  pthread_mutex_destroy(&lock5);
}

int request_for_table(group_state *state, int num_people)
{
  int res;
  // store info into state
  state->group_size = num_people;
  // retrieve mutex and conds of tablesize
  int index = num_people - 1;
  pthread_mutex_t *lock = &mutex_tables[index];
  pthread_mutex_lock(lock);

  if (isEmpty(queues[index]))
  {
    res = reserveTable(num_people);
  }
  else
  {
    res = -1;
  }

  on_enqueue();
  if (res == -1)
  {
    pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
    pthread_cond_t *cond = &condition;
    enqueue(queues[index], cond);
    pthread_cond_wait(cond, lock);
    res = reserveTable(num_people);
    dequeue(queues[index]);
    pthread_cond_destroy(&condition);
  }
  state->table_id = res;
  pthread_mutex_unlock(lock);
  return res;
}

void leave_table(group_state *state)
{
  int index = state->group_size - 1;
  pthread_mutex_t *lock = &mutex_tables[index];
  pthread_mutex_lock(lock);

  pthread_cond_t *cond = peek(queues[index]);
  unReserveTable(state->table_id);

  if (cond != 0)
  {
    pthread_cond_signal(cond);
  }
  pthread_mutex_unlock(lock);
}
