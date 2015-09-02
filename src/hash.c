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
 *      $RCSfile: hash.c,v $
 *      $Author: johns $        $Locker:  $             $State: Exp $
 *      $Revision: 1.10 $      $Date: 2006/01/04 23:58:18 $
 *
 ***************************************************************************
 * DESCRIPTION:
 *   A simple hash table implementation for strings, contributed by John Stone,
 *   derived from his ray tracer code.
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "array.h"
#include "hash.h"

#define HASH_LIMIT 0.5

typedef struct hash_node_t {
  void *data;                         /* data in hash node */
  const char * key;                   /* key for hash lookup */
  struct hash_node_t *next;           /* next node in hash chain */
} hash_node_t;

/*
 * SAL functions to manage HASH objects
 */

obj_t *hash_new(char *name)
{
	obj_t *obj   = object_new(name);
	obj->length  = 0;
	obj->type    = OBJECT_HASH;
	obj->data    = (void *)sal_malloc(sizeof(hash_t));
	hash_init(obj->data, 10);

	return obj;
}

hash_t *hash_get_hash(obj_t *obj)
{
	return obj->data;
}

void hash_set(obj_t *obj, char *str, obj_t *set)
{
	// XXX: support for non-string objects inside da hash
	if (obj->type == OBJECT_HASH) {
	//&&  str->type  == OBJECT_STRING) {
		hash_insert(obj->data, str, set);
	} else {
		printf("INVALID OBJET\n");
		exit(1);
	}
}

obj_t *hash_get(obj_t *obj, obj_t *str)
{
	if (obj->type == OBJECT_HASH && str->type == OBJECT_STRING) {
		return (obj_t *)hash_lookup(obj->data, string_get_value(str));
	}
	printf("hash_get: Invalid objects.\n");
	return NULL;
}

int hash_size(obj_t *obj)
{
	hash_t *h = obj->data;
	if (obj->type == OBJECT_HASH)
		return h->entries;
	return 0;
}

/* htoa opcode (returns values as array) */
obj_t *hash_to_array(obj_t *obj)
{
	hash_node_t *node;
	hash_t *h = obj->data;
	register int i = hash_size(obj);
	obj_t *arr = array_new(i);

	for (node=h->bucket[i]; node!=NULL; node=node->next, i--) {
		printf("ONE: %d\n", i);
		array_set(arr, i, node->data);
	}
	
	return arr;
}

void hash_free(obj_t *obj)
{
	hash_destroy(obj->data);
	free(obj->data);
}

/*
 *  hash() - Hash function returns a hash number for a given key.
 *
 *  tptr: Pointer to a hash table
 *  key: The key to create a hash number for
 */
static int hash(const hash_t *tptr, const char *key) {
  int i=0;
  int hashvalue;
 
  while (*key != '\0')
    i=(i<<3)+(*key++ - '0');
 
  hashvalue = (((i*1103515249)>>tptr->downshift) & tptr->mask);
  if (hashvalue < 0) {
    hashvalue = 0;
  }    

  return hashvalue;
}

/*
 *  hash_init() - Initialize a new hash table.
 *
 *  tptr: Pointer to the hash table to initialize
 *  buckets: The number of initial buckets to create
 */
VMDEXTERNSTATIC void hash_init(hash_t *tptr, int buckets) {

  /* make sure we allocate something */
  if (buckets==0)
    buckets=16;

  /* initialize the table */
  tptr->entries=0;
  tptr->size=2;
  tptr->mask=1;
  tptr->downshift=29;

  /* ensure buckets is a power of 2 */
  while (tptr->size<buckets) {
    tptr->size<<=1;
    tptr->mask=(tptr->mask<<1)+1;
    tptr->downshift--;
  } /* while */

  /* allocate memory for table */
  tptr->bucket=(hash_node_t **) calloc(tptr->size, sizeof(hash_node_t *));

  return;
}

/*
 *  rebuild_table() - Create new hash table when old one fills up.
 *
 *  tptr: Pointer to a hash table
 */
static void rebuild_table(hash_t *tptr) {
  hash_node_t **old_bucket, *old_hash, *tmp;
  int old_size, h, i;

  old_bucket=tptr->bucket;
  old_size=tptr->size;

  /* create a new table and rehash old buckets */
  hash_init(tptr, old_size<<1);
  for (i=0; i<old_size; i++) {
    old_hash=old_bucket[i];
    while(old_hash) {
      tmp=old_hash;
      old_hash=old_hash->next;
      h=hash(tptr, tmp->key);
      tmp->next=tptr->bucket[h];
      tptr->bucket[h]=tmp;
      tptr->entries++;
    } /* while */
  } /* for */

  /* free memory used by old table */
  free(old_bucket);

  return;
}

