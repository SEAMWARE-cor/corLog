#ifndef CORLOG_CORLOGRING_H_
#define CORLOG_CORLOGRING_H_

//
// FILE            corLogRing.h
//
// AUTHOR          Ken Zangelin
//
// Copyright 2026 Seamware
// SPDX-License-Identifier: Apache-2.0
//
#include <stdbool.h>                             // bool
#include <stddef.h>                              // size_t



// -----------------------------------------------------------------------------
//
// corLogRingInit - keep the last 'bytes' of the log in memory as well, wherever it goes (file, stdout): each line
// with its UTC time first - "2026-11-20T12:00:01.123Z W: ...". Off until called; 0 turns it off again.
// false: no memory for it.
//
extern bool corLogRingInit(size_t bytes);



// -----------------------------------------------------------------------------
//
// corLogRingCopy - the lines kept, oldest first, as one zero-terminated text (malloc'ed - the caller frees it);
// NULL when the ring is off. *lenP: its length.
//
extern char* corLogRingCopy(size_t* lenP);



// -----------------------------------------------------------------------------
//
// corLogRingAdd - a line into the ring (corLogOut calls it, under corLogMutex)
//
struct iovec;
extern void corLogRingAdd(const struct iovec* comp, int comps);

#endif  // CORLOG_CORLOGRING_H_
