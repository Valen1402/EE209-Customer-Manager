#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "customer_manager.h"

#define UNIT_ARRAY_SIZE 64

struct UserInfo {
  char *name;                // customer name
  char *id;                  // customer id
  int purchase;              // purchase amount (> 0)
};

struct DB {
  struct UserInfo *pArray;   // pointer to the array
  int curArrSize;            // current array size (max # of elements)
  int numItems;              // # of stored items, needed to determine
			     // # whether the array should be expanded
			     // # or not
};

DB_T
CreateCustomerDB(void)
{
  DB_T d;
  
  d = (DB_T) calloc(1, sizeof(struct DB));
  if (d == NULL) {
    fprintf(stderr, "Can't allocate a memory for DB_T\n");
    return NULL;
  }
  d->curArrSize = UNIT_ARRAY_SIZE; // start with 64 elements
  d->pArray = (struct UserInfo *)
    calloc(d->curArrSize, sizeof(struct UserInfo));
    
  if (d->pArray == NULL) {
    fprintf(stderr, "Can't allocate a memory for array of size %d\n",
	    d->curArrSize);   
    free(d);
    return NULL;
  }
  return d;
  
}
/*--------------------------------------------------------------------*/
// delete d with all registered memory
void
DestroyCustomerDB(DB_T d)
{ 
  if (d) {
    free(d->pArray);
    free(d);
  }
  return;
}
/*--------------------------------------------------------------------*/
// iterate through pArray, check for id and name duplication
// check if pArray is full, then expand it using realloc
// register new customer into the first empty node encountered
int
RegisterCustomer(DB_T d, const char *id,
		 const char *name, const int purchase)
{
  if( !d || !*name || !*id || purchase <=0) return (-1);
  int i;
  int iempty = 0;
  
  /*check for id and name duplication*/
  for (i=0; i< d->curArrSize; i++){
    if (!d->pArray[i].name)
      iempty = i;
    else if(!strcmp(d->pArray[i].name, name) ||!strcmp(d->pArray[i].id, id))
    {
      fprintf(stderr, "Name/ID already exists\n");
      return (-1);
    }
  }
  /*if storage is used up, expand it*/
  if( d->numItems == d->curArrSize) {
    d->curArrSize+= UNIT_ARRAY_SIZE;
    if ((d->pArray = (struct UserInfo *) 
    	realloc(d->pArray, (d->curArrSize)*sizeof(struct UserInfo)) )
	== NULL){
      fprintf(stderr,"Can't expand the memory\n");
      DestroyCustomerDB(d);
      return (-1);
    }
    iempty = d->curArrSize-1;
  }

  /*store data in the empty node*/
  d->pArray[iempty].id = strdup(id);
  d->pArray[iempty].name = strdup(name);
  d->pArray[iempty].purchase = purchase;
  d->numItems ++;
  
  if(!(d->pArray[iempty].id) ||
       !(d->pArray[iempty].name)){
    fprintf(stderr, "Can't duplicate name/id\n");
    return (-1);
  }
  return 0;
}
/*--------------------------------------------------------------------*/
// iterate through the pArray
// if find matched id, delete such node
// return 0 on success
int
UnregisterCustomerByID(DB_T d, const char *id)
{
  int i;
  if (!d || !*id) return (-1);
  
  for (i=0; i< d->curArrSize; i++){
    if(!d->pArray[i].name) continue;
    if(!strcmp(d->pArray[i].id, id)){
      d->numItems--;
      //reset the deleted node
      free(d->pArray[i].id);
      free(d->pArray[i].name);
      d->pArray[i].purchase =0;
      return 0;
    }
  }
  fprintf(stderr, "ID does not exits\n");
  return (-1);
}

/*--------------------------------------------------------------------*/
// iterate through the pArray
// if find matched name, delete such node
// return 0 on success
int
UnregisterCustomerByName(DB_T d, const char *name)
{
  if (!d || !*name) return (-1);
  int i;
  for (i=0; i< d->curArrSize; i++){
    if(!d->pArray[i].name) continue;
    if(!strcmp(d->pArray[i].name, name)){
      d->numItems--;
      //reset the deleted node
      d->pArray[i].name = NULL;
      d->pArray[i].id = NULL;
      d->pArray[i].purchase =0;
      return 0;
    }
  }
  fprintf(stderr,"Name does not exist\n");
  return (-1);
}
/*--------------------------------------------------------------------*/
// iterate through the pArray
// if find matched id, return its purchase value
// return -1 on failure
int
GetPurchaseByID(DB_T d, const char* id)
{
  if (!d || !*id) return (-1);
  int i;
  for (i=0; i< d->curArrSize; i++){
    if(!d->pArray[i].name) continue;
    if(!strcmp(d->pArray[i].id, id)){
      return d->pArray[i].purchase;
    }
  }
  fprintf(stderr,"ID does not exist\n");
  return (-1);
}
/*--------------------------------------------------------------------*/
// iterate through the pArray
// if find matched name, return its purchase value
// return -1 on failure
int
GetPurchaseByName(DB_T d, const char* name)
{
  if (!d || !*name) return (-1);
  int i;
  for (i=0; i< d->curArrSize; i++){
    if(!d->pArray[i].name) continue;
    if(!strcmp(d->pArray[i].name, name)){
      return d->pArray[i].purchase;
    }
  }
  fprintf(stderr,"Name does not exist\n");
  return (-1);
}
/*--------------------------------------------------------------------*/
// pass every node id, name, purchase in to function fp
// add the return value to sum
// return sum
int
GetSumCustomerPurchase(DB_T d, FUNCPTR_T fp)
{
  if (!d || !fp) return (-1);
  int sum=0, i;
  for (i=0; i< d->curArrSize; i++){
    if(!d->pArray[i].name) continue;
    sum+= fp(d->pArray[i].id, d->pArray[i].name, d->pArray[i].purchase);
  }
  return sum;
}
