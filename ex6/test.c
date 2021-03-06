#include "restaurant.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <semaphore.h>

// // declaring mutex
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int *vacant1, *vacant2, *vacant3, *vacant4, *vacant5 = 0;
int *vacancy_tables[5];
struct Queue *queues[5];
int count_tables[5];
// pthread_mutex_t mutex_tables[5];
int counter = 0;
sem_t mutex;

// A structure to represent a queue
struct Queue
{
  unsigned front, rear, size;
  unsigned capacity;
  pthread_cond_t **array;
  int *array2;
};

struct Queue *createQueue()
{
  struct Queue *queue = (struct Queue *)malloc(
      sizeof(struct Queue));
  queue->capacity = 1200;
  queue->front = queue->size = 0;

  // This is important, see the enqueue
  queue->rear = queue->capacity - 1;
  queue->array = (pthread_cond_t **)malloc(
      queue->capacity * sizeof(pthread_cond_t *));
  queue->array2 = (int *)malloc(
      queue->capacity * sizeof(int));
  return queue;
}

int isFull(struct Queue *queue)
{
  return (queue->size == queue->capacity);
}

int isEmpty(struct Queue *queue)
{
  return (queue->size == 0);
}

void enqueue(struct Queue *queue, pthread_cond_t *item, int i)
{
  if (isFull(queue))
    return;
  queue->rear = (queue->rear + 1) % queue->capacity;
  queue->array[queue->rear] = item;
  queue->array2[queue->rear] = i;
  queue->size = queue->size + 1;
}

pthread_cond_t *dequeue(struct Queue *queue)
{
  if (isEmpty(queue))
    return NULL;
  pthread_cond_t *item = queue->array[queue->front];
  queue->front = (queue->front + 1) % queue->capacity;
  queue->size = queue->size - 1;
  return item;
}

pthread_cond_t *peek(struct Queue *queue)
{
  if (isEmpty(queue))
    return NULL;
  pthread_cond_t *item = queue->array[queue->front];
  return item;
}

int peekIndex(struct Queue *queue, int i)
{
  if (isEmpty(queue))
    return INT_MAX;
  int temp = i + queue->front;
  int index = queue->array2[temp];
  return index;
}

int getGroup(struct Queue **arr, int groupSize)
{
  int currFront = INT_MAX;
  int res = INT_MAX;
  int temp = 0;
  // printf("size: %i\n", groupSize);
  for (int i = 0; i < groupSize; i++)
  {
    // printf("size: %i , index %i at queue %i \n", groupSize, peekIndex(arr[i]), i);
    int j = 0;
    int index = peekIndex(arr[i], j);
    while (index < 0 && j < arr[i]->size)
    {
      j = j + 1;
      // printf("gahh");
      index = peekIndex(arr[i], j);
    }
    // printf("i: %i, index: %i", i, index);
    if (index < currFront)
    {
      currFront = index;
      res = i;
      temp = j;
    }
    // printf("currFront %i \n", currFront);
  }
  if (res != INT_MAX)
  {
    struct Queue *queue = queues[res];
    queue->array2[temp + queue->front] = -1;
  }
  // printf("res: %i\n", res);
  return res;
}

void restaurant_init(int num_tables[5])
{
  vacancy_tables[0] = vacant1;
  vacancy_tables[1] = vacant2;
  vacancy_tables[2] = vacant3;
  vacancy_tables[3] = vacant3;
  vacancy_tables[4] = vacant3;

  if (sem_init(&mutex, 0, 1) != 0)
  {
    printf("Error initiating s\n");
  }
  for (int i = 0; i < 5; i++)
  {
    count_tables[i] = num_tables[i];
    vacancy_tables[i] = (int *)calloc(num_tables[i], sizeof(int));
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
    free(queues[i]);
    free(vacancy_tables[i]);
  }
  sem_destroy(&mutex);
  pthread_mutex_destroy(&lock);
}

int getTableSize(int table_id)
{
  for (int i = 0; i < 5; i++)
  {
    table_id -= count_tables[i];
    if (table_id <= 0)
    {
      return i + 1;
    }
  }
}

