#ifdef HAVE_BDB
#ifndef _HCACHE_BDB_H_
#define _HCACHE_BDB_H_ 1

#include "hcache.h"

#include <db.h>

struct header_cache
{
  char *folder;
  unsigned int crc;
  DB_ENV *env;
  DB *db;
  int fd;
  char lockfile[_POSIX_PATH_MAX];
};

int hcache_open(header_cache_t * h, const char * path);

#endif /* _HCACHE_BDB_H_ */
#endif /* HAVE_BDB */
