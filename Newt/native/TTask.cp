//
//  TTask.cp
//  Einstein
//
//  Created by Matthias Melcher on 2/22/14.
//  Code and Code Excerpts with kind permission from "Newton Research Group"
//
//

#include "TTask.h"

#include "globals.h"

#include "TInterruptManager.h"
#include "TMemory.h"

#ifdef IS_NEWT_SIM
#include <stdlib.h>
#endif



/**
 * Remove the first task from this queue.
 * \param inState flags to clear in the head task
 * \return the task that was removed or NULL if there were no tasks
 */
TTask* TTaskQueue::Remove(KernelObjectState inState)
{
	TTask *headTask = Head();
	if (headTask) {
		TTask *nextTask = headTask->TaskQItem().Next();
		SetHead(nextTask);
		if ( nextTask ) {
			nextTask->TaskQItem().SetPrev(0L);
		} else {
			SetTail(0L);
		}
		headTask->TaskQItem().SetPrev(0L);
		headTask->TaskQItem().SetNext(0L);
		headTask->SetState( headTask->State() & ~inState );
		headTask->SetContainer(0L);
	}
	return headTask;
}


/**
 * Check a task for something before we add it to the queue.
 * This does not do anything.
 */
void TTaskQueue::CheckBeforeAdd(TTask*)
{
	// Doesn't do anyting
}


/**
 * Tell the scheduler that we changed something in the task lists.
 */
void WantSchedule()
{
	if (GHoldScheduleLevel()==0) {
		SetGSchedule(1);
	} else {
		SetGScheduleRequested(1);
	}
}


/**
 * Add the give task to the end of this queue.
 * The task must not be part of this or another queue.
 */
void TTaskQueue::Add(TTask *inTask, KernelObjectState inState, TTaskContainer *inContainer)
{
	inTask->TaskQItem().SetNext( 0L );
	CheckBeforeAdd( inTask );
	
	TTask *headTask = Head();
	if ( headTask == 0L ) {
		SetHead(inTask);
		inTask->TaskQItem().SetPrev((TTask*)this); // This should probably be set to NULL
	} else {
		Tail()->TaskQItem().SetNext(inTask);
		inTask->TaskQItem().SetPrev(Tail());
	}
	SetTail(inTask);
	inTask->SetState( inTask->State() | inState );
	inTask->SetContainer( inContainer );
}


/**
 * Add a teask to the scheduler, updating priority flags.
 */
void TScheduler::Add(TTask *inTask)
{
	// update the priority mask of the scheduler
	int taskPriority = inTask->Priority();
	ULong priorityMask = PriorityMask();
	SetPriorityMask(priorityMask | (1<<taskPriority) );
	
	// add the task to the matching priority queue
	int maxPriority = HighestPriority();
	if ( taskPriority>maxPriority ) {
		SetHighestPriority( taskPriority );
	}
	TTaskQueue &priorityQueue = Task()[taskPriority];
	priorityQueue.Add(inTask, 0x00020000, (TTaskContainer*)this);
	
	// if we are ideling, tell the system that we want to schedule this task
	TTask *currentTask = GCurrentTask();
	TTask *idleTask = GIdleTask();
	if ( currentTask == idleTask ) {
		WantSchedule();
	}
	
	// reprioritize the current task priority, if needed
	int currentPriority = GTaskPriority();
	if ( taskPriority >= currentPriority ) {
		SetGWantSchedulerToRun(1);
	}
}


/**
 * Add a tesk to the scheduler if it is not the current task.
 */
void TScheduler::AddWhenNotCurrent(TTask *inTask)
{
	if ( inTask != GCurrentTask() )
		Add(inTask);
}


/**
 * Schedule a tesk for execution.
 */
void ScheduleTask(TTask *inTask)
{
	GKernelScheduler()->AddWhenNotCurrent(inTask);
}


/**
 * Wake all the tasks that wait for the semaphore to reach zero.
 */
void TSemaphore::WakeTasksOnZero()
{
	// Get the list of tasks to wake
	TTaskQueue &zeroTasks = ZeroTasks();
	
	// Is there a task in the list?
	TTask *nextTask = zeroTasks.Remove(0x00100000);
	
	// If not, we are done now. Bye.
	if (!nextTask)
		return;
	
	// If there is at least one task, add all in the list
	do {
		ScheduleTask(nextTask);
		nextTask = zeroTasks.Remove(0x00100000);
	}
	while (nextTask);
	
	// Make sure that they are excuted as at the next scheduler call
	WantSchedule();
}


/**
 * Wake all the tasks that wait for the semaphore to be incrmented.
 */
