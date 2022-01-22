#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"

#define UNIT_IBUCKET 64
#define MAX_IBUCKET 1000000

enum {HASH_MULTIPLIER = 65599};

struct UserInfo {
  char *name;                	// customer name
  char *id;                  	// customer id
  int purchase;              	// purchase amount (> 0)
  struct UserInfo* nextID;	// pointer to next user in ID hashtable
  struct UserInfo* nextName;	//pointer to next user in Name hashtable
  struct UserInfo* prevID;  	//pointer to prev user in ID hashtable
  struct UserInfo* prevName;	//pointer to prev user in Name hashtable
};

struct DB {
  struct UserInfo *pIDTab;   	// pointer to the ID table
  struct UserInfo *pNameTab; 	// pointer to Name table
  int buckets;            	// # of buckets
  int nodes;              	// # of nodes
};

static int hash_func(const char *pcKey, int iBucketCount)
{
   int i;
   unsigned int uiHash = 0U;
   for (i = 0; pcKey[i] != '\0'; i++)
      uiHash = uiHash * (unsigned int)HASH_MULTIPLIER
               + (unsigned int)pcKey[i];
   return (int)(uiHash % (unsigned int)(iBucketCount-1));
}

DB_T
CreateCustomerDB(void)
{
  DB_T d;
  
  d = (DB_T) calloc(1, sizeof(struct DB));
  if (d == NULL) {
    fprintf(stderr, "Can't allocate a memory for DB_T\n");
    return NULL;
  }
  d->buckets = UNIT_IBUCKET; // start with 64 elements
  d->pIDTab = (struct UserInfo *)
    calloc(d->buckets, sizeof(struct UserInfo));
  d->pNameTab = (struct UserInfo *)
    calloc(d->buckets, sizeof(struct UserInfo));
    
  if (d->pIDTab == NULL || d->pNameTab == NULL) {
    fprintf(stderr, 
      "Can't allocate a memory for ID/Name table of size %d\n",
      d->buckets);   
    free(d);
    return NULL;
  }
  
  return d;
  
}
/*--------------------------------------------------------------------*/
// delete all nodes by iterate through every node in every bucket
// delete ID Table and Name Table
// delete d
void
DestroyCustomerDB(DB_T d)
{  
  if (d) {
    free(d->pNameTab);
    int i;
    struct UserInfo *p, *q;
    // free d->pIDTab with all registered nodes
    for (i=0; i< d->buckets; i++){
    	p = d->pIDTab[i].nextID;
    	while (p) {
    	    q=p;
    	    p = p->nextID;
    	    free(q);
    	}
    }
    free(d->pIDTab);
    free(d);	
  }
  return;
}

/*--------------------------------------------------------------------*/
// calculate hash-value of id and name
// iterate through those id-bucket & name-bucket to check for duplicate
// allocate memo for a new UserInfo, assgin id, name and purchase to it
// increment node count
// if nodecount is 75% of buckets, expand the hash table
// rehash all old customers in old table to new table
// hash the newly added customer, add them to the table
int
RegisterCustomer(DB_T d, const char *id,
		 const char *name, const int purchase)
{
  if( !d || !*name || !*id || purchase <=0) return (-1);

  struct UserInfo* p, *q;
  struct UserInfo* new;
  struct UserInfo* temp;
  
  int hash_id =  hash_func(id, d->buckets);
  int hash_name = hash_func(name, d->buckets);
  
  /*check for id duplication*/
  for (p = d->pIDTab[hash_id].nextID; p!=NULL; p = p->nextID){
    if(!strcmp(p->id, id))
    {
      fprintf(stderr, "ID already exists\n");
      return (-1);
    }
  }
  /*check for name duplication*/
  for (p = d->pNameTab[hash_name].nextName; p!=NULL; p = p->nextName){
    if(!strcmp(p->name, name))
    {
      fprintf(stderr, "Name already exists\n");
      return (-1);
    }
  } 

  /*allocate memory for new UserInfo struct*/
  new = (struct UserInfo *) calloc(1, sizeof(struct UserInfo));
  if (new == NULL){
    fprintf(stderr, "Can't allocate memory for new data\n");
    return (-1);
  }   
  // initialize new
  new->id = strdup(id);
  new->name = strdup(name);
  if(!(new->id) ||!(new->name)){
    fprintf(stderr, "Can't duplicate name/id\n");
    return (-1);
  }
  new->purchase = purchase;
  d->nodes++;
  
  //check if the current table need to be expanded 
  if ((float) d->nodes >= (float) 0.75 * d->buckets){
    //expand table if necessary
    int i, rehash_name, rehash_id;
    int old_bucket_size = d->buckets;
    if (d->buckets > MAX_IBUCKET/2) d->buckets = MAX_IBUCKET;
    d->buckets += d->buckets;
    
    d->pIDTab = (struct UserInfo*)
      realloc (d->pIDTab, d->buckets*sizeof(struct UserInfo));
    d->pNameTab = (struct UserInfo*)
      realloc (d->pNameTab, d->buckets*sizeof(struct UserInfo));
    
    if(!d->pIDTab ||!d->pNameTab) 
    {	fprintf(stderr, "Can't expand the hash table\n");
    	return (-1);
    }
    //rehash the new customer
    hash_id = hash_func(id, d->buckets);
    hash_name = hash_func(name, d->buckets);
    
    //rehash elements into new tables
    for (i =0; i<old_bucket_size; i++ ){
      q= d->pIDTab[i].nextID;
      if (q) q->prevID = & d->pIDTab[i];
      while(q) {
	rehash_id = hash_func(q->id, d->buckets);
	if (rehash_id == hash_func(q->id, old_bucket_size)){
	  q=q->nextID;
	  continue;
	}
	temp = q->nextID;
	q->prevID->nextID = q->nextID;
	if (q->nextID) q->nextID->prevID = q->prevID;

	q->nextID = d->pIDTab[rehash_id].nextID;
        if (q->nextID) q->nextID->prevID = q;
	q->prevID = d->pIDTab + rehash_id;
	d->pIDTab[rehash_id].nextID = q;
	
	q = temp;
      }

      p=d->pNameTab[i].nextName;
      if (p) p->prevName =& d->pNameTab[i];
      while(p)
      {
	rehash_name = hash_func(p->name, d->buckets);
	if (rehash_name == i) p=p->nextName;
	else {
	  temp = p->nextName;
	  p->prevName->nextName = p->nextName;
	  if (p->nextName) p->nextName->prevName = p->prevName;
	  
	  p->nextName= d->pNameTab[rehash_name].nextName;
	  if (p->nextName) d->pNameTab[rehash_name].nextName->prevName = p;
	  d->pNameTab[rehash_name].nextName =p;
	  p->prevName = d->pNameTab + rehash_name;
	  p=temp;
	}
      }

    }
  }
  
  /*update the links*/
  new->nextID = d->pIDTab[hash_id].nextID;
  if (new->nextID) new->nextID->prevID = new;
  d->pIDTab[hash_id].nextID = new;
  new->prevID = &(d->pIDTab[hash_id]);
  
  new->nextName = d->pNameTab[hash_name].nextName;
  if (new->nextName) new->nextName->prevName = new;
  d->pNameTab[hash_name].nextName = new;
  new->prevName = &(d->pNameTab[hash_name]);
  
  return 0;
}
/*--------------------------------------------------------------------*/
// calculate hash-value of id
// iterate through those id-bucket to find matched id
// delete customer
int
UnregisterCustomerByID(DB_T d, const char *id)
{  
  if (!d || !*id) return (-1);
  int hash_id =  hash_func(id, d->buckets);
  int hash_name;
  
  struct UserInfo* p,*q;
  for (p = d->pIDTab[hash_id].nextID; p; p = p->nextID){
    if(!strcmp(p->id, id))
    { 
      if (p->nextID){
        p->prevID->nextID = p->nextID;
        p->nextID->prevID = p->prevID;
      }
      else p->prevID->nextID = NULL;
      hash_name = hash_func(p->name, d->buckets);
      
      for (q = d->pNameTab[hash_name].nextName; q; q = q->nextName){
   	if(!strcmp(q->name, p->name))
   	{ 
      	  if (q->nextName){
            q->prevName->nextName = q->nextName;
            q->nextName->prevName = q->prevName;
      	  }
        else q->prevName->nextName = NULL;
        free(q);
        d->nodes--;
        return 0;
    	}
      }
    }
  }
  fprintf(stderr,"Can't find user with such id\n");
  return (-1);
}

