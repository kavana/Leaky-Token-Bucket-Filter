#ifndef _MY402LIST_H_
#define _MY402LIST_H_

#include "cs402.h"
#include "my402list.h"
#include <stdlib.h>
#include<stdio.h>

typedef struct tagMy402ListElem {
    void *obj;
    struct tagMy402ListElem *next;
    struct tagMy402ListElem *prev;
} My402ListElem;

typedef struct tagMy402List {
    int num_members;
    My402ListElem anchor;

    /* You do not have to set these function pointers */
    int  (*Length)(struct tagMy402List*);
    int  (*Empty)(struct tagMy402List *);

    int  (*Append)(struct tagMy402List *, void*);
    int  (*Prepend)(struct tagMy402List *, void*);
    void (*Unlink)(struct tagMy402List *, My402ListElem*);
    void (*UnlinkAll)(struct tagMy402List *);
    int  (*InsertBefore)(struct tagMy402List *, void*, My402ListElem*);
    int  (*InsertAfter)(struct tagMy402List *, void*, My402ListElem*);

    My402ListElem *(*First)(struct tagMy402List *);
    My402ListElem *(*Last)(struct tagMy402List *);
    My402ListElem *(*Next)(struct tagMy402List *, My402ListElem *cur);
    My402ListElem *(*Prev)(struct tagMy402List *, My402ListElem *cur);

    My402ListElem *(*Find)(struct tagMy402List *, void *obj);
} My402List;

int  My402ListLength(My402List* list )
 {
	 return list->num_members;
 }
 int  My402ListEmpty(My402List* list)
 {
	 if((My402ListLength(list)) == 0)
		 return TRUE;
	 else
		 return FALSE;
 }
 My402ListElem *My402ListFirst(My402List* list)
  {
 	 if((My402ListEmpty(list)) == 1)
 	 			return NULL;
 	 	 else
 	 			return list->anchor.next;

  }


  My402ListElem *My402ListLast(My402List* list)
  {
 	 if((My402ListEmpty(list)) == 1)
 			return NULL;
 	 else
 			return list->anchor.prev;
  }

 int  My402ListAppend(My402List* list, void* obj)
 {
	 My402ListElem *newElem =(My402ListElem *)malloc(sizeof(My402ListElem));
	 		 if(newElem == NULL)
	 		 {
	 			fprintf(stderr,"Failed to allocate memory");
	 			return FALSE;

	 		 }
	 if(My402ListEmpty(list) == 1)
	 {
		 list->anchor.next = newElem;
		 list->anchor.prev = newElem;
		 newElem->prev = &(list->anchor);
		 newElem->next = &(list->anchor);
		 newElem ->obj = obj;
		 list->num_members++;
	 }
	 else
	 {
		 My402ListElem *last = My402ListLast(list);
		 last->next = newElem;
		 newElem->prev = last;
		 newElem->next = &(list->anchor);
		 list->anchor.prev = newElem;
		 newElem ->obj = obj;
		 list->num_members++;
	 }

	return TRUE;
 }
 int  My402ListPrepend(My402List* list, void* obj)
 {
	 My402ListElem *newElem =(My402ListElem *)malloc(sizeof(My402ListElem));
	 			 if(newElem == NULL)
	 			 {
	 				fprintf(stderr,"Failed to allocate memory");
	 				return FALSE;
	 			 }
	 if(My402ListEmpty(list) == 1)
		 {
			 list->anchor.next = newElem;
			 list->anchor.prev = newElem;
			 newElem->prev = &(list->anchor);
			 newElem->next = &(list->anchor);
			 newElem ->obj = obj;
			 list->num_members++;
		 }
		 else
		 {
			 My402ListElem *first = My402ListFirst(list);
			 list->anchor.next = newElem;
			 newElem->prev = &(list->anchor);
			 newElem->next = first;
			 first->prev = newElem;
			 newElem ->obj = obj;
			 list->num_members++;
		 }

		return TRUE;
 }
 void My402ListUnlink(My402List* list, My402ListElem* elem)
 {
	 if(elem != &(list->anchor))
	 {
	 My402ListElem *prevElem = elem->prev;
	 My402ListElem *nextElem = elem->next;
	 prevElem->next = nextElem;
	 nextElem->prev = prevElem;
	 list->num_members--;
	 free(elem);
	 }

 }
 void My402ListUnlinkAll(My402List* list)
 {
	 My402ListElem *current = list->anchor.next;
	 My402ListElem *next;
	 while(current != NULL && current !=&( list->anchor))
	 {
		 next = current->next;
		 free(current);
		 current = next;
		 list->num_members--;
	 }
	 list -> anchor.prev = &(list->anchor);
	 list->anchor.next =&(list ->anchor);
 }
 int  My402ListInsertAfter(My402List* list, void* obj, My402ListElem* elem)
 {
	 if(elem == NULL)
		 return FALSE;
	 else
	 {
		 My402ListElem *newElem =(My402ListElem *)malloc(sizeof(My402ListElem));
		 if(newElem == NULL)
		 {
			 fprintf(stderr,"Failed to allocate memory");
			 return FALSE;
		 }
		 newElem ->next = elem->next;
		 elem->next->prev=newElem;
		 elem->next = newElem;
		 newElem->prev = elem;
		 newElem->obj =obj;
		 list->num_members++;

	 }
	 return TRUE;
 }

 int  My402ListInsertBefore(My402List* list, void* obj, My402ListElem* elem)
 {
	 if(elem == NULL)
	 		 return FALSE;
	 	 else
	 	 {
	 		 My402ListElem *newElem =(My402ListElem *)malloc(sizeof(My402ListElem));
	 		 if(newElem == NULL)
	 		 {
	 			 fprintf(stderr,"Failed to allocate memory");
	 			 return FALSE;
	 		 }
	 		 newElem ->prev = elem->prev;
	 		 elem->prev->next=newElem;
	 		 elem->prev = newElem;
	 		 newElem->next = elem;
	 		 newElem->obj =obj;
	 		 list->num_members++;

	 	 }
	 	 return TRUE;
 }


 My402ListElem *My402ListNext(My402List* list, My402ListElem* elem)
 {
	 if(My402ListEmpty(list) == 1)
		 return NULL;
	 else if(elem->next == &(list->anchor))
	 {
		 return NULL;
	 }
	 else
		 return elem->next;
 }
 My402ListElem *My402ListPrev(My402List* list, My402ListElem* elem)
 {
	 if(My402ListEmpty(list) == 1)
		 return NULL;
	 else if(elem->prev == &(list->anchor))
		 return NULL;
	 else
	 	 return elem->prev;
 }

 My402ListElem *My402ListFind(My402List* list, void* obj)
 {
	 int num=0;
	 My402ListElem *track = My402ListFirst(list);
	 while(num <= list->num_members && track != &(list->anchor) )
	 {
		if(track->obj == obj)
		{
			return track;
		}
		else
			track = track->next;
		    num++;
	 }
	 return NULL;
 }

 int My402ListInit(My402List* list)
 {
	 if(list != NULL)
	 {
	  	 list->anchor.next =&( list->anchor);
	 	 list ->anchor.prev = &(list->anchor);
	 	 list->num_members=0;
	 	 return TRUE;
	 }
	 return FALSE;
 }

#endif /*_MY402LIST_H_*/
