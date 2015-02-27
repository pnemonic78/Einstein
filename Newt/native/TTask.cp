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

#include "Newt/core/ScreenUpdateTask.h"

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
// _SemaphoreOpGlue
// SWIBoot: 0x003AD698-0x003ADBB4


/**
 * Transcoded function Func_0x003ADEE4
 * ROM: 0x003ADEE4 - 0x003ADFAC
 */
void SWI11_SemOp(TARMProcessor* ioCPU)
{
	// parameters come on the stack
	R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP);
	R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP+4);
	
	// get the currently running task
	TTask *task = GCurrentTask();
	
	// do the semaphore operation on it (all the code is native)
	R0 = (KUInt32)DoSemaphoreOp((ObjectId)(R0), (ObjectId)(R1), (SemFlags)(R2), task);
	
	// if there is still a current task, purge the arguments from the stack and return
	if ( GCurrentTask() ) {
		SP = SP + 8; // clean arguments from stack
		return;
	}
	
	LR = LR - 0x00000004;
	// TODO: in the following code, all registers are restored to the
	// values before calling DoSemaphoreOp. The above call adjusts the LR/PC
	// so that the same Semaphore Op is called again when this task is rescheduled.
	// In native mode, we will have to implement some kind of loop.
	
	ULong savedPSR   = ioCPU->GetSPSR();
	ULong currentPSR = ioCPU->GetCPSR();
	
	task->SetPSR( savedPSR );
	
	if ( (savedPSR&0x0000001F) != TARMProcessor::kUndefinedMode )
	{
		ioCPU->SetCPSR(0x000000D3);
		
		task->SetRegister( 2,  R2);
		task->SetRegister( 3,  R3);
		task->SetRegister( 4,  R4);
		task->SetRegister( 5,  R5);
		task->SetRegister( 6,  R6);
		task->SetRegister( 7,  R7);
		if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
			task->SetRegister( 8,  R8);
			task->SetRegister( 9,  R9);
			task->SetRegister(10, R10);
			task->SetRegister(11, R11);
			task->SetRegister(12, R12);
		} else {
			task->SetRegister( 9,  ioCPU->mR8_Bkup);
			task->SetRegister( 9,  ioCPU->mR9_Bkup);
			task->SetRegister(10, ioCPU->mR10_Bkup);
			task->SetRegister(11, ioCPU->mR11_Bkup);
			task->SetRegister(12, ioCPU->mR12_Bkup);
		}
		task->SetRegister(13, ioCPU->mR13_Bkup);
		task->SetRegister(14, ioCPU->mR14_Bkup);
		
		ioCPU->SetCPSR(currentPSR);
	}
	else
	{
		// untested code:
		task->SetRegister( 2,  R2);
		task->SetRegister( 3,  R3);
		task->SetRegister( 4,  R4);
		task->SetRegister( 5,  R5);
		task->SetRegister( 6,  R6);
		task->SetRegister( 7,  R7);
		task->SetRegister( 8,  R8);
		task->SetRegister( 9,  R9);
		task->SetRegister(10, R10);
		task->SetRegister(11, R11);
		task->SetRegister(12, R12);
		
		ioCPU->SetCPSR( savedPSR );
		task->SetRegister(13, SP);
		task->SetRegister(14, LR);
		ioCPU->SetCPSR( currentPSR );
	}
	
	task->SetRegister(15, LR);
	task->SetRegister(0, UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP));
	task->SetRegister(1, UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP+4));
	
	SP += 8;
	
	return;
}

T_ROM_PATCH(0x003ADEE4, "SWI_SemOp") {
	SWI11_SemOp(ioCPU);
	SETPC(0x003AD750+4);
	MMUCALLNEXT_AFTERSETPC
}


/**
 * Transcoded function SWIBoot
 * ROM: 0x003AD698 - 0x003ADBB4
 */
