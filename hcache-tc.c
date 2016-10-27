#include "config.h"

#ifdef HAVE_TC

#include "hcache-tc.h"

void *
mutt_hcache_fetch_raw (header_cache_t *h, const char *filename,
                       size_t(*keylen) (const char *fn))
{
  int sp;

  if (!h)
    return NULL;

  return tcbdbget(h->db, filename, keylen(filename), &sp);
}

int
mutt_hcache_store_raw (header_cache_t* h, const char* filename, void* data,
                       size_t dlen, size_t(*keylen) (const char* fn))
{
	if (!h)
		return -1;

  return tcbdbput(h->db, filename, keylen(filename), data, dlen);
}

int
hcache_open(header_cache_t * h, const char* path)
{
  h->db = tcbdbnew();
  if (!h->db)
      return -1;
  if (option(OPTHCACHECOMPRESS))
    tcbdbtune(h->db, 0, 0, 0, -1, -1, BDBTDEFLATE);
  if (tcbdbopen(h->db, path, BDBOWRITER | BDBOCREAT))
    return 0;
  else
  {
#ifdef DEBUG
    int ecode = tcbdbecode (h->db);
    dprint (2, (debugfile, "tcbdbopen failed for %s: %s (ecode %d)\n", path, tcbdberrmsg (ecode), ecode));
#endif
    tcbdbdel(h->db);
    return -1;
  }
}

void
mutt_hcache_close(header_cache_t *h)
{
  if (!h)
    return;

  if (!tcbdbclose(h->db))
  {
#ifdef DEBUG
    int ecode = tcbdbecode (h->db);
    dprint (2, (debugfile, "tcbdbclose failed for %s: %s (ecode %d)\n", h->folder, tcbdberrmsg (ecode), ecode));
#endif
  }
  tcbdbdel(h->db);
  FREE(&h->folder);
  FREE(&h);
}

int
mutt_hcache_delete(header_cache_t *h, const char *filename,
		   size_t(*keylen) (const char *fn))
{
  if (!h)
    return -1;

  return tcbdbout(h->db, filename, keylen(filename));
}

const char *mutt_hcache_backend (void)
{
  return "tokyocabinet " _TC_VERSION;
}

#endif /* HAVE_TC */
