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

#include "config.h"

#ifdef HAVE_LMDB

#include "hcache-lmdb.h"

static int mdb_get_r_txn(header_cache_t *h)
{
  int rc;

  if (h->txn && (h->txn_mode & (txn_read | txn_write)) > 0)
    return MDB_SUCCESS;

  if (h->txn)
    rc = mdb_txn_renew(h->txn);
  else
    rc = mdb_txn_begin(h->env, NULL, MDB_RDONLY, &h->txn);

  if (rc == MDB_SUCCESS)
    h->txn_mode = txn_read;

  return rc;
}

static int mdb_get_w_txn(header_cache_t *h)
{
  int rc;

  if (h->txn && (h->txn_mode == txn_write))
    return MDB_SUCCESS;

  if (h->txn)
  {
    if (h->txn_mode == txn_read)
      mdb_txn_reset(h->txn);
    h->txn = NULL;
  }

  rc = mdb_txn_begin(h->env, NULL, 0, &h->txn);
  if (rc == MDB_SUCCESS)
    h->txn_mode = txn_write;

  return rc;
}

void *
mutt_hcache_fetch_raw (header_cache_t *h, const char *filename,
                       size_t(*keylen) (const char *fn))
{
  MDB_val key;
  MDB_val data;
  int rc;

  if (!h)
      return NULL;

  key.mv_data = (char *)filename;
  key.mv_size = keylen(filename);
  data.mv_data = NULL;
  data.mv_size = 0;
  rc = mdb_get_r_txn(h);
  if (rc != MDB_SUCCESS)
  {
    h->txn = NULL;
    fprintf(stderr, "txn_renew: %s\n", mdb_strerror(rc));
    return NULL;
  }
  rc = mdb_get(h->txn, h->db, &key, &data);
  if (rc == MDB_NOTFOUND)
  {
    return NULL;
  }
  if (rc != MDB_SUCCESS)
  {
    fprintf(stderr, "mdb_get: %s\n", mdb_strerror(rc));
    return NULL;
  }
  /* Caller frees the data we return, so I MUST make a copy of it */

  char *d = safe_malloc(data.mv_size);
  memcpy(d, data.mv_data, data.mv_size);

  return d;
}

int
mutt_hcache_store_raw (header_cache_t* h, const char* filename, void* data,
                       size_t dlen, size_t(*keylen) (const char* fn))
{

  MDB_val key;
  MDB_val databuf;
  int rc;

  if (!h)
      return -1;

  key.mv_data = (char *)filename;
  key.mv_size = keylen(filename);
  databuf.mv_data = data;
  databuf.mv_size = dlen;
  rc = mdb_get_w_txn(h);
  if (rc != MDB_SUCCESS)
  {
    fprintf(stderr, "txn_begin: %s\n", mdb_strerror(rc));
    return rc;
  }
  rc = mdb_put(h->txn, h->db, &key, &databuf, 0);
  if (rc != MDB_SUCCESS)
  {
    fprintf(stderr, "mdb_put: %s\n", mdb_strerror(rc));
    mdb_txn_abort(h->txn);
    h->txn_mode = txn_uninitialized;
    h->txn = NULL;
    return rc;
  }
  return rc;
}

int
hcache_open (struct header_cache* h, const char* path)
{
  int rc;

  h->txn = NULL;
  h->db  = 0;

  rc = mdb_env_create(&h->env);
  if (rc != MDB_SUCCESS)
  {
    fprintf(stderr, "hcache_open_lmdb: mdb_env_create: %s", mdb_strerror(rc));
    return -1;
  }

  mdb_env_set_mapsize(h->env, LMDB_DB_SIZE);

  rc = mdb_env_open(h->env, path, MDB_NOSUBDIR, 0644);
  if (rc != MDB_SUCCESS)
  {
    fprintf(stderr, "hcache_open_lmdb: mdb_env_open: %s", mdb_strerror(rc));
    goto fail_env;
  }

  rc = mdb_get_r_txn(h);
  if (rc != MDB_SUCCESS)
  {
      fprintf(stderr, "hcache_open_lmdb: mdb_txn_begin: %s", mdb_strerror(rc));
      goto fail_env;
  }

  rc = mdb_dbi_open(h->txn, NULL, MDB_CREATE, &h->db);
  if (rc != MDB_SUCCESS)
  {
    fprintf(stderr, "hcache_open_lmdb: mdb_dbi_open: %s", mdb_strerror(rc));
    goto fail_dbi;
  }

  mdb_txn_reset(h->txn);
  h->txn_mode = txn_uninitialized;
  return 0;

fail_dbi:
  mdb_txn_abort(h->txn);
  h->txn_mode = txn_uninitialized;
  h->txn = NULL;

fail_env:
  mdb_env_close(h->env);
  return -1;
}

void
mutt_hcache_close(header_cache_t *h)
{
  if (!h)
    return;

  if (h->txn && h->txn_mode == txn_write)
  {
    mdb_txn_commit(h->txn);
    h->txn_mode = txn_uninitialized;
    h->txn = NULL;
  }

  mdb_env_close(h->env);
  FREE (&h->folder);
  FREE (&h);
}

int
mutt_hcache_delete(header_cache_t *h, const char *filename,
                   size_t(*keylen) (const char *fn))
{
  MDB_val key;
  int rc;

  if (!h)
    return -1;

  key.mv_data = (char *)filename;
  key.mv_size = keylen(filename);
  rc = mdb_get_w_txn(h);
  if (rc != MDB_SUCCESS)
  {
    fprintf(stderr, "txn_begin: %s\n", mdb_strerror(rc));
    return rc;
  }
  rc = mdb_del(h->txn, h->db, &key, NULL);
  if (rc != MDB_SUCCESS)
  {
    if (rc != MDB_NOTFOUND)
    {
      fprintf(stderr, "mdb_del: %s\n", mdb_strerror(rc));
      mdb_txn_abort(h->txn);
      h->txn_mode = txn_uninitialized;
      h->txn = NULL;
    }
    return rc;
  }

  return rc;
}

const char *mutt_hcache_backend (void)
{
  return "lmdb " MDB_VERSION_STRING;
}

#endif /* HAVE_LMDB */