/*
 *  hash_lookup() - Lookup an entry in the hash table and return a pointer to
 *    it or HASH_FAIL if it wasn't found.
 *
 *  tptr: Pointer to the hash table
 *  key: The key to lookup
 */
VMDEXTERNSTATIC void *hash_lookup(const hash_t *tptr, const char *key) {
  int h;
  hash_node_t *node;

  /* find the entry in the hash table */
  h=hash(tptr, key);
  for (node=tptr->bucket[h]; node!=NULL; node=node->next) {
    if (!strcmp(node->key, key))
      break;
  }

  /* return the entry if it exists, or HASH_FAIL */
  return(node ? node->data : NULL);
}

/*
 *  hash_insert() - Insert an entry into the hash table.  If the entry already
 *  exists return a pointer to it, otherwise return HASH_FAIL.
 *
 *  tptr: A pointer to the hash table
 *  key: The key to insert into the hash table
 *  data: A pointer to the data to insert into the hash table
 */
VMDEXTERNSTATIC void *hash_insert(hash_t *tptr, const char *key, void *data) {
  void *tmp;
  hash_node_t *node;
  int h;

  /* check to see if the entry exists */
  if ((tmp=hash_lookup(tptr, key)) != NULL)
    return(tmp);

  /* expand the table if needed */
  while (tptr->entries>=HASH_LIMIT*tptr->size)
    rebuild_table(tptr);

  /* insert the new entry */
  h=hash(tptr, key);
  node=(struct hash_node_t *) sal_malloc(sizeof(hash_node_t));
  node->data=data;
  node->key=key;
  node->next=tptr->bucket[h];
  tptr->bucket[h]=node;
  tptr->entries++;

  return NULL;
}

/*
 *  hash_delete() - Remove an entry from a hash table and return a pointer
 *  to its data or HASH_FAIL if it wasn't found.
 *
 *  tptr: A pointer to the hash table
 *  key: The key to remove from the hash table
 */
VMDEXTERNSTATIC void *hash_delete(hash_t *tptr, const char *key) {
  hash_node_t *node, *last;
  void *data;
  int h;

  /* find the node to remove */
  h=hash(tptr, key);
  for (node=tptr->bucket[h]; node; node=node->next) {
    if (!strcmp(node->key, key))
      break;
  }

  /* Didn't find anything, return HASH_FAIL */
  if (node==NULL)
    return NULL;

  /* if node is at head of bucket, we have it easy */
  if (node==tptr->bucket[h])
    tptr->bucket[h]=node->next;
  else {
    /* find the node before the node we want to remove */
    for (last=tptr->bucket[h]; last && last->next; last=last->next) {
      if (last->next==node)
        break;
    }
    last->next=node->next;
  }

  /* free memory and return the data */
  data=node->data;
  free(node);

  return(data);
}



/*
 * hash_destroy() - Delete the entire table, and all remaining entries.
 * 
 */
VMDEXTERNSTATIC void hash_destroy(hash_t *tptr) {
  hash_node_t *node, *last;
  int i;

  for (i=0; i<tptr->size; i++) {
    node = tptr->bucket[i];
    while (node != NULL) { 
      last = node;   
      node = node->next;
      free(last);
    }
  }     

  /* free the entire array of buckets */
  if (tptr->bucket != NULL) {
    free(tptr->bucket);
    memset(tptr, 0, sizeof(hash_t));
  }
}

/*
 *  alos() - Find the average length of search.
 *
 *  tptr: Pointer to a hash table
 */
static float alos(hash_t *tptr) {
  int i,j;
  float alos=0;
  hash_node_t *node;


  for (i=0; i<tptr->size; i++) {
    for (node=tptr->bucket[i], j=0; node!=NULL; node=node->next, j++);
    if (j)
      alos+=((j*(j+1))>>1);
  } /* for */

  return(tptr->entries ? alos/tptr->entries : 0);
}


/*
 *  hash_stats() - Return a string with stats about a hash table.
 *
 *  tptr: A pointer to the hash table
 */
VMDEXTERNSTATIC char * hash_stats(hash_t *tptr) {
  static char buf[1024];

  sprintf(buf, "%u slots, %u entries, and %1.2f ALOS",
    (int)tptr->size, (int)tptr->entries, alos(tptr));

  return(buf);
}
