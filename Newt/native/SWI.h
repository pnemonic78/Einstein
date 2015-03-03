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
#include "Newt/native/TTask.h"
#include "Newt/native/TDoubleQContainer.h"

#include "SimGlue.h"


class TTime
{
public:
//	CTime()								{ }
//	CTime(const CTime & x)			{ fTime = x.fTime; };
//	CTime(ULong low)					{ set(low); }
//	CTime(ULong low, ULong high)	{ set(low, SLong(high)); }
//	CTime(ULong amount, TimeUnits units) { set(amount, units); }
//	
//	void 	set(ULong low)						{ fTime.lo = low; fTime.hi = 0L; }
//	void 	set(ULong low, SLong high)		{ fTime.lo = low; fTime.hi = high; }
//	void	set(ULong amount, TimeUnits units);
//	
//	ULong	convertTo(TimeUnits units);
//	
//	operator	ULong()	const			{ return fTime.lo; }
//	
//	CTime operator+  (const CTime& b)	{ CTime ret(b); WideAdd(&ret.fTime, &fTime); return ret; }
//	CTime operator-  (const CTime& b)	{ CTime ret(*this); WideSubtract(&ret.fTime, &b.fTime); return ret; }
//	
//	BOOL	operator== (const CTime& b) const	{ return WideCompare(&fTime, &b.fTime) == 0; }
//	BOOL	operator!= (const CTime& b) const	{ return WideCompare(&fTime, &b.fTime) != 0; }
//	BOOL	operator>  (const CTime& b) const	{ return WideCompare(&fTime, &b.fTime) > 0; }
//	BOOL	operator>= (const CTime& b) const	{ return WideCompare(&fTime, &b.fTime) >= 0; }
//	BOOL	operator<  (const CTime& b) const	{ return WideCompare(&fTime, &b.fTime) < 0; }
//	BOOL	operator<= (const CTime& b) const	{ return WideCompare(&fTime, &b.fTime) <= 0; }
//	
//	wide	fTime;	// CUPort::sendGoo needs hi,lo longs
	NEWT_GET_SET_L(0x000, KSInt64, Time);
};


class TTimerEngine : public TDoubleQContainer
{
public:
//	CTimerEngine();
//	
//	void		init(void);
//	void		start(void);
	void				Alarm();
//	BOOL		queueTimer(CSharedMemMsg * inMsg, Timeout inTimeout, void * inData, TimeoutProcPtr inProc);
//	BOOL		queueTimeout(CSharedMemMsg * inMsg);
//	BOOL		queueDelay(CSharedMemMsg * inMsg);
//	BOOL		queue(CSharedMemMsg * inMsg);
//	void		remove(CSharedMemMsg * inMsg);
};


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
extern void Func_0x00392B90(TARMProcessor* ioCPU, KUInt32 ret); // _EnterFIQAtomicFast
extern void Func_0x00392BB0(TARMProcessor* ioCPU, KUInt32 ret); // _ExitFIQAtomicFast
extern void Func_0x003AD390(TARMProcessor* ioCPU, KUInt32 ret); // SetAlarm1
extern void Func_0x003AD3BC(TARMProcessor* ioCPU, KUInt32 ret); // DisableAlarm1
extern void Func_0x003AD448(TARMProcessor* ioCPU, KUInt32 ret); // SetAlarm
extern void Func_0x00255620(TARMProcessor* ioCPU, KUInt32 ret); // SetAlarmAtomic__FR5TTime
extern void Func_0x003AD4C4(TARMProcessor* ioCPU, KUInt32 ret); // ClearAlarm
extern void Func_0x00255C04(TARMProcessor* ioCPU, KUInt32 ret); // ClearAlarmAtomic__Fv
extern void Func_0x003AD41C(TARMProcessor* ioCPU, KUInt32 ret); // GetClock
extern void Func_0x00255CB0(TARMProcessor* ioCPU, KUInt32 ret); // Alarm__12TTimerEngineFv
extern void Func_0x00070D80(TARMProcessor* ioCPU, KUInt32 ret); // (RET) CompCompare


int CompCompare(KSInt64 *a, KSInt64 *b);
void GetClock(TTime *);
void ClearAlarmAtomic();
void ClearAlarm();
BOOL SetAlarmAtomic(TTime *inTime);
BOOL SetAlarm(TTime *inTime);
void SetAlarm1(ULong when);
void DisableAlarm1();
void _EnterFIQAtomicFast();
void _ExitFIQAtomicFast();
void _ExitAtomic();
NewtonErr SemaphoreOpGlue(ObjectId inGroupId, ObjectId inListId, SemFlags inBlocking);
NewtonErr DoSchedulerSWI();


#endif /* defined(_NEWT_SWI_) */