void TSemaphore::WakeTasksOnInc()
{
	// Get the list of tasks to wake
	TTaskQueue &incTasks = IncTasks();
	
	// Is there a task in the list?
	TTask *nextTask = incTasks.Remove(0x00100000);
	
	// If not, we are done now. Bye.
	if (!nextTask)
		return;
	
	// If there is at least one task, add all in the list
	do {
		ScheduleTask(nextTask);
		nextTask = incTasks.Remove(0x00100000);
	}
	while (nextTask);
	
	// Make sure that they are excuted as at the next scheduler call
	WantSchedule();
}


/**
 * Get the next task in this queue.
 * \returns the next task, NULL if there is none
 */
TTask *TTaskQueue::Peek()
{
	return Head();
}


/**
 * Recalculate priorities by priority map.
 */
void TScheduler::UpdateCurrentBucket()
{
	for (int i = HighestPriority()-1; i > 0; i--) {
		if (PriorityMask() & (1<<i)) {
			SetHighestPriority(i);
			return;
		}
	}
	SetHighestPriority(0);
}


/**
 * Remove a specific task from a queue.
 */
BOOL TTaskQueue::RemoveFromQueue(TTask *inTask, KernelObjectState inState)
{
	if ( (inTask==0L) || ((inTask->State() & inState)==0) )	{
		return 0;
	}
	
	TTask *nextTask = inTask->TaskQItem().Next();
	TTask *prevTask = inTask->TaskQItem().Prev();
	if ( inTask == Head() ) {
		if ( Head()!=Tail() ) {
			SetHead(nextTask);
			nextTask->TaskQItem().SetPrev(0L);
		} else {
			SetHead(0L);
			SetTail(0L);
		}
	} else {
		prevTask->TaskQItem().SetNext(nextTask);
		if ( inTask != Tail() ) {
			nextTask->TaskQItem().SetPrev(prevTask);
		} else {
			SetTail(prevTask);
		}
	}
	
	inTask->TaskQItem().SetNext(0L);
	inTask->TaskQItem().SetPrev(0L);
	inTask->SetState( inTask->State() & ~inState );
	inTask->SetContainer(0L);
	
	return 1;
}


void TScheduler::Remove(TTask *inTask)
{
	if ( inTask ) {
		if ( inTask == GCurrentTask() ) {
			SetGCurrentTask(0L);
			WantSchedule();
			SetGWantSchedulerToRun(1);
		} else {
			int priority = inTask->Priority();
			Task()[priority].RemoveFromQueue(inTask, 0x00020000);
			if ( Task()[priority].Peek() == 0L )
			{
				SetPriorityMask( PriorityMask() & ~(1<<priority) );
				if ( priority == HighestPriority() )
					UpdateCurrentBucket();
			}
		}
	}
}


void UnScheduleTask(TTask *inTask)
{
	TScheduler *kernelScheduler = GKernelScheduler();
	kernelScheduler->Remove(inTask); // FIXME: a virtual function
}


void TSemaphore::BlockOnInc(TTask *inTask, SemFlags inFlags)
{
	if ( (inFlags&kNoWaitOnBlock)==0 ) {
		UnScheduleTask(inTask);
		IncTasks().Add(inTask, 0x00100000, (TTaskContainer*)this);
	}
}


void TSemaphore::BlockOnZero(TTask *inTask, SemFlags inFlags)
{
	if ( (inFlags&kNoWaitOnBlock)==0 ) {
		UnScheduleTask(inTask);
		ZeroTasks().Add(inTask, 0x00100000, (TTaskContainer*)this);
	}
}


void TSemaphoreGroup::UnwindOp(TSemaphoreOpList *inList, long index)
{
	while (index>0)
	{
		--index;
		
		class SemOp &semOp = inList->OpList()[index];
		unsigned short ix = semOp.Num();
		signed short op = semOp.Op();
		if ( ix < Count() )
		{
			TSemaphore &sem = Group()[ix];
			sem.SetVal( sem.Val()-op );
		}
	}
}