void Func_0x003AD698(TARMProcessor* ioCPU, KUInt32 ret, KUInt32 inSWI)
{
	// 003AD698: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003AD698+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
//	// 003AD69C: E51E0004  ldr	r0, [lr, -#0x004]
//	SETPC(0x003AD69C+8);
//	R0 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, LR - 0x0004);
	R0 = 0xEF000000 & inSWI;
	
//	// 003AD6A0: E200140F  and	r1, r0, #0x0f000000
//	R1 = R0 & 0x0F000000;
//	
//	// 003AD6A4: E331040F  teq	r1, #0x0f000000
//	{
//		KUInt32 Opnd2 = 0x0F000000;
//		KUInt32 Opnd1 = R1;
//		const KUInt32 theResult = Opnd1 ^ Opnd2;
//		ioCPU->mCPSR_Z = (theResult==0);
//		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
//		ioCPU->mCPSR_C = ((Opnd2&0x80000000)!=0);
//	}
//	
//	// 003AD6A8: 1A0001B0  bne	003ADD70=SWIBoot+6D8
//	if (ioCPU->TestNE()) {
//		// rt cjitr 003ADD70
//		SETPC(0x003ADD70+4);
//		return Func_0x003ADD70(ioCPU, ret);
//	}
	
//	// 003AD6AC: E200120F  and	r1, r0, #0xf0000000
//	R1 = R0 & 0xF0000000;
//	
//	// 003AD6B0: E331020E  teq	r1, #0xe0000000
//	{
//		KUInt32 Opnd2 = 0xE0000000;
//		KUInt32 Opnd1 = R1;
//		const KUInt32 theResult = Opnd1 ^ Opnd2;
//		ioCPU->mCPSR_Z = (theResult==0);
//		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
//		ioCPU->mCPSR_C = ((Opnd2&0x80000000)!=0);
//	}
//	
//	// 003AD6B4: 1A0001B0  bne	003ADD7C=SWIBoot+6E4
//	if (ioCPU->TestNE()) {
//		// rt cjitr 003ADD7C
//		SETPC(0x003ADD7C+4);
//		return Func_0x003ADD7C(ioCPU, ret);
//	}
	
	// 003AD6B8: E14F1000  mrs	r1, spsr
	R1 = ioCPU->GetSPSR();
	
	// 003AD6BC: E201101F  and	r1, r1, #0x0000001f
	R1 = R1 & 0x0000001F;
	
	// 003AD6C0: E3510010  cmp	r1, #0x00000010
	{
		KUInt32 Opnd2 = 0x00000010;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD6C4: 13510000  cmpne	r1, #0x00000000
	if (ioCPU->TestNE()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD6C8: 1AFFFFE4  bne	003AD660=DoSchedulerSWI+8
	if (ioCPU->TestNE()) {
		// rt cjitr 003AD660
		SETPC(0x003AD660+4);
		NEWT_ASSERT(0==1);
//		return Func_0x003AD660(ioCPU, ret);
	}
	
	// 003AD6CC: E3A01055  mov	r1, #0x00000055
	R1 = 0x00000055; // 85
	
	// 003AD6D0: E2811C55  add	r1, r1, #0x00005500
	R1 = R1 + 0x00005500; // 21760
	
	// 003AD6D4: E2811855  add	r1, r1, #0x00550000
	R1 = R1 + 0x00550000; // 5570560
	
	// 003AD6D8: E2811455  add	r1, r1, #0x55000000
	R1 = R1 + 0x55000000; // 1426063360
	
	// 003AD6DC: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003AD6DC+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD6E0: E1A00001  mov	r0, r1
	R0 = R1;
	
	// 003AD6E4: E59F17AC  ldr	r1, 003ADE98=SWIBoot+800
	R1 = 0x0C008400; // gParamBlockFromImage
	
	// 003AD6E8: E58100B0  str	r0, [r1, #0x0b0]
	SETPC(0x003AD6E8+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x00B0, R0);
	
	// 003AD6EC: E8BD0003  ldmea	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003AD6EC+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD6F0: EE031F13  mcr	p15, 0, r1, c3, c3, 0
	SETPC(0x003AD6F0+8);
	ioCPU->SystemCoprocRegisterTransfer(0xEE031F13);
	
	// 003AD6F4: E59F17A0  ldr	r1, 003ADE9C=SWIBoot+804
	R1 = 0x0C100E58; // gAtomicFIQNestCountFast
	
	// 003AD6F8: E5911000  ldr	r1, [r1]
	SETPC(0x003AD6F8+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD6FC: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD700: 059F1798  ldreq	r1, 003ADEA0=SWIBoot+808
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100E5C; // gAtomicIRQNestCountFast
	}
	
	// 003AD704: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD704+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD708: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD70C: 059F1790  ldreq	r1, 003ADEA4=SWIBoot+80C
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100FE8; // gAtomicNestCount
	}
	
	// 003AD710: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD710+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD714: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD718: 059F1788  ldreq	r1, 003ADEA8=SWIBoot+810
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100FF0; // gAtomicFIQNestCount
	}
	
	// 003AD71C: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD71C+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD720: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD724: CA000002  bgt	003AD734=SWIBoot+9C
	if (ioCPU->TestGT()) {
		SETPC(0x003AD734+4);
		goto L003AD734;
	}
	
	// 003AD728: E321F013  msr	cpsr_c, #0x00000013
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x00000013 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x00000013);
		}
	}
	
	// 003AD72C: E1A00000  mov	r0, r0
	
	// 003AD730: E1A00000  mov	r0, r0
	
L003AD734:
	// 003AD734: E1A0100E  mov	r1, lr
	R1 = LR;
	
	// 003AD738: E5111004  ldr	r1, [r1, -#0x004]
	SETPC(0x003AD738+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 - 0x0004);
	
	// 003AD73C: E3C114FF  bic	r1, r1, #0xff000000
	R1 = R1 & 0x00FFFFFF;
	
//	// 003AD740: E3510023  cmp	r1, #0x00000023
//	{
//		KUInt32 Opnd2 = 0x00000023;
//		KUInt32 Opnd1 = R1;
//		const KUInt32 theResult = Opnd1 - Opnd2;
//		ioCPU->mCPSR_Z = (theResult==0);
//		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
//		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
//		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
//	}
//	
//	// 003AD744: AA000181  bge	003ADD50=SWIBoot+6B8
//	if (ioCPU->TestGE()) {
//		// rt cjitr 003ADD50
//		SETPC(0x003ADD50+4);
//		return Func_0x003ADD50(ioCPU, ret);
//	}
	
	// 003AD748: E51F01E8  ldr	r0, 003AD568=FlushEntireTLB+20
	{
		KUInt32 offset = 0x000001E8;
		KUInt32 theAddress = 0x003AD748 + 8 - offset;
		SETPC(0x003AD748+8);
		KUInt32 theData = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, theAddress);
		R0 = theData;
	}
	
	switch (inSWI) {
		case 0x0000000B: SWI11_SemOp(ioCPU); break;
		default:
			NEWT_ASSERT(0==1);
	}
	
	// 003AD74C: E790F101  ldr	pc, [r0, r1, lsl #2]
