#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
// FILE            corLogRingTest.c - the ring (corLogRingInit, corLogRingCopy): off, one line, wrapped, off again
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
#include "corLog/corLog.h"
#include "corLog/corLogRing.h"
int main(void)
{
  corLogInit("t", "/tmp", false, NULL, NULL, false, false);
  size_t len;
  if (corLogRingCopy(&len) != NULL) { printf("FAIL: on before init\n"); return 1; }
  corLogRingInit(400);
  COR_W("first %d", 1);
  char* t = corLogRingCopy(&len);
  printf("--one:\n%s", t); free(t);
  for (int i = 0; i < 20; i++) COR_W("line %02d", i);
  t = corLogRingCopy(&len);
  printf("--wrapped (%zu bytes):\n%s", len, t);
  int ok = (len <= 400) && (strncmp(t, "20", 2) == 0) && strstr(t, "line 19\n") && !strstr(t, "first");
  free(t);
  corLogRingInit(0);
  printf("%s\n", (ok && corLogRingCopy(&len) == NULL) ? "OK" : "FAIL");
  return 0;
}
