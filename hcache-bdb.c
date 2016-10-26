#include "config.h"

#ifdef HAVE_BDB

#include "hcache-bdb.h"
#include "mx.h"
#include <fcntl.h>

static void
mutt_hcache_dbt_init(DBT * dbt, void *data, size_t len)
{
  dbt->data = data;
  dbt->size = dbt->ulen = len;
  dbt->dlen = dbt->doff = 0;
  dbt->flags = DB_DBT_USERMEM;
}

static void
mutt_hcache_dbt_empty_init(DBT * dbt)
{
  dbt->data = NULL;
  dbt->size = dbt->ulen = dbt->dlen = dbt->doff = 0;
  dbt->flags = 0;
}

void *
mutt_hcache_fetch_raw (header_cache_t *h, const char *filename,
                       size_t(*keylen) (const char *fn))
{
  DBT key;
  DBT data;

  if (!h)
    return NULL;

  mutt_hcache_dbt_init(&key, (void *) filename, keylen(filename));
  mutt_hcache_dbt_empty_init(&data);
  data.flags = DB_DBT_MALLOC;
  
  h->db->get(h->db, NULL, &key, &data, 0);
  
  return data.data;
}

int
mutt_hcache_store_raw (header_cache_t* h, const char* filename, void* data,
                       size_t dlen, size_t(*keylen) (const char* fn))
{
  DBT key;
  DBT databuf;

	if (!h)
		return -1;

  mutt_hcache_dbt_init(&key, (void *) filename, keylen(filename));
  
  mutt_hcache_dbt_empty_init(&databuf);
  databuf.flags = DB_DBT_USERMEM;
  databuf.data = data;
  databuf.size = dlen;
  databuf.ulen = dlen;
  
  return h->db->put(h->db, NULL, &key, &databuf, 0);
}

int
hcache_open(header_cache_t * h, const char* path)
{
  struct stat sb;
  int ret;
  u_int32_t createflags = DB_CREATE;
  int pagesize;

  if (mutt_atoi (HeaderCachePageSize, &pagesize) < 0 || pagesize <= 0)
    pagesize = 16384;

  snprintf (h->lockfile, _POSIX_PATH_MAX, "%s-lock-hack", path);

  h->fd = open (h->lockfile, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
  if (h->fd < 0)
    return -1;

  if (mx_lock_file (h->lockfile, h->fd, 1, 0, 5))
    goto fail_close;

  ret = db_env_create (&h->env, 0);
  if (ret)
    goto fail_unlock;

  ret = (*h->env->open)(h->env, NULL, DB_INIT_MPOOL | DB_CREATE | DB_PRIVATE,
	0600);
  if (ret)
    goto fail_env;

  h->db = NULL;
  ret = db_create (&h->db, h->env, 0);
  if (ret)
    goto fail_env;

  if (stat(path, &sb) != 0 && errno == ENOENT)
  {
    createflags |= DB_EXCL;
    h->db->set_pagesize(h->db, pagesize);
  }

  ret = (*h->db->open)(h->db, NULL, path, h->folder, DB_BTREE, createflags,
                       0600);
  if (ret)
    goto fail_db;

  return 0;

  fail_db:
  h->db->close (h->db, 0);
  fail_env:
  h->env->close (h->env, 0);
  fail_unlock:
  mx_unlock_file (h->lockfile, h->fd, 0);
  fail_close:
  close (h->fd);
  unlink (h->lockfile);

  return -1;
}

void
mutt_hcache_close(header_cache_t *h)
{
  if (!h)
    return;

  h->db->close (h->db, 0);
  h->env->close (h->env, 0);
  mx_unlock_file (h->lockfile, h->fd, 0);
  close (h->fd);
  unlink (h->lockfile);
  FREE (&h->folder);
  FREE (&h);
}

int
mutt_hcache_delete(header_cache_t *h, const char *filename,
		   size_t(*keylen) (const char *fn))
{
  DBT key;

  if (!h)
    return -1;

  mutt_hcache_dbt_init(&key, (void *) filename, keylen(filename));
  return h->db->del(h->db, NULL, &key, 0);
}

const char *mutt_hcache_backend (void)
{
  return DB_VERSION_STRING;
}

#endif /* HAVE_BDB */