//	switch (R1) {
//		case  0: SETPC(0x003ADD3C+4); Func_0x003ADD3C(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  1: SETPC(0x003ADFAC+4); Func_0x003ADFAC(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  2: SETPC(0x003AE070+4); Func_0x003AE070(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  3: SETPC(0x00393D8C+4); Func_0x00393D8C(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  4: SETPC(0x00393E2C+4); Func_0x00393E2C(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  5: SETPC(0x003ADBB4+4); Func_0x003ADBB4(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  6: SETPC(0x003ADEDC+4); Func_0x003ADEDC(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  7: SETPC(0x003ADC88+4); Func_0x003ADC88(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  8: SETPC(0x003ADCB0+4); Func_0x003ADCB0(ioCPU, 0x003AD750+4); goto L003AD750;
//		case  9: SETPC(0x003ADCD4+4); Func_0x003ADCD4(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 10: SETPC(0x003ADD10+4); Func_0x003ADD10(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 11: SETPC(0x003ADEE4+4); Func_0x003ADEE4(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 12: SETPC(0x003ADD1C+4); Func_0x003ADD1C(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 13: SETPC(0x00394050+4); Func_0x00394050(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 14: SETPC(0x00394064+4); Func_0x00394064(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 15: SETPC(0x003940C0+4); Func_0x003940C0(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 16: SETPC(0x00394120+4); Func_0x00394120(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 17: SETPC(0x003941A4+4); Func_0x003941A4(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 18: SETPC(0x003941B8+4); Func_0x003941B8(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 19: SETPC(0x003941CC+4); Func_0x003941CC(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 20: SETPC(0x003941F8+4); Func_0x003941F8(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 21: SETPC(0x0039420C+4); Func_0x0039420C(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 22: SETPC(0x00394238+4); Func_0x00394238(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 23: SETPC(0x00394264+4); Func_0x00394264(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 24: SETPC(0x003ADE7C+4); Func_0x003ADE7C(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 25: SETPC(0x003ADEC8+4); Func_0x003ADEC8(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 26: SETPC(0x00394180+4); Func_0x00394180(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 27: SETPC(0x00394278+4); Func_0x00394278(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 28: SETPC(0x00394370+4); Func_0x00394370(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 29: SETPC(0x00394384+4); Func_0x00394384(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 30: SETPC(0x00393F10+4); Func_0x00393F10(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 31: SETPC(0x00393FD4+4); Func_0x00393FD4(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 32: SETPC(0x00394398+4); Func_0x00394398(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 33: SETPC(0x003AE138+4); Func_0x003AE138(ioCPU, 0x003AD750+4); goto L003AD750;
//		case 34: SETPC(0x003AE14C+4); Func_0x003AE14C(ioCPU, 0x003AD750+4); goto L003AD750;
//	}
//	{
//		KUInt32 offset = GetShiftNoCarryNoR15( 0xE790F101, ioCPU->mCurrentRegisters, ioCPU->mCPSR_C );
//		KUInt32 theAddress = R0 + offset;
//		SETPC(0x003AD74C+8);
//		KUInt32 theData = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, theAddress);
//		SETPC(theData + 4);
//		return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
//	}
	
	// 003AD750: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003AD750+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD754: E321F0D3  msr	cpsr_c, #0x000000d3
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x000000D3 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x000000D3);
		}
	}
	
	// 003AD758: E1A00000  mov	r0, r0
	
	// 003AD75C: E1A00000  mov	r0, r0
	
	// 003AD760: E59F1734  ldr	r1, 003ADE9C=SWIBoot+804
	R1 = 0x0C100E58; // gAtomicFIQNestCountFast
	
	// 003AD764: E5911000  ldr	r1, [r1]
	SETPC(0x003AD764+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD768: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD76C: 059F172C  ldreq	r1, 003ADEA0=SWIBoot+808
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100E5C; // gAtomicIRQNestCountFast
	}
	
	// 003AD770: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD770+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD774: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD778: 059F1724  ldreq	r1, 003ADEA4=SWIBoot+80C
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100FE8; // gAtomicNestCount
	}
	
	// 003AD77C: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD77C+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD780: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD784: 059F171C  ldreq	r1, 003ADEA8=SWIBoot+810
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100FF0; // gAtomicFIQNestCount
	}
	
	// 003AD788: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD788+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD78C: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD790: CA0000B7  bgt	003ADA74=SWIBoot+3DC
	if (ioCPU->TestGT()) {
		SETPC(0x003ADA74+4);
		goto L003ADA74;
	}
	
	// 003AD794: E321F093  msr	cpsr_c, #0x00000093
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x00000093 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x00000093);
		}
	}
	
	// 003AD798: E1A00000  mov	r0, r0
	
	// 003AD79C: E1A00000  mov	r0, r0
	
	// 003AD7A0: E59F1704  ldr	r1, 003ADEAC=SWIBoot+814
	R1 = 0x0C101028; // gWantDeferred
	
	// 003AD7A4: E5911000  ldr	r1, [r1]
	SETPC(0x003AD7A4+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD7A8: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD7AC: 1A000005  bne	003AD7C8=SWIBoot+130
	if (ioCPU->TestNE()) {
		SETPC(0x003AD7C8+4);
		goto L003AD7C8;
	}
	
	// 003AD7B0: E59F16F8  ldr	r1, 003ADEB0=SWIBoot+818
	R1 = 0x0C100FE4; // gSchedule
	
	// 003AD7B4: E5911000  ldr	r1, [r1]
	SETPC(0x003AD7B4+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD7B8: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD7BC: 0A0000AC  beq	003ADA74=SWIBoot+3DC
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADA74+4);
		goto L003ADA74;
	}
	
	// 003AD7C0: E92D5C0C  stmfd	r13!, {r2-r3, r10-r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (6 * 4);
		baseAddress -= (6 * 4);
		SETPC(0x003AD7C0+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD7C4: EA00000B  b	003AD7F8=SWIBoot+160
	SETPC(0x003AD7F8+4);
	goto L003AD7F8;
	
L003AD7C8:
	// 003AD7C8: E92D5C0C  stmfd	r13!, {r2-r3, r10-r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (6 * 4);
		baseAddress -= (6 * 4);
		SETPC(0x003AD7C8+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD7CC: E321F013  msr	cpsr_c, #0x00000013
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x00000013 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x00000013);
		}
	}
	
	// 003AD7D0: E1A00000  mov	r0, r0
	
	// 003AD7D4: E1A00000  mov	r0, r0
	
	// 003AD7D8: EB5D28EA  bl	DoDeferrals
	{
		SETPC(0x003AD7D8+8);
		KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01AF7B88);
		if (jumpInstr!=0xEA9941C2) {
			__asm__("int $3\n" : : ); // unexpected jump table entry
			UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01AF7B88);
		}
	}
	LR = 0x003AD7D8 + 4;
	// rt cjitr DoDeferrals
	SETPC(0x00148298+4);
	Func_0x00148298(ioCPU, 0x003AD7E0);
	
	// 003AD7DC: E321F093  msr	cpsr_c, #0x00000093
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x00000093 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x00000093);
		}
	}
	
	// 003AD7E0: E1A00000  mov	r0, r0
	
	// 003AD7E4: E1A00000  mov	r0, r0
	
	// 003AD7E8: E59F16C0  ldr	r1, 003ADEB0=SWIBoot+818
	R1 = 0x0C100FE4; // gSchedule
	
	// 003AD7EC: E5911000  ldr	r1, [r1]
	SETPC(0x003AD7EC+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD7F0: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD7F4: 0A00009D  beq	003ADA70=SWIBoot+3D8
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADA70+4);
		goto L003ADA70;
	}
	
