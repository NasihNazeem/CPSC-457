/// ============================================================================
/// Copyright (C) 2022 Pavol Federl (pfederl@ucalgary.ca)
/// All Rights Reserved. Do not distribute this file.
/// ============================================================================
///
/// You must modify this file and then submit it for grading to D2L.
///
/// You can delete all contents of this file and start from scratch if
/// you wish, as long as you implement the detect_primes() function as
/// defined in "detectPrimes.h".

#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

std::atomic<bool> global_finished(false); // set to false and used atomic to be safe
std::atomic<bool> prime_number(false);

std::vector<int64_t> result; // set global result
int64_t num;
pthread_barrier_t barrier;
int numThreads;
std::vector<int64_t> numbers;
int64_t perThread;

// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
void is_prime(int64_t threadId)
{
  // handle trivial cases // cancels thread task
  if (num < 2)
  {
    prime_number = false;
    return;
  }
  if (num <= 3)
  {
    return;
  } // 2 and 3 are primes
  if (num % 2 == 0)
  {
    prime_number = false;
    return;
  } // handle multiples of 2
  if (num % 3 == 0)
  {
    prime_number = false;
    return;
  } // handle multiples of 3
  // try to divide n by every number 5 .. sqrt(n)
  int64_t start = 5 + (6 * threadId * perThread);
  int64_t finish = start + (6 * perThread);
  int64_t max = sqrt(num);

  for (int64_t i = start; i < finish; i += 6)
  {
    if (!prime_number)
    {
      pthread_cancel(pthread_self());
    }

    if (i <= max)
    {
      if (num % i == 0)
      {
        prime_number = false;
      }
      if (num % (i + 2) == 0)
      {
        prime_number = false;
      }
    }
    else
    {
      return;
    }
  }
  // didn't find any divisors, so it must be a prime
  return;
}

void *threadTask(void *taskId)
{
  int64_t tid = (intptr_t)taskId;

  while (1)
  {
    if (pthread_barrier_wait(&barrier) != 0)
    {
      if (numbers.empty())
      {
        global_finished = true;
      }
      else
      {
        num = numbers.back();
        numbers.pop_back();

        int div = (int)ceil((double)(sqrt(num) + 1.0) / 6.0);
        perThread = (int64_t)ceil((double)div / (double)numThreads);
      }
    }

    pthread_barrier_wait(&barrier);

    if (global_finished)
    {
      pthread_cancel(pthread_self());
      pthread_exit(NULL);
    }
    else
    {
      is_prime(tid);
    }

    if (pthread_barrier_wait(&barrier) != 0)
    {
      if (prime_number)
      {
        result.push_back(num);
      }
      prime_number = true;
    }
  }

  pthread_exit(NULL);
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
// You will most likely need to re-implement this function entirely.
// Note that the current implementation ignores n_threads. Your multithreaded
// implementation must use this parameter.
std::vector<int64_t>
detect_primes(const std::vector<int64_t> &nums, int n_threads)
{

  // store values globally to use in other functions
  numbers = nums;
  prime_number = true;
  numThreads = n_threads;
  pthread_t th[numThreads];

  // pthread barrier instead of barrier class because i am more familiar with pthread
  pthread_barrier_init(&barrier, NULL, numThreads);

  for (int i = 0; i < numThreads; i++)
  {
    pthread_create(&th[i], NULL, threadTask, (void *)(intptr_t)i);
  }

  for (int i = 0; i < numThreads; i++)
  {
    pthread_join(th[i], NULL);
  }

  return result;
}
