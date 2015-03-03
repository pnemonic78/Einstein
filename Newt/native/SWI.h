//
//  SWI.h
//  Einstein
//
//  Created by Matthias Melcher on 2/28/14.
//
//

#ifndef _NEWT_SWI_
#define _NEWT_SWI_

#include "Newt/native/types.h"

#include "SimulatorGlue.h"


extern void Func_0x00392AC0(TARMProcessor* ioCPU, KUInt32 ret); // PublicEnterAtomic


extern void Func_0x001CC7F4(TARMProcessor* ioCPU, KUInt32 ret); // WantSchedule__Fv
extern void Func_0x001CC1B0(TARMProcessor* ioCPU, KUInt32 ret); // UpdateCurrentBucket__10TSchedulerFv
extern void Func_0x00359AA8(TARMProcessor* ioCPU, KUInt32 ret); // CheckBeforeAdd__10TTaskQueueFP5TTask
extern void Func_0x00359BE0(TARMProcessor* ioCPU, KUInt32 ret); // Peek__10TTaskQueueFv
extern void Func_0x00359AAC(TARMProcessor* ioCPU, KUInt32 ret); // Add__10TTaskQueueFP5TTask17KernelObjectStateP14TTaskContainer
extern void Func_0x00359B5C(TARMProcessor* ioCPU, KUInt32 ret); // RemoveFromQueue__10TTaskQueueFP5TTask17KernelObjectState
extern void Func_0x001CC564(TARMProcessor* ioCPU, KUInt32 ret); // Add__10TSchedulerFP5TTask

extern void Func_0x003AE1FC(TARMProcessor* ioCPU, KUInt32 ret); // _SemaphoreOpGlue
extern void Func_0x003AD658(TARMProcessor* ioCPU, KUInt32 ret); // DoSchedulerSWI

extern void Func_0x00392B1C(TARMProcessor* ioCPU, KUInt32 ret); // _ExitAtomic
extern void Func_0x00392B90(TARMProcessor* ioCPU, KUInt32 ret); // _EnterFIQAtomic

void _EnterFIQAtomic();
void _ExitAtomic();
NewtonErr SemaphoreOpGlue(ObjectId inGroupId, ObjectId inListId, SemFlags inBlocking);
NewtonErr DoSchedulerSWI();


#endif /* defined(_NEWT_SWI_) */
