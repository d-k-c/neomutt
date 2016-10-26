#include "config.h"

#ifdef HAVE_KC

#include "hcache-kc.h"

void *
mutt_hcache_fetch_raw (header_cache_t *h, const char *filename,
                       size_t(*keylen) (const char *fn))
{
  size_t sp;

	if (!h)
		return NULL;

  return kcdbget(h->db, filename, keylen(filename), &sp);
}

int
mutt_hcache_store_raw (header_cache_t* h, const char* filename, void* data,
                       size_t dlen, size_t(*keylen) (const char* fn))
{
	if (!h)
		return -1;

  return kcdbset(h->db, filename, keylen(filename), data, dlen);
}

int
hcache_open(header_cache_t * h, const char* path)
{
  char kcdbpath[_POSIX_PATH_MAX];
  int printfresult;

  printfresult = snprintf(kcdbpath, sizeof(kcdbpath),
                          "%s#type=kct#opts=%s#rcomp=lex",
                          path, option(OPTHCACHECOMPRESS) ? "lc" : "l");
  if ((printfresult < 0) || (printfresult >= sizeof(kcdbpath)))
  {
    return -1;
  }

  h->db = kcdbnew();
  if (!h->db)
      return -1;

  if (kcdbopen(h->db, kcdbpath, KCOWRITER | KCOCREATE))
    return 0;
  else
  {
#ifdef DEBUG
    int ecode = kcdbecode (h->db);
    dprint (2, (debugfile, "kcdbopen failed for %s: %s (ecode %d)\n", kcdbpath, kcdbemsg (h->db), ecode));
#endif
    kcdbdel(h->db);
    return -1;
  }
}

void
mutt_hcache_close(header_cache_t *h)
{
  if (!h)
    return;

  if (!kcdbclose(h->db))
  {
#ifdef DEBUG
    int ecode = kcdbecode (h->db);
    dprint (2, (debugfile, "kcdbclose failed for %s: %s (ecode %d)\n", h->folder, kcdbemsg (h->db), ecode));
#endif
  }
  kcdbdel(h->db);
  FREE(&h->folder);
  FREE(&h);
}

int
mutt_hcache_delete(header_cache_t *h, const char *filename,
		   size_t(*keylen) (const char *fn))
{
  if (!h)
    return -1;

  return kcdbremove(h->db, filename, keylen(filename));
}

const char *mutt_hcache_backend (void)
{
  /* SHORT_STRING(128) should be more than enough for KCVERSION */
  static char version_cache[SHORT_STRING] = "";
  if (!version_cache[0])
    snprintf(version_cache, sizeof(version_cache), "kyotocabinet %s", KCVERSION);

  return version_cache;
}

#endif /* HAVE_KC */
