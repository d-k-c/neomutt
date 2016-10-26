#include "config.h"

#ifdef HAVE_GDBM

#include "hcache-gdbm.h"

void *
mutt_hcache_fetch_raw (header_cache_t *h, const char *filename,
                       size_t(*keylen) (const char *fn))
{
  datum key;
  datum data;
  
  if (!h)
    return NULL;

  key.dptr = (char *)filename;
  key.dsize = keylen(filename);
  data = gdbm_fetch(h->db, key);
  return data.dptr;
}

int
mutt_hcache_store_raw (header_cache_t* h, const char* filename, void* data,
                       size_t dlen, size_t(*keylen) (const char* fn))
{
  datum key;
  datum databuf;
  
  if (!h)
    return -1;

  key.dptr = (char *)filename;
  key.dsize = keylen(filename);
  
  databuf.dsize = dlen;
  databuf.dptr = data;
  
  return gdbm_store(h->db, key, databuf, GDBM_REPLACE);
}

int
hcache_open (header_cache_t * h, const char* path)
{
  int pagesize;

  if (mutt_atoi (HeaderCachePageSize, &pagesize) < 0 || pagesize <= 0)
    pagesize = 16384;

  h->db = gdbm_open((char *) path, pagesize, GDBM_WRCREAT, 00600, NULL);
  if (h->db)
    return 0;

  /* if rw failed try ro */
  h->db = gdbm_open((char *) path, pagesize, GDBM_READER, 00600, NULL);
  if (h->db)
    return 0;

  return -1;
}

void
mutt_hcache_close(header_cache_t *h)
{
  if (!h)
    return;

  gdbm_close(h->db);
  FREE(&h->folder);
  FREE(&h);
}

int
mutt_hcache_delete(header_cache_t *h, const char *filename,
		   size_t(*keylen) (const char *fn))
{
  datum key;

  if (!h)
    return -1;

  key.dptr = (char *)filename;
  key.dsize = keylen(filename);

  return gdbm_delete(h->db, key);
}

const char *mutt_hcache_backend (void)
{
  return gdbm_version;
}

#endif /* HAVE_GDBM */
