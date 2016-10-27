#ifdef HAVE_GDBM
#ifndef _HCACHE_GDBM_H_
#define _HCACHE_GDBM_H_ 1

#include "hcache.h"

#include <gdbm.h>

struct header_cache
{
  char *folder;
  unsigned int crc;
  GDBM_FILE db;
};

#endif /* _HCACHE_GDBM_H_ */
#endif /* HAVE_GDBM */