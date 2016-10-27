#ifdef HAVE_TC
#ifndef _HCACHE_TC_H_
#define _HCACHE_TC_H_ 1

#include "hcache.h"

#include <tcbdb.h>

struct header_cache
{
  char *folder;
  unsigned int crc;
  TCBDB *db;
};

#endif /* _HCACHE_TC_H_ */
#endif /* HAVE_TC */