NewtonErr TSemaphoreGroup::SemOp(TSemaphoreOpList *inList, SemFlags inBlocking, TTask *inTask)
{
	int			index;
	unsigned short semNum;
	short		semOper;
	
	for (index = 0; index < inList->Count(); index++)
	{
		semNum  = inList->OpList()[index].Num();
		semOper = inList->OpList()[index].Op();
		if (semNum < Count())
		{
			TSemaphore &sem = Group()[semNum];
			if (semOper == 0)
			{
				if (sem.Val() != 0)
				{
					Group()[semNum].BlockOnZero(inTask, inBlocking);
					UnwindOp(inList, index);
					return -10000-25; // kOSErrSemaphoreWouldCauseBlock;
				}
			}
			else if (semOper > 0 || sem.Val() >= -semOper)	// - = abs
				sem.SetVal( sem.Val() + semOper );
			else
			{
				Group()[semNum].BlockOnInc(inTask, inBlocking);
				UnwindOp(inList, index);
				return -10000-25; // kOSErrSemaphoreWouldCauseBlock;
			}
		}
	}
	
	// now all the semaphores have been updated, see if any tasks can be unblocked
	for (index = 0; index < inList->Count(); index++)
	{
		semNum  = inList->OpList()[index].Num();
		semOper = inList->OpList()[index].Op();
		if (semNum <= Count())
		{
			TSemaphore &sem = Group()[semNum];
			if (semOper > 0)
				sem.WakeTasksOnInc();
			else if (semOper == 0 && sem.Val() == 0)
				sem.WakeTasksOnZero();
		}
	}
	
	return noErr;
}


TObject *TObjectTable::Get(ObjectId inId)
{
	TObject* obj = Entry( (inId>>4) & kObjectTableMask );
	while (obj) {
		if ( obj->Id() == inId ) {
			if ( obj->Id()==0 )
				return NULL;
			return obj;
		}
		obj = obj->Next();
	}
	return NULL;
}


NewtonErr DoSemaphoreOp(ObjectId inGroupId, ObjectId inListId, SemFlags inBlocking, TTask *inTask)
{
	TSemaphoreGroup* semGroup = NULL;
	if ( (inGroupId & 0x0f) == 7 )
		semGroup = (TSemaphoreGroup*)GObjectTable()->Get(inGroupId);
	if (!semGroup)
		return -10022; // kOSErrBadSemaphoreGroupId
	
	TSemaphoreOpList* opList = NULL;
	if ( (inListId & 0x0f) == 6 )
		opList = (TSemaphoreOpList*)GObjectTable()->Get(inListId);
	
	if (opList==NULL)
		return -10023; // kOSErrBadSemaphoreOpListId
	
	return semGroup->SemOp(opList, inBlocking, inTask);
}


// ScreenUpdateTask__FPvUlT2
//  SemOp__16TUSemaphoreGroupFP17TUSemaphoreOpList8SemFlags
//   _SemaphoreOpGlue
//    swi 0x0000000B (COMPLEX!) calling DoSemaphoreOp at 0x003ADEF8
//     003AD698-003AD74C SWIBoot
//     Jump table dispatch from 003AD568
//     003ADEE4
//      DoSemaphoreOp
//     At 003ADF14 (or later if Semaphore is blocking), jump to 003AD750 (task switch)
//      DoDeferrals
//       _EnterAtomicFast (leaf)
//       _ExitAtomicFast (laef)
//       PortDeferredSendNotify__Fv ?
//       DeferredNotify__Fv ?
//       DoDeferral__18TExtPageTrackerMgrFv ?
//        Peek__17TDoubleQContainerFv ?
//        DoDeferral__15TExtPageTrackerFv ?
//         RemovePMappings__FUlT1 ?
//          IsSuperMode ?
//          PrimRemovePMappings__FUlT1 ?
//          _GenericSWI ? (moan!)
//         Remove__12TObjectTableFUl (leaf)
//        GetNext__17TDoubleQContainerFPv (leaf)
//      Scheduler ?
//      SwapInGlobals ?
//      StartScheduler ?
//     exit to task at 003ADB10 (or wherever task switching leads us)

// bl      VEC_SemOp__16TUSemaphoreGroupFP17TUSemaphoreOpList8SemFlags  @ 0x001CD160 0xEB6836F5 - .h6.
// bl      VEC_SemOp__16TUSemaphoreGroupFP17TUSemaphoreOpList8SemFlags  @ 0x001CD194 0xEB6836E8 - .h6.
// SemOp__16TUSemaphoreGroupFP17TUSemaphoreOpList8SemFlags: 0x0025A464-0x0025A470
// SWIBoot: 0x003AD698-0x003ADBB4
// SWI11_SemOp: 0x003ADEE4-0x003ADFAC

