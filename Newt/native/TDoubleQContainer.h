//
//  TDoubleQContainer.h
//  Einstein
//
//  Created by Matthias Melcher on 3/3/15.
//
//

#ifndef _NEWT_TDOUBLEWCONTAINER_H_
#define _NEWT_TDOUBLEWCONTAINER_H_


#include "Newt/native/types.h"

#include "SimGlue.h"


class TDoubleQContainer;


class TDoubleQItem
{
public:
	//	TDoubleQItem();
	
	NEWT_GET_SET_W(0x000, TDoubleQItem*, Next);
	TDoubleQItem	*pNext;							///< 000
	NEWT_GET_SET_W(0x004, TDoubleQItem*, Prev);
	TDoubleQItem	*pPrev;							///< 004
	NEWT_GET_SET_W(0x008, TDoubleQContainer*, Container);
	TDoubleQContainer *pContainer;					///< 008
};


typedef void (*DestructorProcPtr)(void *, void *);		// instance pointer, data to destroy


class TDoubleQContainer
{
public:
//	TDoubleQContainer();
//	TDoubleQContainer(ULong inOffsetToDoubleQItem);
//	TDoubleQContainer(ULong inOffsetToDoubleQItem, DestructorProcPtr, void*);
//	
//	void		add(void * inItem);
//	void		addBefore(void * inItem, void * inBeforeItem);
//	void		addToFront(void * inItem);
//	void		checkBeforeAdd(void * inItem);
//	
	void			*Peek();
//	void *	getNext(void * inItem);
//	
	void			*Remove();						///< Remove the first item in the Queue
//	BOOL		removeFromQueue(void * inItem);
//	BOOL		deleteFromQueue(void * inItem);
//	
//	void		init(ULong inOffsetToDoubleQItem);
	
	NEWT_GET_SET_W(0x000, TDoubleQItem*, Head);
	TDoubleQItem	*pHead;							///< 000
	NEWT_GET_SET_W(0x004, TDoubleQItem*, Tail);
	TDoubleQItem	*pTail;							///< 004
	NEWT_GET_SET_W(0x008, ULong, OffsetToDoubleQItem);
	ULong			pOffsetToDoubleQItem;			///< 008
	NEWT_GET_SET_W(0x00C, DestructorProcPtr, Destructor);
	DestructorProcPtr pDestructor;					///< 00C
	NEWT_GET_SET_W(0x010, void*, DestructorInstance);
	void			*pDestructorInstance;			///< 010
};


extern void Func_0x0009C77C(TARMProcessor* ioCPU, KUInt32 ret); // Remove__17TDoubleQContainerFv
extern void Func_0x0009C884(TARMProcessor* ioCPU, KUInt32 ret); // Peek__17TDoubleQContainerFv


#endif