int getTableId(int num_people, int index)
{
  int res = 0;
  for (int i = 0; i < num_people - 1; i++)
  {
    res += count_tables[i];
  }
  return res + index;
}

int reserveVacantTable(int num_people)
{
  int index = num_people - 1;
  int counter_init, counter = 0;
  for (int j = 0; j < index; j++)
  {
    counter += count_tables[j];
    // printf("\nstarting for size: %i: ", count_tables[j]);
    // for (int k = 0; k < count_tables[j]; k++) {
    //     printf("%i,", vacancy_tables[j][k]);
    // }
  }
  counter_init = counter;
  for (int i = index; i < 5; i++)
  {
    for (int j = 0; j < count_tables[i]; j++)
    {
      // printf("\na Considering %i, %i for id %i. Vacancy: %i\n", i+1, j+1, counter+j, vacancy_tables[i][j]);
      if ((vacancy_tables[i][j] - num_people) >= 0 &&
          vacancy_tables[i][j] == i + 1)
      {
        vacancy_tables[i][j] -= num_people;
        return counter + j;
      }
    }
    counter += count_tables[i];
  }
  counter = counter_init;
  int min = INT_MAX;
  int i_val, j_val, counter_temp = 0;
  // printf("\nmin: %i\n", min);

  for (int i = index; i < 5; i++)
  {
    for (int j = 0; j < count_tables[i]; j++)
    {
      // printf("\nb Considering %i, %i for id %i. Vacancy: %i\n", i+1, j+1, counter+j, vacancy_tables[i][j]);
      if (vacancy_tables[i][j] - num_people >= 0)
      {
        // printf("\nc Considering %i, %i for id %i. Vacancy: %i min: %i\n", i+1, j+1, counter+j, vacancy_tables[i][j], min);
        if (vacancy_tables[i][j] < min)
        {
          // printf("counter_temp: %i", counter_temp);
          min = vacancy_tables[i][j];
          i_val = i;
          j_val = j;
          counter_temp = counter + j;
        }
      }
    }
    counter += count_tables[i];
  }
  if (min != INT_MAX)
  {
    vacancy_tables[i_val][j_val] -= num_people;
    return counter_temp;
  }
  return -1;
}

int releaseTable(int table_id, int num_people)
{
  int temp = table_id;
  if (temp < count_tables[0])
  {
    vacancy_tables[0][temp] += num_people;
    // printf("\nReleasing 1, %i", table_id);
    return vacancy_tables[0][temp];
  }

  for (int i = 1; i < 5; i++)
  {
    temp -= count_tables[i - 1];
    if (temp < count_tables[i])
    {
      vacancy_tables[i][temp] += num_people;
      // printf("\nReleasing %i seats at table %i, now: %i", num_people, table_id, vacancy_tables[i][temp]);
      return vacancy_tables[i][temp];
    }
  }
}

int request_for_table(group_state *state, int num_people)
{
  int res;
  // store info into state
  state->group_size = num_people;
  // retrieve mutex and conds of tablesize
  int index = num_people - 1;
  // pthread_mutex_t lock = mutex_tables[index];
  pthread_mutex_lock(&lock);
  if (isEmpty(queues[index]))
  {
    res = reserveVacantTable(num_people);
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
    // printf("\nlemme see\n");
    res = reserveVacantTable(num_people);
    dequeue(queues[index]);
  }
  state->table_id = res;
  pthread_mutex_unlock(&lock);
  return res;
}

void leave_table(group_state *state)
{
  pthread_mutex_lock(&lock);
  int newVacancy = releaseTable(state->table_id, state->group_size);

  while (newVacancy > 0)
  {
    int index = getGroup(queues, newVacancy);
    // printf("newvacancy: %i\n", newVacancy);
    if (index == INT_MAX)
    {
      break;
    }
    pthread_cond_t *cond = peek(queues[index]);
    if (cond != 0)
    {
      pthread_cond_signal(cond);
      pthread_cond_destroy(cond);
    }
    newVacancy = newVacancy - index - 1;
  }
  pthread_mutex_unlock(&lock);
}