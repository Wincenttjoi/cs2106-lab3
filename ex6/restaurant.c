#include "restaurant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>

// // declaring mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
sem_t mutex;

int *vacancy_tables[5];
struct Queue *queues[5];
int count_tables[5];
int counter = 0;

// A structure to represent a queue
struct Queue
{
  unsigned capacity;
  unsigned head, tail, size;
  pthread_cond_t **array;
  int *array_temp;
};

struct Queue *createQueue()
{
  struct Queue *queue = (struct Queue *)malloc(sizeof(struct Queue));
  queue->capacity = 100;
  queue->head = queue->size = 0;

  // This is important, see the enqueue
  queue->tail = queue->capacity - 1;
  queue->array = (pthread_cond_t **)malloc(queue->capacity * sizeof(pthread_cond_t *));
  queue->array_temp = (int *)malloc(queue->capacity * sizeof(int));
  return queue;
}

void enqueue(struct Queue *queue, pthread_cond_t *item, int i)
{
  if (queue->size == queue->capacity)
    return;
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->array[queue->tail] = item;
  queue->array_temp[queue->tail] = i;
  queue->size = queue->size + 1;
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

pthread_cond_t *peek(struct Queue *queue)
{
  if (queue->size == 0)
    return NULL;
  pthread_cond_t *item = queue->array[queue->head];
  return item;
}

int peekIndex(struct Queue *queue, int num)
{
  if (queue->size == 0)
    return INT_MAX;
  int index = queue->array_temp[queue->head + num];
  return index;
}

int reserveTable(int num_people)
{
  int index = num_people - 1;
  int counter = 0;
  int ctr = 0;
  for (int j = 0; j < index; j++)
  {
    counter += count_tables[j];
  }

  ctr = counter;
  for (int i = index; i < 5; i++)
  {
    for (int j = 0; j < count_tables[i]; j++)
    {
      if ((vacancy_tables[i][j] == i + 1) && ((vacancy_tables[i][j] - num_people) >= 0))
      {
        vacancy_tables[i][j] -= num_people;
        return counter + j;
      }
    }
    counter += count_tables[i];
  }

  counter = ctr;
  int minimum = INT_MAX;
  int temp_i = 0;
  int temp_j = 0;
  int temp_counter = 0;

  for (int i = index; i < 5; i++)
  {
    for (int j = 0; j < count_tables[i]; j++)
    {
      if ((vacancy_tables[i][j] - num_people >= 0) && (vacancy_tables[i][j] < minimum))
      {
        minimum = vacancy_tables[i][j];
        temp_i = i;
        temp_j = j;
        temp_counter = counter + j;
      }
    }
    counter += count_tables[i];
  }

  if (minimum == INT_MAX)
  {
    return -1;
  }
  vacancy_tables[temp_i][temp_j] -= num_people;
  return temp_counter;
}

int unReserveTable(int table_id, int total_people)
{
  if (table_id < count_tables[0])
  {
    vacancy_tables[0][table_id] += total_people;
    return vacancy_tables[0][table_id];
  }

  int i = 1;
  while (i < 5)
  {
    table_id -= count_tables[i - 1];
    if (table_id < count_tables[i])
    {
      vacancy_tables[i][table_id] += total_people;
      return vacancy_tables[i][table_id];
    }
    i++;
  }
}

int getTableSize(int table_id)
{
  int i = 0;
  while (i < 5)
  {
    table_id -= count_tables[i];
    if (table_id <= 0)
    {
      return i + 1;
    }
    i++;
  }
}

int getTableId(int num_people, int index)
{
  int res = 0;
  int i = 0;
  while (i < num_people - 1)
  {
    res += count_tables[i];
    i++;
  }
  return res + index;
}

int getGroup(struct Queue **arr, int groupSize)
{
  int curr = INT_MAX;
  int result = INT_MAX;
  int i = 0;
  int temp = 0;
  while (i < groupSize)
  {
    int k = 0;
    int index = peekIndex(arr[i], k);
    while (k < arr[i]->size && index < 0)
    {
      k++;
      index = peekIndex(arr[i], k);
    }
    if (index < curr)
    {
      curr = index;
      result = i;
      temp = k;
    }
    i++;
  }
  if (result != INT_MAX)
  {
    struct Queue *queue = queues[result];
    queue->array_temp[temp + queue->head] = -1;
  }
  return result;
}

void restaurant_init(int num_tables[5])
{
  sem_init(&mutex, 0, 1);
  for (int i = 0; i < 5; i++)
  {
    count_tables[i] = num_tables[i];
    vacancy_tables[i] = (int *)malloc(num_tables[i] * sizeof(int));
    queues[i] = createQueue();
    for (int j = 0; j < num_tables[i]; j++)
    {
      vacancy_tables[i][j] = i + 1;
    }
  }
}

void restaurant_destroy(void)
{
  for (int i = 4; i >= 0; i--)
  {
    free(queues[i]->array);
    free(queues[i]->array_temp);
    free(queues[i]);
    free(vacancy_tables[i]);
  }
  pthread_mutex_destroy(&lock);
  sem_destroy(&mutex);
}

int request_for_table(group_state *state, int num_people)
{
  int res;
  state->group_size = num_people;
  int index = num_people - 1;
  pthread_mutex_lock(&lock);
  if (queues[index]->size == 0)
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
    pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
    pthread_cond_t *cond = &cond1;
    sem_wait(&mutex);
    enqueue(queues[index], cond, counter);
    counter += 1;
    sem_post(&mutex);
    pthread_cond_wait(cond, &lock);
    res = reserveTable(num_people);
    dequeue(queues[index]);
    pthread_cond_destroy(&cond1);
  }
  state->table_id = res;
  pthread_mutex_unlock(&lock);
  return res;
}

void leave_table(group_state *state)
{

  pthread_mutex_lock(&lock);
  int vacancyLeft = unReserveTable(state->table_id, state->group_size);

  while (vacancyLeft > 0)
  {
    int index = getGroup(queues, vacancyLeft);
    if (index == INT_MAX)
    {
      break;
    }
    pthread_cond_t *cond = peek(queues[index]);
    if (cond != 0)
    {
      pthread_cond_signal(cond);
    }
    vacancyLeft = vacancyLeft - index - 1;
  }

  pthread_mutex_unlock(&lock);
}
