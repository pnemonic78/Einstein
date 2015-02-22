//
//  TTask.cp
//  Einstein
//
//  Created by Matthias Melcher on 2/22/14.
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


TTask *TTaskQueue::Peek()
{
	return Head();
}


// UnScheduleTask__FP5TTask -> uses a virtual function or jump vector!
// Remove__10TSchedulerFP5TTask (1AE3C)
//  RemoveFromQueue__10TTaskQueueFP5TTask17KernelObjectState (leaf)
//  UpdateCurrentBucket__10TSchedulerFv (leaf)





/**
 * Transcoded function Peek__10TTaskQueueFv
 * ROM: 0x00359BE0 - 0x00359BE8
 */
void Func_0x00359BE0(TARMProcessor* ioCPU, KUInt32 ret)
{
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
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
	gCPU = ioCPU;
	TTaskQueue *This = (TTaskQueue*)(R0);
	KernelObjectState inState = (KernelObjectState)(R1);
	R0 = (KUInt32)This->Remove(inState);
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00359B14, "Remove__10TTaskQueueF17KernelObjectState", Func_0x00359B14)
