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
#include "SWI.h"

#include "TInterruptManager.h"
#include "TMemory.h"

#include "ScreenUpdateTask.h"

#ifdef IS_NEWT_SIM
#include <stdlib.h>
#endif


#pragma mark - C and C++ Functions


NewtonErr ClearInterrupt(InterruptObject *inIntObj)
{
	if ( (inIntObj->Flags() & 0x00000040) == 0 ) {
		gCPU->GetMemory()->GetInterruptManager()->ClearInterrupts(inIntObj->HWIntMask());
	} else {
		inIntObj->SetFlags( inIntObj->Flags() & 0x00000200 );
	}
	return 0;
}


void SwapInGlobals(TTask *inTask)
{
	SetGCurrentTaskId( inTask->Id() );
	SetGCurrentGlobals( inTask->Globals() );
	SetGCurrentMonitorId( inTask->MonitorId() );
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


void UnScheduleTask(TTask *inTask)
{
	TScheduler *kernelScheduler = GKernelScheduler();
	kernelScheduler->Remove(inTask); // FIXME: a virtual function
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
 * Schedule a tesk for execution.
 */
void ScheduleTask(TTask *inTask)
{
	GKernelScheduler()->AddWhenNotCurrent(inTask);
}


#pragma mark - TTaskQueue


/**
 * Get the next task in this queue.
 * \returns the next task, NULL if there is none
 */
TTask *TTaskQueue::Peek()
{
	return Head();
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


#pragma mark - TScheduler


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


#pragma mark - TSemaphore


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


#pragma mark - TSemaphoreGroup

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
			sem.SetValue( sem.Value()-op );
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
				if (sem.Value() != 0)
				{
					Group()[semNum].BlockOnZero(inTask, inBlocking);
					UnwindOp(inList, index);
					return -10000-25; // kOSErrSemaphoreWouldCauseBlock;
				}
			}
			else if (semOper > 0 || sem.Value() >= -semOper)	// - = abs
				sem.SetValue( sem.Value() + semOper );
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
			else if (semOper == 0 && sem.Value() == 0)
				sem.WakeTasksOnZero();
		}
	}
	
	return noErr;
}


#pragma mark - TObjectTable


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


#pragma mark - Stubs for all the functions above

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

/**
 * SwapInGlobals
 * ROM: 0x0025215C - 0x00252190
 */
void Func_0x0025215C(TARMProcessor* ioCPU, KUInt32 ret)
{
	SwapInGlobals((TTask*)R0);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x0025215C, "SwapInGlobals", Func_0x0025215C)

/**
 * ClearInterrupt
 * ROM: 0x000E5960 - 0x000E598C
 */
void Func_0x000E5960(TARMProcessor* ioCPU, KUInt32 ret)
{
	InterruptObject *inIntObj = (InterruptObject*)R0;
	R0 = ClearInterrupt(inIntObj);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x000E5960, "ClearInterrupt", Func_0x000E5960)


