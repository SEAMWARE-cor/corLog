//
// FILE            corLogRing.c
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
// The last lines of the log, in memory - for a process that wants to show its own recent log (an admin API)
// without a log file. A byte ring: lines are written on, the oldest overwritten; a copy starts at the first
// whole line. Written under corLogMutex (corLogOut), copied under it too.
//
#include <pthread.h>                             // pthread_mutex_lock
#include <stdbool.h>                             // bool
#include <stdio.h>                               // snprintf
#include <stdlib.h>                              // malloc, free
#include <string.h>                              // memcpy, memchr
#include <sys/uio.h>                             // struct iovec
#include <time.h>                                // clock_gettime, gmtime_r

#include "corLog/corLogGlobals.h"                // corLogMutex
#include "corLog/corLogRing.h"                   // Own interface



static char*   ring    = NULL;
static size_t  size    = 0;
static size_t  head    = 0;                      // where the next byte goes
static bool    wrapped = false;



// -----------------------------------------------------------------------------
//
// put - bytes into the ring
//
static void put(const char* p, size_t n)
{
  if (n >= size)                                 // larger than the ring: its end only
  {
    p += n - (size - 1);
    n  = size - 1;
  }

  size_t first = size - head;
  if (n < first)
  {
    memcpy(&ring[head], p, n);
    head += n;
  }
  else
  {
    memcpy(&ring[head], p, first);
    memcpy(ring, p + first, n - first);
    head    = n - first;
    wrapped = true;
  }
}



// -----------------------------------------------------------------------------
//
// corLogRingInit -
//
bool corLogRingInit(size_t bytes)
{
  char* newRing = (bytes > 0) ? malloc(bytes) : NULL;
  if ((bytes > 0) && (newRing == NULL))
    return false;

  pthread_mutex_lock(&corLogMutex);
  char* old = ring;
  ring    = newRing;
  size    = bytes;
  head    = 0;
  wrapped = false;
  pthread_mutex_unlock(&corLogMutex);

  free(old);
  return true;
}



// -----------------------------------------------------------------------------
//
// corLogRingAdd -
//
void corLogRingAdd(const struct iovec* comp, int comps)
{
  if (ring == NULL)
    return;

  struct timespec now;
  struct tm       tm;
  char            stamp[32];

  clock_gettime(CLOCK_REALTIME, &now);
  gmtime_r(&now.tv_sec, &tm);
  size_t n = strftime(stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%S", &tm);
  n += snprintf(&stamp[n], sizeof(stamp) - n, ".%03dZ ", (int) (now.tv_nsec / 1000000));

  put(stamp, n);
  for (int ix = 0; ix < comps; ix++)
    put(comp[ix].iov_base, comp[ix].iov_len);
}



// -----------------------------------------------------------------------------
//
// corLogRingCopy -
//
char* corLogRingCopy(size_t* lenP)
{
  pthread_mutex_lock(&corLogMutex);

  if (ring == NULL)
  {
    pthread_mutex_unlock(&corLogMutex);
    return NULL;
  }

  size_t len = wrapped ? size : head;
  char*  out = malloc(len + 1);
  if (out == NULL)
  {
    pthread_mutex_unlock(&corLogMutex);
    return NULL;
  }

  if (wrapped)
  {
    memcpy(out, &ring[head], size - head);
    memcpy(&out[size - head], ring, head);
  }
  else
    memcpy(out, ring, head);
  pthread_mutex_unlock(&corLogMutex);

  out[len] = 0;

  //
  // Wrapped: the first line is cut - it starts after its newline
  //
  size_t from = 0;
  if (wrapped)
  {
    char* nl = memchr(out, '\n', len);
    from = (nl != NULL) ? (size_t) (nl - out) + 1 : len;
    memmove(out, &out[from], len - from + 1);
  }

  *lenP = len - from;
  return out;
}