L003AD7F8:
	// 003AD7F8: EB5D5EA8  bl	Scheduler
	{
		SETPC(0x003AD7F8+8);
		KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B052A0);
		if (jumpInstr!=0xEA9B1BD1) {
			__asm__("int $3\n" : : ); // unexpected jump table entry
			UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B052A0);
		}
	}
	LR = 0x003AD7F8 + 4;
	// rt cjitr Scheduler
	SETPC(0x001CC1EC+4);
	Func_0x001CC1EC(ioCPU, 0x003AD800);
	
	// 003AD7FC: E59F16B0  ldr	r1, 003ADEB4=SWIBoot+81C
	R1 = 0x0C101A2C; // gWantSchedulerToRun
	
	// 003AD800: E5911000  ldr	r1, [r1]
	SETPC(0x003AD800+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD804: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD808: 0A000002  beq	003AD818=SWIBoot+180
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD818+4);
		goto L003AD818;
	}
	
	// 003AD80C: E92D0001  stmfd	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (1 * 4);
		baseAddress -= (1 * 4);
		SETPC(0x003AD80C+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD810: EB5D66F1  bl	StartScheduler
	{
		SETPC(0x003AD810+8);
		KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B073DC);
		if (jumpInstr!=0xEA9B1431) {
			__asm__("int $3\n" : : ); // unexpected jump table entry
			UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B073DC);
		}
	}
	LR = 0x003AD810 + 4;
	// rt cjitr StartScheduler
	SETPC(0x001CC4A8+4);
	Func_0x001CC4A8(ioCPU, 0x003AD818);
	
	// 003AD814: E8BD0001  ldmea	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (1 * 4);
		SETPC(0x003AD814+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
L003AD818:
	// 003AD818: E59F1698  ldr	r1, 003ADEB8=SWIBoot+820
	R1 = 0x0C100FF8; // gCurrentTask
	
	// 003AD81C: E5911000  ldr	r1, [r1]
	SETPC(0x003AD81C+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD820: E1500001  cmp	r0, r1
	{
		KUInt32 Opnd2 = R1;
		KUInt32 Opnd1 = R0;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD824: 0A000091  beq	003ADA70=SWIBoot+3D8
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADA70+4);
		goto L003ADA70;
	}
	
	// 003AD828: E59F2688  ldr	r2, 003ADEB8=SWIBoot+820
	R2 = 0x0C100FF8; // gCurrentTask
	
	// 003AD82C: E5820000  str	r0, [r2]
	SETPC(0x003AD82C+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R2, R0);
	
	// 003AD830: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD834: 028DD020  addeq	r13, r13, #0x00000020
	if (ioCPU->TestEQ()) {
		SP = SP + 0x00000020; // 32
	}
	
	// 003AD838: 0A000054  beq	003AD990=SWIBoot+2F8
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD990+4);
		goto L003AD990;
	}
	
	// 003AD83C: E8BD5C0C  ldmea	r13!, {r2-r3, r10-r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (6 * 4);
		SETPC(0x003AD83C+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		LR = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD840: E1A00001  mov	r0, r1
	R0 = R1;
	
	// 003AD844: E59F1670  ldr	r1, 003ADEBC=SWIBoot+824
	R1 = 0x0C101040; // gCopyDone
	
	// 003AD848: E5911000  ldr	r1, [r1]
	SETPC(0x003AD848+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD84C: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD850: 1A000022  bne	003AD8E0=SWIBoot+248
	if (ioCPU->TestNE()) {
		SETPC(0x003AD8E0+4);
		goto L003AD8E0;
	}
	
	// 003AD854: E14F1000  mrs	r1, spsr
	R1 = ioCPU->GetSPSR();
	
	// 003AD858: E5801050  str	r1, [r0, #0x050]
	SETPC(0x003AD858+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R0 + 0x0050, R1);
	
	// 003AD85C: E201101F  and	r1, r1, #0x0000001f
	R1 = R1 & 0x0000001F;
	
	// 003AD860: E351001B  cmp	r1, #0x0000001b
	{
		KUInt32 Opnd2 = 0x0000001B;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD864: E2801018  add	r1, r0, #0x00000018
	R1 = R0 + 0x00000018; // 24
	
	// 003AD868: E92D0001  stmfd	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (1 * 4);
		baseAddress -= (1 * 4);
		SETPC(0x003AD868+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD86C: E10F0000  mrs	r0, cpsr
	R0 = ioCPU->GetCPSR();
	
	// 003AD870: E321F0D3  msr	cpsr_c, #0x000000d3
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x000000D3 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x000000D3);
		}
	}
	
	// 003AD874: E1A00000  mov	r0, r0
	
	// 003AD878: E1A00000  mov	r0, r0
	
	// 003AD87C: 18A100FC  stmneia	r1!, {r2-r7}
	if (ioCPU->TestNE()) {
		KUInt32 baseAddress = R1;
		KUInt32 wbAddress = baseAddress + (6 * 4);
		SETPC(0x003AD87C+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
		R1 = wbAddress;
	}
	
	// 003AD880: 18C17F00  stmneia	r1, {r8-lr}^
	if (ioCPU->TestNE()) {
		KUInt32 baseAddress = R1;
		SETPC(0x003AD880+8);
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
	
	// 003AD884: E1A00000  mov	r0, r0
	
	// 003AD888: E129F000  msr	cpsr_cf, r0
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R0 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R0);
		}
	}
	
	// 003AD88C: E1A00000  mov	r0, r0
	
	// 003AD890: E1A00000  mov	r0, r0
	
	// 003AD894: E8BD0001  ldmea	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (1 * 4);
		SETPC(0x003AD894+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD898: 1A00000B  bne	003AD8CC=SWIBoot+234
	if (ioCPU->TestNE()) {
		SETPC(0x003AD8CC+4);
		goto L003AD8CC;
	}
	
	// 003AD89C: E8A11FFC  stmia	r1!, {r2-r12}
	{
		KUInt32 baseAddress = R1;
		KUInt32 wbAddress = baseAddress + (11 * 4);
		SETPC(0x003AD89C+8);
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
	
	// 003AD8A0: E1A00000  mov	r0, r0
	
	// 003AD8A4: E10F4000  mrs	r4, cpsr
	R4 = ioCPU->GetCPSR();
	
	// 003AD8A8: E14F5000  mrs	r5, spsr
	R5 = ioCPU->GetSPSR();
	
	// 003AD8AC: E129F005  msr	cpsr_cf, r5
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R5 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R5);
		}
	}
	
	// 003AD8B0: E1A00000  mov	r0, r0
	
	// 003AD8B4: E1A00000  mov	r0, r0
	
	// 003AD8B8: E8816000  stmia	r1, {r13-lr}
	{
		KUInt32 baseAddress = R1;
		SETPC(0x003AD8B8+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, SP); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
	}
	
	// 003AD8BC: E1A00000  mov	r0, r0
	
	// 003AD8C0: E129F004  msr	cpsr_cf, r4
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R4 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R4);
		}
	}
	
	// 003AD8C4: E1A00000  mov	r0, r0
	
	// 003AD8C8: E1A00000  mov	r0, r0
	