/*
SWI11_SemOp:
        @ label = 'SWI11_SemOp'
        ldmia   sp, {r0, r1}                    @ 0x003ADEE4 0xE89D0003 - ....
        ldr     r3, L003ADEB8                   @ [ gCurrentTask (0x0C100FF8) ] 0x003ADEE8 0xE51F3038 - ..08
        ldr     r3, [r3]                        @ 0x003ADEEC 0xE5933000 - ..0.
 ;; get the current task and remember it for later
        stmdb   sp!, {r2, r3}                   @ 0x003ADEF0 0xE92D000C - .-.. 
        stmdb   sp!, {r10-r12, lr}              @ 0x003ADEF4 0xE92D5C00 - .-\. 
 ;; call the semaphore operation according to the given Id's
        bl      VEC_DoSemaphoreOp               @ 0x003ADEF8 0xEB5D2728 - .]'(
        ldmia   sp!, {r10-r12, lr}              @ 0x003ADEFC 0xE8BD5C00 - ..\. 
        ldmia   sp!, {r2, r3}                   @ 0x003ADF00 0xE8BD000C - .... 
        ldr     r1, L003ADEB8                   @ [ gCurrentTask (0x0C100FF8) ] 0x003ADF04 0xE51F1054 - ...T
        ldr     r1, [r1]                        @ 0x003ADF08 0xE5911000 - ....
        cmp     r1, #0                          @ [ 0x00000000 ] 0x003ADF0C 0xE3510000 - .Q.. 
 ;; if the current task was not blocked, continue with the usual cooperative task switching
        addne   sp, sp, #8                      @ [ 0x00000008 ] 0x003ADF10 0x128DD008 - .... 
        bne     L003AD750                       @ 0x003ADF14 0x1AFFFE0D - ....
 ;; continue here if there is no longer a current task
 ;; this happens, if the semaphore blocked the current task
        sub     lr, lr, #4                      @ [ 0x00000004 ] 0x003ADF18 0xE24EE004 - .N.. 
 ;; adjust the lr? Is that the return address? If so, do we decrement it to make up for the pipeline?
        mov     r0, r3                          @ 0x003ADF1C 0xE1A00003 - ....
        mrs     r1, spsr                        @ 0x003ADF20 0xE14F1000 - .O..
        str     r1, [r0, #80]                   @ 0x003ADF24 0xE5801050 - ...P 
        and     r1, r1, #31                     @ [ 0x0000001F ] 0x003ADF28 0xE201101F - .... 
        cmp     r1, #27                         @ [ 0x0000001B ] 0x003ADF2C 0xE351001B - .Q.. 
        add     r1, r0, #24                     @ [ 0x00000018 ] 0x003ADF30 0xE2801018 - .... 
        stmdb   sp!, {r0}                       @ 0x003ADF34 0xE92D0001 - .-.. 
        mrs     r0, cpsr                        @ 0x003ADF38 0xE10F0000 - ....
;; figure out the callers CPU mode and save the registers in the previously current TTask class
        msr     cpsr_ctl, #211                  @ [ 0x000000D3 ] 0x003ADF3C 0xE321F0D3 - .!.. 
        nop                                     @ 0x003ADF40 0xE1A00000 - ....
        nop                                     @ 0x003ADF44 0xE1A00000 - ....
        stmneia r1!, {r2-r7}                    @ 0x003ADF48 0x18A100FC - .... 
        stmneia r1, {r8-lr}^                    @ 0x003ADF4C 0x18C17F00 - ..^?.
        nop                                     @ 0x003ADF50 0xE1A00000 - ....
        msr     cpsr, r0                        @ 0x003ADF54 0xE129F000 - .)..
        nop                                     @ 0x003ADF58 0xE1A00000 - ....
        nop                                     @ 0x003ADF5C 0xE1A00000 - ....
        ldmia   sp!, {r0}                       @ 0x003ADF60 0xE8BD0001 - .... 
        bne     L003ADF98                       @ 0x003ADF64 0x1A00000B - ....
        stmia   r1!, {r2-r12}                   @ 0x003ADF68 0xE8A11FFC - .... 
        nop                                     @ 0x003ADF6C 0xE1A00000 - ....
        mrs     r4, cpsr                        @ 0x003ADF70 0xE10F4000 - ..@.
        mrs     r5, spsr                        @ 0x003ADF74 0xE14F5000 - .OP.
        msr     cpsr, r5                        @ 0x003ADF78 0xE129F005 - .)..
        nop                                     @ 0x003ADF7C 0xE1A00000 - ....
        nop                                     @ 0x003ADF80 0xE1A00000 - ....
        stmia   r1, {sp, lr}                    @ 0x003ADF84 0xE8816000 - ..`.
        nop                                     @ 0x003ADF88 0xE1A00000 - ....
        msr     cpsr, r4                        @ 0x003ADF8C 0xE129F004 - .)..
        nop                                     @ 0x003ADF90 0xE1A00000 - ....
        nop                                     @ 0x003ADF94 0xE1A00000 - ....
L003ADF98:
        add     r1, r0, #16                     @ [ 0x00000010 ] 0x003ADF98 0xE2801010 - .... 
        str     lr, [r1, #60]                   @ 0x003ADF9C 0xE581E03C - ...< 
        ldmia   sp!, {r2, r3}                   @ 0x003ADFA0 0xE8BD000C - .... 
        stmia   r1, {r2, r3}                    @ 0x003ADFA4 0xE881000C - ....
 ;; jump to the task switcher (currentTask is NULL, previously current task has all registers saved)
        b       L003AD750                       @ 0x003ADFA8 0xEAFFFDE8 - ....
*/

