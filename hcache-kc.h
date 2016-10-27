#ifdef HAVE_KC
#ifndef _HCACHE_KC_H_
#define _HCACHE_KC_H_ 1

#include "hcache.h"

#include <kclangc.h>

struct header_cache
{
  char *folder;
  unsigned int crc;
  KCDB *db;
};

#endif /* _HCACHE_KC_H_ */
#endif /* HAVE_KC */
