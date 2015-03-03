//
//  TDoubleQContainer.cp
//  Einstein
//
//  Created by Matthias Melcher on 3/3/15.
//  Code and Code Excerpts with kind permission from "Newton Research Group"
//
//

#include "TDoubleQContainer.h"

#include "globals.h"

#include "TInterruptManager.h"
#include "TMemory.h"



#pragma mark - TDoubleQContainer


/**
 * Remove the first item in the Queue.
 * \return a pointer to the data of the first item
 */
void *TDoubleQContainer::Remove()
{
	TDoubleQItem *item = Head();
	if (item)
	{
		TDoubleQItem *next = item->Next();
		SetHead( next );
		if ( next )
			next->SetPrev( NULL );
		else
			SetTail( NULL );
		item->SetNext( NULL );
		item->SetPrev( NULL );
		item->SetContainer( NULL );
		return ((char*)item) - OffsetToDoubleQItem();
	}
	return NULL;
}


/**
 * Return the first item in the queue.
 * \return NULL if the queue is empty.
 */
void *TDoubleQContainer::Peek()
{
	TDoubleQItem *item = Head();
	if (item) {
		return ((char*)item) - OffsetToDoubleQItem();
	}
	return NULL;
}


#pragma mark - Stubs for all the functions above


/**
 * Peek__17TDoubleQContainerFv
 * ROM: 0x0009C884 - 0x0009C89C
 */
void Func_0x0009C884(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
		TDoubleQContainer *This = (TDoubleQContainer*)R0;
		R0 = (KUInt32)This->Peek();
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x0009C884, "Peek__17TDoubleQContainerFv", Func_0x0009C884)


/**
 * Remove__17TDoubleQContainerFv
 * ROM: 0x0009C77C - 0x0009C7C4
 */
void Func_0x0009C77C(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
		TDoubleQContainer *This = (TDoubleQContainer*)R0;
		R0 = (KUInt32)This->Remove();
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x0009C77C, "Remove__17TDoubleQContainerFv", Func_0x0009C77C)