T_ROM_PATCH(0x003ADEF8, "SWI Call to DoSemaphoreOp")
{
	R0 = (KUInt32)DoSemaphoreOp((ObjectId)(R0), (ObjectId)(R1), (SemFlags)(R2), (TTask*)(R3));
//	PC = PC + 4;
//	return ioUnit;
	CALLNEXTUNIT;
}


/**
 * Transcoded function Func_0x003ADEE4
 * ROM: 0x003ADEE4 - 0x003ADFAC
 */
T_ROM_PATCH(0x003ADEE4, "SWI_SemOp")
//void Func_0x003ADEE4(TARMProcessor* ioCPU, KUInt32 ret)
{
	// 003ADEE4: E89D0003  ldmea	r13, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		SETPC(0x003ADEE4+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
	}
	
	// 003ADEE8: E51F3038  ldr	r3, 003ADEB8=SWIBoot+820
	{
		KUInt32 offset = 0x00000038;
		KUInt32 theAddress = 0x003ADEE8 + 8 - offset;
		SETPC(0x003ADEE8+8);
		KUInt32 theData = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, theAddress);
		R3 = theData;
	}
	
	// 003ADEEC: E5933000  ldr	r3, [r3]
	SETPC(0x003ADEEC+8);
	R3 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R3);
	
	// 003ADEF0: E92D000C  stmfd	r13!, {r2-r3}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003ADEF0+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADEF4: E92D5C00  stmfd	r13!, {r10-r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (4 * 4);
		baseAddress -= (4 * 4);
		SETPC(0x003ADEF4+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
		SP = wbAddress;
	}
	
	R0 = (KUInt32)DoSemaphoreOp((ObjectId)(R0), (ObjectId)(R1), (SemFlags)(R2), (TTask*)(R3));
	
	// 003ADEFC: E8BD5C00  ldmea	r13!, {r10-r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (4 * 4);
		SETPC(0x003ADEFC+8);
		R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		LR = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADF00: E8BD000C  ldmea	r13!, {r2-r3}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003ADF00+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADF04: E51F1054  ldr	r1, 003ADEB8=SWIBoot+820
	{
		KUInt32 offset = 0x00000054;
		KUInt32 theAddress = 0x003ADF04 + 8 - offset;
		SETPC(0x003ADF04+8);
		KUInt32 theData = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, theAddress);
		R1 = theData;
	}
	
	// 003ADF08: E5911000  ldr	r1, [r1]
	SETPC(0x003ADF08+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003ADF0C: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADF10: 128DD008  addne	r13, r13, #0x00000008
	if (ioCPU->TestNE()) {
		SP = SP + 0x00000008; // 8
	}
	
	// 003ADF14: 1AFFFE0D  bne	003AD750=SWIBoot+B8
	if (ioCPU->TestNE()) {
		// rt cjitr 003AD750
		SETPC(0x003AD750+4);
		MMUCALLNEXT_AFTERSETPC
	}
	
	// 003ADF18: E24EE004  sub	lr, lr, #0x00000004
	LR = LR - 0x00000004; // 4
	
	// 003ADF1C: E1A00003  mov	r0, r3
	R0 = R3;
	
	// 003ADF20: E14F1000  mrs	r1, spsr
	R1 = ioCPU->GetSPSR();
	
	// 003ADF24: E5801050  str	r1, [r0, #0x050]
	SETPC(0x003ADF24+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R0 + 0x0050, R1);
	
	// 003ADF28: E201101F  and	r1, r1, #0x0000001f
	R1 = R1 & 0x0000001F;
	
	// 003ADF2C: E351001B  cmp	r1, #0x0000001b
	{
		KUInt32 Opnd2 = 0x0000001B;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADF30: E2801018  add	r1, r0, #0x00000018
	R1 = R0 + 0x00000018; // 24
	
	// 003ADF34: E92D0001  stmfd	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (1 * 4);
		baseAddress -= (1 * 4);
		SETPC(0x003ADF34+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADF38: E10F0000  mrs	r0, cpsr
	R0 = ioCPU->GetCPSR();
	
	// 003ADF3C: E321F0D3  msr	cpsr_c, #0x000000d3
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x000000D3 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x000000D3);
		}
	}
	
	// 003ADF40: E1A00000  mov	r0, r0
	
	// 003ADF44: E1A00000  mov	r0, r0
	
	// 003ADF48: 18A100FC  stmneia	r1!, {r2-r7}
	if (ioCPU->TestNE()) {
		KUInt32 baseAddress = R1;
		KUInt32 wbAddress = baseAddress + (6 * 4);
		SETPC(0x003ADF48+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
		R1 = wbAddress;
	}
	
	// 003ADF4C: 18C17F00  stmneia	r1, {r8-lr}^
	if (ioCPU->TestNE()) {
		KUInt32 baseAddress = R1;
		SETPC(0x003ADF4C+8);
		if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R8); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R9); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
		} else {
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR8_Bkup); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR9_Bkup); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR10_Bkup); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR11_Bkup); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR12_Bkup); baseAddress += 4;
		}
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR13_Bkup); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR14_Bkup); baseAddress += 4;
	}
	
	// 003ADF50: E1A00000  mov	r0, r0
	
	// 003ADF54: E129F000  msr	cpsr_cf, r0
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R0 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R0);
		}
	}
	
	// 003ADF58: E1A00000  mov	r0, r0
	
	// 003ADF5C: E1A00000  mov	r0, r0
	
	// 003ADF60: E8BD0001  ldmea	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (1 * 4);
		SETPC(0x003ADF60+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADF64: 1A00000B  bne	003ADF98=SWIBoot+900
	if (ioCPU->TestNE()) {
		SETPC(0x003ADF98+4);
		goto L003ADF98;
	}
	
	// 003ADF68: E8A11FFC  stmia	r1!, {r2-r12}
	{
		KUInt32 baseAddress = R1;
		KUInt32 wbAddress = baseAddress + (11 * 4);
		SETPC(0x003ADF68+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R8); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R9); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
		R1 = wbAddress;
	}
	
	// 003ADF6C: E1A00000  mov	r0, r0
	
	// 003ADF70: E10F4000  mrs	r4, cpsr
	R4 = ioCPU->GetCPSR();
	
	// 003ADF74: E14F5000  mrs	r5, spsr
	R5 = ioCPU->GetSPSR();
	
	// 003ADF78: E129F005  msr	cpsr_cf, r5
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R5 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R5);
		}
	}
	
	// 003ADF7C: E1A00000  mov	r0, r0
	
	// 003ADF80: E1A00000  mov	r0, r0
	
	// 003ADF84: E8816000  stmia	r1, {r13-lr}
	{
		KUInt32 baseAddress = R1;
		SETPC(0x003ADF84+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, SP); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
	}
	
	// 003ADF88: E1A00000  mov	r0, r0
	
	// 003ADF8C: E129F004  msr	cpsr_cf, r4
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R4 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R4);
		}
	}
	
	// 003ADF90: E1A00000  mov	r0, r0
	
	// 003ADF94: E1A00000  mov	r0, r0
	
