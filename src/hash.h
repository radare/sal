/***************************************************************************
 *cr
 *cr            (C) Copyright 1995-2006 The Board of Trustees of the
 *cr                        University of Illinois
 *cr                         All Rights Reserved
 *cr
 ***************************************************************************/

/***************************************************************************
 * RCS INFORMATION:
 *
 *      $RCSfile: hash.h,v $
 *      $Author: johns $        $Locker:  $             $State: Exp $
 *      $Revision: 1.8 $      $Date: 2006/01/04 23:58:18 $
 *
 ***************************************************************************
 * DESCRIPTION:
 *   A simple hash table implementation for strings, contributed by John Stone,
 *   derived from his ray tracer code.
 ***************************************************************************/
#ifndef HASH_H
#define HASH_H

typedef struct hash_t {
  struct hash_node_t **bucket;        /* array of hash nodes */
  int size;                           /* size of the array */
  int entries;                        /* number of entries in table */
  int downshift;                      /* shift cound, used in hash function */
  int mask;                           /* used to select bits for hashing */
} hash_t;

#define HASH_FAIL -1

#if defined(VMDPLUGIN_STATIC)
#define VMDEXTERNSTATIC static
#include "hash.c"
#else

#define VMDEXTERNSTATIC 

#ifdef __cplusplus
extern "C" {
#endif

/* sal ones */
obj_t *hash_new(char *name);
void hash_set(obj_t *obj, char *str, obj_t *set);
obj_t *hash_get(obj_t *obj, obj_t *str);
int hash_size(obj_t *obj);
hash_t *hash_get_hash(obj_t *obj);
int hash_size(obj_t *obj);
obj_t *hash_to_array(obj_t *obj);
void hash_free(obj_t *obj);
/* sal ones */

void hash_init(hash_t *, int);

void *hash_lookup (const hash_t *, const char *);

void *hash_insert (hash_t *, const char *, void *);

void *hash_delete (hash_t *, const char *);

void hash_destroy(hash_t *);

char *hash_stats (hash_t *);

#ifdef __cplusplus
}
#endif

#endif

#endif
