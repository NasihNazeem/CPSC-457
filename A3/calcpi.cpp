// ======================================================================
// You must modify this file and then submit it for grading to D2L.
// ======================================================================
//
// count_pi() calculates the number of pixels that fall into a circle
// using the algorithm explained here:
//
// https://en.wikipedia.org/wiki/Approximations_of_%CF%80
//
// count_pixels() takes 2 paramters:
//  r         =  the radius of the circle
//  n_threads =  the number of threads you should create
//
// Currently the function ignores the n_threads parameter. Your job is to
// parallelize the function so that it uses n_threads threads to do
// the computation.

#include "calcpi.h"
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// GOAL: Have threads SHARE the amount of summation while incrementing synchornously.
void *calculate(void *arg);

struct Summation
{
  int threadId;
  uint64_t partial_count;
};

// 256 threads
Summation sum[256];

int radius;
int n_thread;
double rsq;

uint64_t
count_pixels(int r, int n_threads)
{
  if (r < n_threads)
    n_threads = r;
  pthread_t th[n_threads];
  radius = r;
  rsq = double(r) * r;
  n_thread = n_threads;
  uint64_t totalSum = 0;

  // Create n amount of threads to do the summation task.
  for (int i = 0; i < n_threads; i++)
  {
    sum[i].threadId = i + 1;
    if (pthread_create(&th[i], NULL, &calculate, &sum[i]) != 0)
    {
      perror("Failed to create thread");
      exit(-1);
    }
  }

  // Join all threads.
  for (int i = 0; i < n_threads; i++)
  {
    if (pthread_join(th[i], NULL) != 0)
    {
      perror("Failed to join thread");
      exit(-1);
    }
  }

  // Add all thread summation together
  for (int i = 0; i < n_threads; i++)
  {
    totalSum += sum[i].partial_count;
  }

  return totalSum * 4 + 1;
}

void *calculate(void *arg)
{

  struct Summation *summation = (struct Summation *)arg;
  uint64_t count = 0;

  /* This for loop iterates over the threads and increments by the # of threads in use.
   *
   */
  for (double x = summation->threadId; x <= radius; x += n_thread)
  {
    for (double y = 0; y <= radius; y++)
    {
      if (x * x + y * y <= rsq)
        count++;
    }
  }

  summation->partial_count = count; // Summation from one task is now shared to the global struct.
  return NULL;
}