L003ADF98:
	// 003ADF98: E2801010  add	r1, r0, #0x00000010
	R1 = R0 + 0x00000010; // 16
	
	// 003ADF9C: E581E03C  str	lr, [r1, #0x03c]
	SETPC(0x003ADF9C+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x003C, LR);
	
	// 003ADFA0: E8BD000C  ldmea	r13!, {r2-r3}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003ADFA0+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADFA4: E881000C  stmia	r1, {r2-r3}
	{
		KUInt32 baseAddress = R1;
		SETPC(0x003ADFA4+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
	}
	
	// 003ADFA8: EAFFFDE8  b	003AD750=SWIBoot+B8
	// rt cjitr 003AD750
	SETPC(0x003AD750+4);
	MMUCALLNEXT_AFTERSETPC
}
//T_ROM_SIMULATION3(0x003ADEE4, "Func_0x003ADEE4", Func_0x003ADEE4)


/**
 * DoSemaphoreOp
 * ROM: 0x001D4CE4 - 0x001D4D98
 */
void Func_0x001D4CE4(TARMProcessor* ioCPU, KUInt32 ret)
{
	ObjectId inGroupId = (ObjectId)(R0);
	ObjectId inListId = (ObjectId)(R1);
	SemFlags inBlocking = (SemFlags)(R2);
	TTask *inTask = (TTask*)(R3);
	R0 = (KUInt32)DoSemaphoreOp(inGroupId, inListId, inBlocking, inTask);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001D4CE4, "DoSemaphoreOp", Func_0x001D4CE4)

/**
 * Get__12TObjectTableFUl
 * ROM: 0x00319F14 - 0x00319F60
 */
void Func_0x00319F14(TARMProcessor* ioCPU, KUInt32 ret)
{
	TObjectTable *This = (TObjectTable*)(R0);
	ObjectId inId = (ObjectId)(R1);
	R0 = (KUInt32)This->Get(inId);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00319F14, "Get__12TObjectTableFUl", Func_0x00319F14)

