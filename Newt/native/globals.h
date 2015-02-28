//
//  TTask.h
//  Einstein
//
//  Created by Matthias Melcher on 2/22/14.
//
//

#ifndef _NEWT_GLOBALS_
#define _NEWT_GLOBALS_

#include "Newt/native/types.h"

#include "SimulatorGlue.h"



extern TParamBlock* GParamBlockFromImage();
extern void SetGCurrentTask(TTask *newTask);
extern KUInt32 GCopyDone();
extern void SetGCopyDone(KUInt32 v);
extern KUInt32 GAtomicFIQNestCountFast();
extern KUInt32 GAtomicIRQNestCountFast();
extern KUInt32 GAtomicNestCount();
extern KUInt32 GAtomicFIQNestCount();
extern KUInt32 GWantDeferred();
extern KUInt32 GSchedule();
extern void SetGCurrentTaskId(ObjectId id);
extern void SetGCurrentGlobals(void *v);
extern void SetGCurrentMonitorId(ObjectId id);
extern KUInt32 GSchedulerRunning();
extern void GSchedulerRunning(KUInt32 v);
extern InterruptObject *GSchedulerIntObj();


NEWT_GLOBAL_W_DEF(0x0C100FC4, TTask*,			IdleTask)
NEWT_GLOBAL_W_DEF(0x0C100FC8, TObjectTable*,	ObjectTable)

NEWT_GLOBAL_W_DEF(0x0C100FD0, TScheduler*,		KernelScheduler)
NEWT_GLOBAL_W_DEF(0x0C100FD4, KUInt32,			ScheduleRequested)
NEWT_GLOBAL_W_DEF(0x0C100FD8, KUInt32,			HoldScheduleLevel)

NEWT_GLOBAL_W_DEF(0x0C100FE4, KUInt32,			Schedule)

NEWT_GLOBAL_W_DEF(0x0C100FF8, TTask*,			CurrentTask)

NEWT_GLOBAL_W_DEF(0x0C101980, int,				TaskPriority)

NEWT_GLOBAL_W_DEF(0x0C101A2C, BOOL,				WantSchedulerToRun)


#endif /* defined(_NEWT_GLOBALS_) */
