//
//  SimHandcoded.h
//  Einstein
//
//  Created by Matthias Melcher on 7/30/14.
//
//

#ifndef __Einstein__SimHandcoded__
#define __Einstein__SimHandcoded__

#include "SimulatorGlue.h"

// keep this empty if possible

typedef KUInt32 ObjectId;

class TTask;

extern TARMProcessor *gCPU;

extern TTask *GCurrentTask();
extern void GSetCurrentTask(TTask *newTask);
extern KUInt32 GAtomicFIQNestCountFast();
extern KUInt32 GAtomicIRQNestCountFast();
extern KUInt32 GAtomicNestCount();
extern KUInt32 GAtomicFIQNestCount();
extern KUInt32 GCopyDone();
extern void GSetCopyDone(KUInt32 v);
extern KUInt32 GWantDeferred();
extern KUInt32 GSchedule();
extern KUInt32 GWantSchedulerToRun();
extern void GSetCurrentTaskId(ObjectId id);
extern void GSetCurrentGlobals(void *v);
extern void GSetCurrentMonitorId(ObjectId id);

extern void Func_0x003AD750(TARMProcessor* ioCPU, KUInt32 ret); // Func_0x003AD750

#endif /* defined(__Einstein__SimHandcoded__) */