/**
 * SemOp__15TSemaphoreGroupFP16TSemaphoreOpList8SemFlagsP5TTask
 * ROM: 0x001D4F38 - 0x001D5078
 */
void Func_0x001D4F38(TARMProcessor* ioCPU, KUInt32 ret)
{
	TSemaphoreGroup *This = (TSemaphoreGroup*)(R0);
	TSemaphoreOpList *inList = (TSemaphoreOpList*)(R1);
	SemFlags inFlags = (SemFlags)(R2);
	TTask *inTask = (TTask*)(R3);
	R0 = (KUInt32)This->SemOp(inList, inFlags, inTask);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001D4F38, "SemOp__15TSemaphoreGroupFP16TSemaphoreOpList8SemFlagsP5TTask", Func_0x001D4F38)

/**
 * UnWindOp__15TSemaphoreGroupFP16TSemaphoreOpListl
 * ROM: 0x001D4EE8 - 0x001D4F38
 */
void Func_0x001D4EE8(TARMProcessor* ioCPU, KUInt32 ret)
{
	TSemaphoreGroup *This = (TSemaphoreGroup*)(R0);
	TSemaphoreOpList *inList = (TSemaphoreOpList*)(R1);
	long index = (long)(R2);
	This->UnwindOp(inList, index);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001D4EE8, "UnWindOp__15TSemaphoreGroupFP16TSemaphoreOpListl", Func_0x001D4EE8)

/**
 * BlockOnZero__10TSemaphoreFP5TTask8SemFlags
 * ROM: 0x001D5264 - 0x001D52A0
 */
void Func_0x001D5264(TARMProcessor* ioCPU, KUInt32 ret)
{
	TSemaphore *This = (TSemaphore*)(R0);
	TTask *inTask = (TTask*)(R1);
	SemFlags inFlags = (SemFlags)(R2);
	This->BlockOnZero(inTask, inFlags);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001D5264, "BlockOnZero__10TSemaphoreFP5TTask8SemFlags", Func_0x001D5264)

/**
 * BlockOnInc__10TSemaphoreFP5TTask8SemFlags
 * ROM: 0x001D4D98 - 0x001D4DD4
 */
void Func_0x001D4D98(TARMProcessor* ioCPU, KUInt32 ret)
{
	TSemaphore *This = (TSemaphore*)(R0);
	TTask *inTask = (TTask*)(R1);
	SemFlags inFlags = (SemFlags)(R2);
	This->BlockOnInc(inTask, inFlags);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001D4D98, "BlockOnInc__10TSemaphoreFP5TTask8SemFlags", Func_0x001D4D98)

/**
 * Transcoded function UnScheduleTask__FP5TTask
 * ROM: 0x001918FC - 0x00191914
 */
void Func_0x001918FC(TARMProcessor* ioCPU, KUInt32 ret)
{
	UnScheduleTask((TTask*)(R0));
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001918FC, "UnScheduleTask__FP5TTask", Func_0x001918FC)

/**
 * Remove__10TSchedulerFP5TTask
 * ROM: 0x001CC5F8 - 0x001CC690
 */
void Func_0x001CC5F8(TARMProcessor* ioCPU, KUInt32 ret)
{
	TScheduler *This = (TScheduler*)(R0);
	TTask *inTask = (TTask*)(R1);
	This->Remove(inTask);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001CC5F8, "Remove__10TSchedulerFP5TTask", Func_0x001CC5F8)

/**
 * RemoveFromQueue__10TTaskQueueFP5TTask17KernelObjectState
 * ROM: 0x00359B5C - 0x00359BE0
 */
void Func_0x00359B5C(TARMProcessor* ioCPU, KUInt32 ret)
{
	TTaskQueue *This = (TTaskQueue*)(R0);
	TTask *inTask = (TTask*)(R1);
	KernelObjectState inState = (KernelObjectState)(R2);
	R0 = (KUInt32)This->RemoveFromQueue(inTask, inState);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00359B5C, "RemoveFromQueue__10TTaskQueueFP5TTask17KernelObjectState", Func_0x00359B5C)

/**
 * UpdateCurrentBucket__10TSchedulerFv
 * ROM: 0x001CC1B0 - 0x001CC1EC
 */
void Func_0x001CC1B0(TARMProcessor* ioCPU, KUInt32 ret)
{
	TScheduler *This = (TScheduler*)(R0);
	This->UpdateCurrentBucket();
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001CC1B0, "UpdateCurrentBucket__10TSchedulerFv", Func_0x001CC1B0)

