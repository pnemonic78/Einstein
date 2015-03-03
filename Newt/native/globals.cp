//
//  TTask.cp
//  Einstein
//
//  Created by Matthias Melcher on 2/22/14.
//
//

#include "globals.h"


//  _GLOBAL_W_REF(0x0C008400, TParamBlock*,		ParamBlockFromImage)

NEWT_GLOBAL_W_IMP(0x0C100E54, ULong,			IntMaskShadowReg)
NEWT_GLOBAL_W_IMP(0x0C100E58, KUInt32,			AtomicFIQNestCountFast)
NEWT_GLOBAL_W_IMP(0x0C100E5C, KUInt32,			AtomicIRQNestCountFast)

NEWT_GLOBAL_W_IMP(0x0C100E6C, InterruptObject*, SchedulerIntObj)

NEWT_GLOBAL_W_IMP(0x0C100FC4, TTask*,			IdleTask)
NEWT_GLOBAL_W_IMP(0x0C100FC8, TObjectTable*,	ObjectTable)

NEWT_GLOBAL_W_IMP(0x0C100FD0, TScheduler*,		KernelScheduler)
NEWT_GLOBAL_W_IMP(0x0C100FD4, KUInt32,			ScheduleRequested)
NEWT_GLOBAL_W_IMP(0x0C100FD8, KUInt32,			HoldScheduleLevel)

NEWT_GLOBAL_W_IMP(0x0C100FE4, KUInt32,			Schedule)
NEWT_GLOBAL_W_IMP(0x0C100FE8, KUInt32,			AtomicNestCount)

NEWT_GLOBAL_W_IMP(0x0C100FF0, KUInt32,			AtomicFIQNestCount)

NEWT_GLOBAL_W_IMP(0x0C100FF8, TTask*,			CurrentTask)

NEWT_GLOBAL_W_IMP(0x0C101028, KUInt32,			WantDeferred)

NEWT_GLOBAL_W_IMP(0x0C101040, KUInt32,			CopyDone)

NEWT_GLOBAL_W_IMP(0x0C101054, ObjectId,			CurrentTaskId)
NEWT_GLOBAL_W_IMP(0x0C101058, ObjectId,			CurrentMonitorId)
NEWT_GLOBAL_W_IMP(0x0C10105C, void*,			CurrentGlobals)

NEWT_GLOBAL_L_IMP(0x0C10156C, KSInt64,			TimerSample)

NEWT_GLOBAL_W_IMP(0x0C101980, int,				TaskPriority)

NEWT_GLOBAL_W_IMP(0x0C101A2C, BOOL,				WantSchedulerToRun)

NEWT_GLOBAL_W_IMP(0x0C101A30, KUInt32,			SchedulerRunning)



