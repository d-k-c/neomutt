#ifdef HAVE_QDBM
#ifndef _HCACHE_QDBM_H_
#define _HCACHE_QDBM_H_ 1

#include "hcache.h"

#include <depot.h>
#include <cabin.h>
#include <villa.h>

struct header_cache
{
  char *folder;
  unsigned int crc;
  VILLA *db;
};

#endif /* _HCACHE_QDBM_H_ */
#endif /* HAVE_QDBM */