/**
 * Peek__10TTaskQueueFv
 * ROM: 0x00359BE0 - 0x00359BE8
 */
void Func_0x00359BE0(TARMProcessor* ioCPU, KUInt32 ret)
{
	TTaskQueue *This = (TTaskQueue*)(R0);
	R0 = (KUInt32)This->Peek();
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00359BE0, "Peek__10TTaskQueueFv", Func_0x00359BE0)

/**
 * WakeTasksOnZero__10TSemaphoreFv
 * ROM: 0x001D4DD4
 */
void Func_0x001D4DD4(TARMProcessor* ioCPU, KUInt32 ret)
{
	TSemaphore *This = (TSemaphore*)(R0);
	This->WakeTasksOnZero();
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001D4DD4, "WakeTasksOnZero__10TSemaphoreFv", Func_0x001D4DD4)

/**
 * WakeTasksOnInc__10TSemaphoreFv
 * ROM: 0x001D4E18 - 0x001D4E5C
 */
void Func_0x001D4E18(TARMProcessor* ioCPU, KUInt32 ret)
{
	TSemaphore *This = (TSemaphore*)(R0);
	This->WakeTasksOnInc();
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001D4E18, "WakeTasksOnInc__10TSemaphoreFv", Func_0x001D4E18)

/**
 * ScheduleTask__FP5TTask
 * ROM: 0x001918E8 - 0x001918FC
 */
void Func_0x001918E8(TARMProcessor* ioCPU, KUInt32 ret)
{
	TTask *inTask = (TTask*)(R0);
	ScheduleTask(inTask);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001918E8, "ScheduleTask__FP5TTask", Func_0x001918E8)

/**
 * AddWhenNotCurrent__10TSchedulerFP5TTask
 * ROM: 0x001CC5E0 - 0x001CC5F8
 */
void Func_0x001CC5E0(TARMProcessor* ioCPU, KUInt32 ret)
{
	TScheduler *This = (TScheduler*)(R0);
	TTask *inTask = (TTask*)(R1);
	This->AddWhenNotCurrent(inTask);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001CC5E0, "AddWhenNotCurrent__10TSchedulerFP5TTask", Func_0x001CC5E0)

/**
 * Add__10TSchedulerFP5TTask
 * ROM: 0x001CC564 - 0x001CC5E0
 */
void Func_0x001CC564(TARMProcessor* ioCPU, KUInt32 ret)
{
	TScheduler *This = (TScheduler*)(R0);
	TTask *inTask = (TTask*)(R1);
	This->Add(inTask);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001CC564, "Add__10TSchedulerFP5TTask", Func_0x001CC564)

/**
 * Add__10TTaskQueueFP5TTask17KernelObjectStateP14TTaskContainer
 * ROM: 0x00359AAC - 0x00359B14
 */
void Func_0x00359AAC(TARMProcessor* ioCPU, KUInt32 ret)
{
	TTaskQueue *This = (TTaskQueue*)(R0);
	TTask *inTask = (TTask*)(R1);
	KernelObjectState inState = (KernelObjectState)(R2);
	TTaskContainer *inContainer = (TTaskContainer*)(R3);
	This->Add(inTask, inState, inContainer);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00359AAC, "Add__10TTaskQueueFP5TTask17KernelObjectStateP14TTaskContainer", Func_0x00359AAC)

/**
 * CheckBeforeAdd__10TTaskQueueFP5TTask
 * ROM: 0x00359AA8 - 0x00359AAC
 */
void Func_0x00359AA8(TARMProcessor* ioCPU, KUInt32 ret)
{
	TTaskQueue *This = (TTaskQueue*)(R0);
	TTask *inTask = (TTask*)(R1);
	This->CheckBeforeAdd(inTask);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00359AA8, "CheckBeforeAdd__10TTaskQueueFP5TTask", Func_0x00359AA8)

/**
 * WantSchedule__Fv
 * ROM: 0x001CC7F4 - 0x001CC820
 */
void Func_0x001CC7F4(TARMProcessor* ioCPU, KUInt32 ret)
{
	WantSchedule();
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x001CC7F4, "WantSchedule__Fv", Func_0x001CC7F4)

/**
 * Remove__10TTaskQueueF17KernelObjectState
 * ROM: 0x00359B14 - 0x00359B5C
 */
void Func_0x00359B14(TARMProcessor* ioCPU, KUInt32 ret)
{
	TTaskQueue *This = (TTaskQueue*)(R0);
	KernelObjectState inState = (KernelObjectState)(R1);
	R0 = (KUInt32)This->Remove(inState);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00359B14, "Remove__10TTaskQueueF17KernelObjectState", Func_0x00359B14)
