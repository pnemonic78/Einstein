//
//  ScreenUpdateTask.cp
//  Einstein
//
//  Created by Matthias Melcher on 3/2/15.
//  Code and Code Excerpts with kind permission from "Newton Research Group"
//
//

#include "ScreenUpdateTask.h"

#pragma mark - Still to do:

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


