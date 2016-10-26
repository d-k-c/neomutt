/**
 * Copyright (c) 2016 Pietro Cerutti <gahr@gahr.ch> All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifdef HAVE_LMDB
#ifndef _HCACHE_LMDB_H_
#define _HCACHE_LMDB_H_ 1

#include "hcache.h"

#define LMDB_DB_SIZE (1024 * 1024 * 1024)
#include <lmdb.h>

enum mdb_txn_mode
{
  txn_uninitialized = 0,
  txn_read          = 1 << 0,
  txn_write         = 1 << 1
};

struct header_cache
{
  char *folder;
  unsigned int crc;
  MDB_env *env;
  MDB_txn *txn;
  MDB_dbi db;
  enum mdb_txn_mode txn_mode;
};

int hcache_open(header_cache_t * h, const char * path);

#endif /* _HCACHE_LMDB_H_ */
#endif /* HAVE_LMDB */