L003AD8CC:
	// 003AD8CC: E2801010  add	r1, r0, #0x00000010
	R1 = R0 + 0x00000010; // 16
	
	// 003AD8D0: E581E03C  str	lr, [r1, #0x03c]
	SETPC(0x003AD8D0+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x003C, LR);
	
	// 003AD8D4: E8BD000C  ldmea	r13!, {r2-r3}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003AD8D4+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD8D8: E881000C  stmia	r1, {r2-r3}
	{
		KUInt32 baseAddress = R1;
		SETPC(0x003AD8D8+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
	}
	
	// 003AD8DC: EA00002B  b	003AD990=SWIBoot+2F8
	SETPC(0x003AD990+4);
	goto L003AD990;
	
L003AD8E0:
	// 003AD8E0: E1A01000  mov	r1, r0
	R1 = R0;
	
	// 003AD8E4: E280004C  add	r0, r0, #0x0000004c
	R0 = R0 + 0x0000004C; // 76
	
	// 003AD8E8: E590E000  ldr	lr, [r0]
	SETPC(0x003AD8E8+8);
	LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 003AD8EC: E2810010  add	r0, r1, #0x00000010
	R0 = R1 + 0x00000010; // 16
	
	// 003AD8F0: E890000C  ldmia	r0, {r2-r3}
	{
		KUInt32 baseAddress = R0;
		SETPC(0x003AD8F0+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
	}
	
	// 003AD8F4: E88D000C  stmea	r13, {r2-r3}
	{
		KUInt32 baseAddress = SP;
		SETPC(0x003AD8F4+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
	}
	
	// 003AD8F8: E1A00001  mov	r0, r1
	R0 = R1;
	
	// 003AD8FC: E14F1000  mrs	r1, spsr
	R1 = ioCPU->GetSPSR();
	
	// 003AD900: E5801050  str	r1, [r0, #0x050]
	SETPC(0x003AD900+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R0 + 0x0050, R1);
	
	// 003AD904: E201101F  and	r1, r1, #0x0000001f
	R1 = R1 & 0x0000001F;
	
	// 003AD908: E351001B  cmp	r1, #0x0000001b
	{
		KUInt32 Opnd2 = 0x0000001B;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD90C: E2801018  add	r1, r0, #0x00000018
	R1 = R0 + 0x00000018; // 24
	
	// 003AD910: E92D0001  stmfd	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (1 * 4);
		baseAddress -= (1 * 4);
		SETPC(0x003AD910+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD914: E10F0000  mrs	r0, cpsr
	R0 = ioCPU->GetCPSR();
	
	// 003AD918: E321F0D3  msr	cpsr_c, #0x000000d3
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((0x000000D3 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(0x000000D3);
		}
	}
	
	// 003AD91C: E1A00000  mov	r0, r0
	
	// 003AD920: E1A00000  mov	r0, r0
	
	// 003AD924: 18A100FC  stmneia	r1!, {r2-r7}
	if (ioCPU->TestNE()) {
		KUInt32 baseAddress = R1;
		KUInt32 wbAddress = baseAddress + (6 * 4);
		SETPC(0x003AD924+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
		R1 = wbAddress;
	}
	
	// 003AD928: 18C17F00  stmneia	r1, {r8-lr}^
	if (ioCPU->TestNE()) {
		KUInt32 baseAddress = R1;
		SETPC(0x003AD928+8);
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
	
	// 003AD92C: E1A00000  mov	r0, r0
	
	// 003AD930: E129F000  msr	cpsr_cf, r0
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R0 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R0);
		}
	}
	
	// 003AD934: E1A00000  mov	r0, r0
	
	// 003AD938: E1A00000  mov	r0, r0
	
	// 003AD93C: E8BD0001  ldmea	r13!, {r0}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (1 * 4);
		SETPC(0x003AD93C+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD940: 1A00000B  bne	003AD974=SWIBoot+2DC
	if (ioCPU->TestNE()) {
		SETPC(0x003AD974+4);
		goto L003AD974;
	}
	
	// 003AD944: E8A11FFC  stmia	r1!, {r2-r12}
	{
		KUInt32 baseAddress = R1;
		KUInt32 wbAddress = baseAddress + (11 * 4);
		SETPC(0x003AD944+8);
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
	
	// 003AD948: E1A00000  mov	r0, r0
	
	// 003AD94C: E10F4000  mrs	r4, cpsr
	R4 = ioCPU->GetCPSR();
	
	// 003AD950: E14F5000  mrs	r5, spsr
	R5 = ioCPU->GetSPSR();
	
	// 003AD954: E129F005  msr	cpsr_cf, r5
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R5 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R5);
		}
	}
	
	// 003AD958: E1A00000  mov	r0, r0
	
	// 003AD95C: E1A00000  mov	r0, r0
	
	// 003AD960: E8816000  stmia	r1, {r13-lr}
	{
		KUInt32 baseAddress = R1;
		SETPC(0x003AD960+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, SP); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
	}
	
	// 003AD964: E1A00000  mov	r0, r0
	
	// 003AD968: E129F004  msr	cpsr_cf, r4
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R4 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R4);
		}
	}
	
	// 003AD96C: E1A00000  mov	r0, r0
	
	// 003AD970: E1A00000  mov	r0, r0
	
