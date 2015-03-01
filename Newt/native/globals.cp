//
//  TTask.cp
//  Einstein
//
//  Created by Matthias Melcher on 2/22/14.
//
//

#include "globals.h"


KUInt32 GCopyDone()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C101040);
}

void SetGCopyDone(KUInt32 v)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C101040, v);
}


KUInt32 GAtomicFIQNestCountFast()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100E58);
}


KUInt32 GAtomicIRQNestCountFast()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100E5C);
}


KUInt32 GAtomicNestCount()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100FE8);
}


KUInt32 GAtomicFIQNestCount()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100FF0);
}


KUInt32 GWantDeferred()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C101028);
}


void SetGCurrentTaskId(ObjectId id)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C101054, id);
}

void SetGCurrentGlobals(void *v)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C10105C, (uintptr_t)v);
}

void SetGCurrentMonitorId(ObjectId id)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C101058, id);
}

KUInt32 GSchedulerRunning()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C101A30);
}

void GSchedulerRunning(KUInt32 v)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C101A30, v);
}

InterruptObject *GSchedulerIntObj()
{
	return (InterruptObject*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100E6C);
}


//  _GLOBAL_W_REF(0x0C008400, TParamBlock*,		ParamBlockFromImage)

NEWT_GLOBAL_W_IMP(0x0C100FC4, TTask*,			IdleTask)
NEWT_GLOBAL_W_IMP(0x0C100FC8, TObjectTable*,	ObjectTable)

NEWT_GLOBAL_W_IMP(0x0C100FD0, TScheduler*,		KernelScheduler)
NEWT_GLOBAL_W_IMP(0x0C100FD4, KUInt32,			ScheduleRequested)
NEWT_GLOBAL_W_IMP(0x0C100FD8, KUInt32,			HoldScheduleLevel)

NEWT_GLOBAL_W_IMP(0x0C100FE4, KUInt32,			Schedule)

NEWT_GLOBAL_W_IMP(0x0C100FF8, TTask*,			CurrentTask)

NEWT_GLOBAL_W_IMP(0x0C101980, int,				TaskPriority)

NEWT_GLOBAL_W_IMP(0x0C101A2C, BOOL,				WantSchedulerToRun)



