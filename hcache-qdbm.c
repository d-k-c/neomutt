#include "config.h"

#ifdef HAVE_QDBM

#include "hcache-qdbm.h"

void *
mutt_hcache_fetch_raw (header_cache_t *h, const char *filename,
                       size_t(*keylen) (const char *fn))
{
  if (!h)
    return NULL;

  return vlget(h->db, filename, keylen(filename), NULL);
}

int
mutt_hcache_store_raw (header_cache_t* h, const char* filename, void* data,
                       size_t dlen, size_t(*keylen) (const char* fn))
{
  return vlput(h->db, filename, keylen(filename), data, dlen, VL_DOVER);
}

int
hcache_open (struct header_cache* h, const char* path)
{
  int flags = VL_OWRITER | VL_OCREAT;

  if (option(OPTHCACHECOMPRESS))
    flags |= VL_OZCOMP;

  h->db = vlopen (path, flags, VL_CMPLEX);
  if (h->db)
    return 0;
  else
    return -1;
}

void
mutt_hcache_close(header_cache_t *h)
{
  if (!h)
    return;

  vlclose(h->db);
  FREE(&h->folder);
  FREE(&h);
}

int
mutt_hcache_delete(header_cache_t *h, const char *filename,
                   size_t(*keylen) (const char *fn))
{
  if (!h)
    return -1;

  return vlout(h->db, filename, keylen(filename));
}

const char *mutt_hcache_backend (void)
{
  return "qdbm " _QDBM_VERSION;
}

#endif /* HAVE_LMDB */