/*--------------------------------------------------------------------*/
// calculate hash-value of name
// iterate through those name-bucket to find matched name
// delete customer
int
UnregisterCustomerByName(DB_T d, const char *name)
{
  if (!d || !*name) return (-1);
  int hash_name =  hash_func(name, d->buckets);
  int hash_id;
  struct UserInfo* p,*q;
  for (p = d->pNameTab[hash_name].nextName; p; p = p->nextName){
    if(!strcmp(p->name, name))
    { 
      if (p->nextName){
        p->prevName->nextName = p->nextName;
        p->nextName->prevName = p->prevName;
      }
      else p->prevName->nextName = NULL;
      hash_id = hash_func(p->id, d->buckets);
      
      for (q = d->pIDTab[hash_id].nextID; q; q = q->nextID){
        if(!strcmp(q->id, p->id))
	{ 
	  if (q->nextID){
		q->prevID->nextID = q->nextID;
		q->nextID->prevID = q->prevID;
	      }
	  else q->prevID->nextID = NULL;
	  free(q);
	  d->nodes--;
	  return 0;
        }
      }
    }
  }
  fprintf(stderr,"Can't find user with such name\n");
  return (-1);
}
/*--------------------------------------------------------------------*/
// calculate hash-value of id
// iterate through those id-bucket to find matched id
// return purchase of them
int
GetPurchaseByID(DB_T d, const char* id)
{
  if (!d || !*id) return (-1);
  int hash_id =  hash_func(id, d->buckets);
  struct UserInfo* p;
  for (p = d->pIDTab[hash_id].nextID; p; p = p->nextID){
    if(!strcmp(p->id, id)) return p->purchase;
  }
  fprintf(stderr,"Can't find user with such id\n");
  return (-1);
}
/*--------------------------------------------------------------------*/
// calculate hash-value of name
// iterate through those name-bucket to find matched name
// return purchase of them
int
GetPurchaseByName(DB_T d, const char* name)
{
  if (!d || !*name) return (-1);
  int hash_name = hash_func(name, d->buckets);
  struct UserInfo* p;
  for (p = d->pNameTab[hash_name].nextName; p; p=p->nextName){
    if (!strcmp(p->name, name)) return p->purchase;
  }
  fprintf(stderr, "Can't find user with such name\n");
  return (-1);
}
/*--------------------------------------------------------------------*/
// iterate through all nodes in all buckets
// pass them into function fp, add the return value to sum
// return sum
int
GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{ 
  if (!d||!fp) return (-1);
  int sum=0,i;
  struct UserInfo *p;
  for (i=0; i< d->buckets; i++){
    for (p= d->pIDTab[i].nextID; p; p= p->nextID){
      sum+= fp(p->id, p->name, p->purchase);
    }
  }
  return (sum);
}