L003AD974:
	// 003AD974: E2801010  add	r1, r0, #0x00000010
	R1 = R0 + 0x00000010; // 16
	
	// 003AD978: E581E03C  str	lr, [r1, #0x03c]
	SETPC(0x003AD978+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x003C, LR);
	
	// 003AD97C: E8BD000C  ldmea	r13!, {r2-r3}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003AD97C+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003AD980: E881000C  stmia	r1, {r2-r3}
	{
		KUInt32 baseAddress = R1;
		SETPC(0x003AD980+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
	}
	
	// 003AD984: E59F0530  ldr	r0, 003ADEBC=SWIBoot+824
	R0 = 0x0C101040; // gCopyDone
	
	// 003AD988: E3A01000  mov	r1, #0x00000000
	R1 = 0x00000000; // 0
	
	// 003AD98C: E5801000  str	r1, [r0]
	SETPC(0x003AD98C+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R0, R1);
	
L003AD990:
	// 003AD990: E59F0520  ldr	r0, 003ADEB8=SWIBoot+820
	R0 = 0x0C100FF8; // gCurrentTask
	
	// 003AD994: E5900000  ldr	r0, [r0]
	SETPC(0x003AD994+8);
	R0 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 003AD998: E1A04000  mov	r4, r0
	R4 = R0;
	
	// 003AD99C: EB5D6A9B  bl	SwapInGlobals
	{
		SETPC(0x003AD99C+8);
		KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B08410);
		if (jumpInstr!=0xEA9D2751) {
			__asm__("int $3\n" : : ); // unexpected jump table entry
			UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B08410);
		}
	}
	LR = 0x003AD99C + 4;
	// rt cjitr SwapInGlobals
	SETPC(0x0025215C+4);
	Func_0x0025215C(ioCPU, 0x003AD9A4);
	
	// 003AD9A0: E1A00004  mov	r0, r4
	R0 = R4;
	
	// 003AD9A4: E2800010  add	r0, r0, #0x00000010
	R0 = R0 + 0x00000010; // 16
	
	// 003AD9A8: E590E03C  ldr	lr, [r0, #0x03c]
	SETPC(0x003AD9A8+8);
	LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x003C);
	
	// 003AD9AC: E5901040  ldr	r1, [r0, #0x040]
	SETPC(0x003AD9AC+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0040);
	
	// 003AD9B0: E169F001  msr	spsr_cf, r1
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetSPSR();
			ioCPU->SetSPSR((R1 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetSPSR(R1);
		}
	}
	
	// 003AD9B4: E1A00000  mov	r0, r0
	
	// 003AD9B8: E1A00000  mov	r0, r0
	
	// 003AD9BC: E201101F  and	r1, r1, #0x0000001f
	R1 = R1 & 0x0000001F;
	
	// 003AD9C0: E351001B  cmp	r1, #0x0000001b
	{
		KUInt32 Opnd2 = 0x0000001B;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD9C4: 18D07FFF  ldmneia	r0, {r0-lr}^
	if (ioCPU->TestNE()) {
		KUInt32 baseAddress = R0;
		SETPC(0x003AD9C4+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R4 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R5 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R6 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R7 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
			R8 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R9 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		} else {
			ioCPU->mR8_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			ioCPU->mR9_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			ioCPU->mR10_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			ioCPU->mR11_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			ioCPU->mR12_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		}
		ioCPU->mR13_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		ioCPU->mR14_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
	}
	
	// 003AD9C8: E1A00000  mov	r0, r0
	
	// 003AD9CC: 1A00000A  bne	003AD9FC=SWIBoot+364
	if (ioCPU->TestNE()) {
		SETPC(0x003AD9FC+4);
		goto L003AD9FC;
	}
	
	// 003AD9D0: E5902040  ldr	r2, [r0, #0x040]
	SETPC(0x003AD9D0+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0040);
	
	// 003AD9D4: E10F1000  mrs	r1, cpsr
	R1 = ioCPU->GetCPSR();
	
	// 003AD9D8: E129F002  msr	cpsr_cf, r2
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R2 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R2);
		}
	}
	
	// 003AD9DC: E1A00000  mov	r0, r0
	
	// 003AD9E0: E1A00000  mov	r0, r0
	
	// 003AD9E4: E590D034  ldr	r13, [r0, #0x034]
	SETPC(0x003AD9E4+8);
	SP = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0034);
	
	// 003AD9E8: E590E038  ldr	lr, [r0, #0x038]
	SETPC(0x003AD9E8+8);
	LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0038);
	
	// 003AD9EC: E129F001  msr	cpsr_cf, r1
	{
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {
			const KUInt32 oldValue = ioCPU->GetCPSR();
			ioCPU->SetCPSR((R1 & 0xF0000000) | (oldValue & 0x0FFFFFFF));
		} else {
			ioCPU->SetCPSR(R1);
		}
	}
	
	// 003AD9F0: E1A00000  mov	r0, r0
	
	// 003AD9F4: E1A00000  mov	r0, r0
	
	// 003AD9F8: E8901FFF  ldmia	r0, {r0-r12}
	{
		KUInt32 baseAddress = R0;
		SETPC(0x003AD9F8+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R4 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R5 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R6 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R7 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R8 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R9 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
	}
	
L003AD9FC:
	// 003AD9FC: E92D0007  stmfd	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (3 * 4);
		baseAddress -= (3 * 4);
		SETPC(0x003AD9FC+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADA00: E59F14B0  ldr	r1, 003ADEB8=SWIBoot+820
	R1 = 0x0C100FF8; // gCurrentTask
	
	// 003ADA04: E5911000  ldr	r1, [r1]
	SETPC(0x003ADA04+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003ADA08: E3A00000  mov	r0, #0x00000000
	R0 = 0x00000000; // 0
	
	// 003ADA0C: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADA0C+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADA10: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADA14: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADA14+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADA18: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
	// 003ADA1C: E5912078  ldr	r2, [r1, #0x078]
	SETPC(0x003ADA1C+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0078);
	
	// 003ADA20: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADA24: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADA24+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADA28: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
L003ADA2C:
	// 003ADA2C: E591107C  ldr	r1, [r1, #0x07c]
	SETPC(0x003ADA2C+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x007C);
	
	// 003ADA30: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADA34: 0A000005  beq	003ADA50=SWIBoot+3B8
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADA50+4);
		goto L003ADA50;
	}
	
	// 003ADA38: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADA38+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADA3C: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADA40: 0A000002  beq	003ADA50=SWIBoot+3B8
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADA50+4);
		goto L003ADA50;
	}
	
	// 003ADA44: E5922010  ldr	r2, [r2, #0x010]
	SETPC(0x003ADA44+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	
	// 003ADA48: E1800002  orr	r0, r0, r2
	R0 = R0 | R2;
	
	// 003ADA4C: EAFFFFF6  b	003ADA2C=SWIBoot+394
	SETPC(0x003ADA2C+4);
	goto L003ADA2C;
	
L003ADA50:
	// 003ADA50: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003ADA50+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADA54: E1A00000  mov	r0, r0
	
	// 003ADA58: E59F1438  ldr	r1, 003ADE98=SWIBoot+800
	R1 = 0x0C008400; // gParamBlockFromImage
	
	// 003ADA5C: E58100B0  str	r0, [r1, #0x0b0]
	SETPC(0x003ADA5C+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x00B0, R0);
	
	// 003ADA60: E8BD0003  ldmea	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003ADA60+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADA64: EE030F13  mcr	p15, 0, r0, c3, c3, 0
	SETPC(0x003ADA64+8);
	ioCPU->SystemCoprocRegisterTransfer(0xEE030F13);
	
	// 003ADA68: E8BD0007  ldmea	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (3 * 4);
		SETPC(0x003ADA68+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADA6C: E1B0F00E  movs	pc, lr
	{
		KUInt32 Opnd2 = LR;
		const KUInt32 theResult = Opnd2;
		SETPC(theResult + 4);
		ioCPU->SetCPSR( ioCPU->GetSPSR() );
		if (ret==0xFFFFFFFF)
			return; // Return to emulator
		if (PC!=ret)
			return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
		return;
	}
	
L003ADA70:
	// 003ADA70: E8BD5C0C  ldmea	r13!, {r2-r3, r10-r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (6 * 4);
		SETPC(0x003ADA70+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		LR = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
L003ADA74:
	// 003ADA74: E92D500C  stmfd	r13!, {r2-r3, r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (4 * 4);
		baseAddress -= (4 * 4);
		SETPC(0x003ADA74+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADA78: E59F1434  ldr	r1, 003ADEB4=SWIBoot+81C
	R1 = 0x0C101A2C; // gWantSchedulerToRun
	
	// 003ADA7C: E5911000  ldr	r1, [r1]
	SETPC(0x003ADA7C+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003ADA80: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADA84: 1B5D6654  blne	StartScheduler
	if (ioCPU->TestNE()) {
		SETPC(0x003ADA84+8);
		KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B073DC);
		if (jumpInstr!=0xEA9B1431) {
			__asm__("int $3\n" : : ); // unexpected jump table entry
			UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B073DC);
		}
		LR = 0x003ADA84 + 4;
		// rt cjitr StartScheduler
		SETPC(0x001CC4A8+4);
		Func_0x001CC4A8(ioCPU, 0x003ADA8C);
	}
	
	// 003ADA88: E8BD500C  ldmea	r13!, {r2-r3, r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (4 * 4);
		SETPC(0x003ADA88+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		LR = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADA8C: E59F0428  ldr	r0, 003ADEBC=SWIBoot+824
	R0 = 0x0C101040; // gCopyDone
	
	// 003ADA90: E5900000  ldr	r0, [r0]
	SETPC(0x003ADA90+8);
	R0 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 003ADA94: E3500000  cmp	r0, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R0;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADA98: 1A00001D  bne	003ADB14=SWIBoot+47C
	if (ioCPU->TestNE()) {
		SETPC(0x003ADB14+4);
		goto L003ADB14;
	}
	
	// 003ADA9C: E8BD0003  ldmea	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003ADA9C+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADAA0: E92D0007  stmfd	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (3 * 4);
		baseAddress -= (3 * 4);
		SETPC(0x003ADAA0+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADAA4: E59F140C  ldr	r1, 003ADEB8=SWIBoot+820
	R1 = 0x0C100FF8; // gCurrentTask
	
	// 003ADAA8: E5911000  ldr	r1, [r1]
	SETPC(0x003ADAA8+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003ADAAC: E3A00000  mov	r0, #0x00000000
	R0 = 0x00000000; // 0
	
	// 003ADAB0: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADAB0+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADAB4: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADAB8: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADAB8+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADABC: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
	// 003ADAC0: E5912078  ldr	r2, [r1, #0x078]
	SETPC(0x003ADAC0+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0078);
	
	// 003ADAC4: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADAC8: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADAC8+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADACC: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
L003ADAD0:
	// 003ADAD0: E591107C  ldr	r1, [r1, #0x07c]
	SETPC(0x003ADAD0+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x007C);
	
	// 003ADAD4: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADAD8: 0A000005  beq	003ADAF4=SWIBoot+45C
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADAF4+4);
		goto L003ADAF4;
	}
	
	// 003ADADC: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADADC+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADAE0: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADAE4: 0A000002  beq	003ADAF4=SWIBoot+45C
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADAF4+4);
		goto L003ADAF4;
	}
	
	// 003ADAE8: E5922010  ldr	r2, [r2, #0x010]
	SETPC(0x003ADAE8+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	
	// 003ADAEC: E1800002  orr	r0, r0, r2
	R0 = R0 | R2;
	
	// 003ADAF0: EAFFFFF6  b	003ADAD0=SWIBoot+438
	SETPC(0x003ADAD0+4);
	goto L003ADAD0;
	
L003ADAF4:
	// 003ADAF4: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003ADAF4+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADAF8: E1A00000  mov	r0, r0
	
	// 003ADAFC: E59F1394  ldr	r1, 003ADE98=SWIBoot+800
	R1 = 0x0C008400; // gParamBlockFromImage
	
	// 003ADB00: E58100B0  str	r0, [r1, #0x0b0]
	SETPC(0x003ADB00+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x00B0, R0);
	
	// 003ADB04: E8BD0003  ldmea	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003ADB04+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADB08: EE030F13  mcr	p15, 0, r0, c3, c3, 0
	SETPC(0x003ADB08+8);
	ioCPU->SystemCoprocRegisterTransfer(0xEE030F13);
	
	// 003ADB0C: E8BD0007  ldmea	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (3 * 4);
		SETPC(0x003ADB0C+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADB10: E1B0F00E  movs	pc, lr
	{
		KUInt32 Opnd2 = LR;
		const KUInt32 theResult = Opnd2;
		SETPC(theResult + 4);
		ioCPU->SetCPSR( ioCPU->GetSPSR() );
		if (ret==0xFFFFFFFF)
			return; // Return to emulator
		if (PC!=ret)
			return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
		return;
	}
	
L003ADB14:
	// 003ADB14: E59F03A0  ldr	r0, 003ADEBC=SWIBoot+824
	R0 = 0x0C101040; // gCopyDone
	
	// 003ADB18: E3A01000  mov	r1, #0x00000000
	R1 = 0x00000000; // 0
	
	// 003ADB1C: E5801000  str	r1, [r0]
	SETPC(0x003ADB1C+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R0, R1);
	
	// 003ADB20: E59F0390  ldr	r0, 003ADEB8=SWIBoot+820
	R0 = 0x0C100FF8; // gCurrentTask
	
	// 003ADB24: E5900000  ldr	r0, [r0]
	SETPC(0x003ADB24+8);
	R0 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 003ADB28: E1A01000  mov	r1, r0
	R1 = R0;
	
	// 003ADB2C: E280004C  add	r0, r0, #0x0000004c
	R0 = R0 + 0x0000004C; // 76
	
	// 003ADB30: E590E000  ldr	lr, [r0]
	SETPC(0x003ADB30+8);
	LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 003ADB34: E2810010  add	r0, r1, #0x00000010
	R0 = R1 + 0x00000010; // 16
	
	// 003ADB38: E8900003  ldmia	r0, {r0-r1}
	{
		KUInt32 baseAddress = R0;
		SETPC(0x003ADB38+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
	}
	
	// 003ADB3C: E28DD008  add	r13, r13, #0x00000008
	SP = SP + 0x00000008; // 8
	
	// 003ADB40: E92D0007  stmfd	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (3 * 4);
		baseAddress -= (3 * 4);
		SETPC(0x003ADB40+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADB44: E59F136C  ldr	r1, 003ADEB8=SWIBoot+820
	R1 = 0x0C100FF8; // gCurrentTask
	
	// 003ADB48: E5911000  ldr	r1, [r1]
	SETPC(0x003ADB48+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003ADB4C: E3A00000  mov	r0, #0x00000000
	R0 = 0x00000000; // 0
	
	// 003ADB50: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADB50+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADB54: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB58: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADB58+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADB5C: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
	// 003ADB60: E5912078  ldr	r2, [r1, #0x078]
	SETPC(0x003ADB60+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0078);
	
	// 003ADB64: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB68: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADB68+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADB6C: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
L003ADB70:
	// 003ADB70: E591107C  ldr	r1, [r1, #0x07c]
	SETPC(0x003ADB70+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x007C);
	
	// 003ADB74: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB78: 0A000005  beq	003ADB94=SWIBoot+4FC
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADB94+4);
		goto L003ADB94;
	}
	
	// 003ADB7C: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADB7C+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADB80: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB84: 0A000002  beq	003ADB94=SWIBoot+4FC
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADB94+4);
		goto L003ADB94;
	}
	
	// 003ADB88: E5922010  ldr	r2, [r2, #0x010]
	SETPC(0x003ADB88+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	
	// 003ADB8C: E1800002  orr	r0, r0, r2
	R0 = R0 | R2;
	
	// 003ADB90: EAFFFFF6  b	003ADB70=SWIBoot+4D8
	SETPC(0x003ADB70+4);
	goto L003ADB70;
	
L003ADB94:
	// 003ADB94: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003ADB94+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADB98: E1A00000  mov	r0, r0
	
	// 003ADB9C: E59F12F4  ldr	r1, 003ADE98=SWIBoot+800
	R1 = 0x0C008400; // gParamBlockFromImage
	
	// 003ADBA0: E58100B0  str	r0, [r1, #0x0b0]
	SETPC(0x003ADBA0+8);
	UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x00B0, R0);
	
	// 003ADBA4: E8BD0003  ldmea	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (2 * 4);
		SETPC(0x003ADBA4+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADBA8: EE030F13  mcr	p15, 0, r0, c3, c3, 0
	SETPC(0x003ADBA8+8);
	ioCPU->SystemCoprocRegisterTransfer(0xEE030F13);
	
	// 003ADBAC: E8BD0007  ldmea	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (3 * 4);
		SETPC(0x003ADBAC+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADBB0: E1B0F00E  movs	pc, lr
	{
		KUInt32 Opnd2 = LR;
		const KUInt32 theResult = Opnd2;
		SETPC(theResult + 4);
		ioCPU->SetCPSR( ioCPU->GetSPSR() );
		if (ret==0xFFFFFFFF)
			return; // Return to emulator
		if (PC!=ret)
			return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
		return;
	}
	
	__asm__("int $3\n" : : ); // There was no return instruction found
}
//T_ROM_SIMULATION3(0x003AD698, "SWIBoot", Func_0x003AD698)

/**
 * Transcoded function _SemaphoreOpGlue
 * ROM: 0x003AE1FC - 0x003AE204
 */
void Func_0x003AE1FC(TARMProcessor* ioCPU, KUInt32 ret)
{
	// 003AE1FC: EF00000B  swi	0x0000000b
	SETPC(0x003AE1FC+8);
	ioCPU->DoSWI();
	Func_0x003AD698(ioCPU, 0x003AE204, 0x0000000b);
	
	// 003AE200: E1A0F00E  mov	pc, lr
	{
		KUInt32 Opnd2 = LR;
		const KUInt32 theResult = Opnd2;
		SETPC(theResult + 4);
		if (ret==0xFFFFFFFF)
			return; // Return to emulator
		if (PC!=ret)
			__asm__("int $3\n" : : ); // Unexpected return address
		return;
	}
	
	__asm__("int $3\n" : : ); // There was no return instruction found
}
T_ROM_SIMULATION3(0x003AE1FC, "_SemaphoreOpGlue", Func_0x003AE1FC)


/**
 * Transcoded function SemOp__16TUSemaphoreGroupFP17TUSemaphoreOpList8SemFlags
 * ROM: 0x0025A464 - 0x0025A470
 */
void Func_0x0025A464(TARMProcessor* ioCPU, KUInt32 ret)
{
	// 0025A464: E5911000  ldr	r1, [r1]
	SETPC(0x0025A464+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 0025A468: E5900000  ldr	r0, [r0]
	SETPC(0x0025A468+8);
	R0 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 0025A46C: EA054F62  b	_SemaphoreOpGlue
	// rt cjitr _SemaphoreOpGlue
	SETPC(0x003AE1FC+4);
	return Func_0x003AE1FC(ioCPU, ret);
	
	__asm__("int $3\n" : : ); // There was no return instruction found
}
T_ROM_SIMULATION3(0x0025A464, "SemOp__16TUSemaphoreGroupFP17TUSemaphoreOpList8SemFlags", Func_0x0025A464)

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
