//
//  TTask.h
//  Einstein
//
//  Created by Matthias Melcher on 2/22/14.
//
//

#ifndef _NEWT_TTASK_
#define _NEWT_TTASK_

#include "SimulatorGlue.h"


typedef KUInt32 ObjectId;
typedef KUInt32 ULong;
typedef KUInt32 KernelObjectState;
typedef void *	Heap;
typedef KSInt8  BOOL;


class TTask;


class TEnvironment {
public:
	KUInt32 GetDomainAccess() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x10); };

//public:
//	~CEnvironment();
//	
//	NewtonErr	init(Heap inHeap);
//	NewtonErr	add(CDomain * inDomain, BOOL inIsManager, BOOL inHasStack, BOOL inHasHeap);
//	NewtonErr	remove(CDomain * inDomain);
//	NewtonErr	hasDomain(CDomain * inDomain, BOOL * outHasDomain, BOOL * outIsManager);
//	void			incrRefCount(void);
//	BOOL			decrRefCount(void);
	
	NEWT_GET_SET_W(0x10, ULong, DomainAccess);
	NEWT_GET_SET_W(0x14, Heap,  fHeap);
	NEWT_GET_SET_W(0x18, ObjectId, StackDomainId);
	NEWT_GET_SET_W(0x1c, ObjectId, HeapDomainId);
	NEWT_GET_SET_W(0x20, KSInt32, RefCount);
	NEWT_GET_SET_B(0x24, BOOL, Unknown24);
	NEWT_GET_SET_W(0x28, TEnvironment*, NextEnvironment);
};


class TObject
{
public:
/*
    CObject(ObjectId id = 0)			{ fId = id; }
    CObject(const CObject & inCopy)	{ fId = inCopy.fId; }
    
    operator	ObjectId()	const			{ return fId; }
    
    void			setOwner(ObjectId id)				{ fOwnerId = id; }
    ObjectId		owner(void)				const			{ return fOwnerId; }
    
    void			assignToTask(ObjectId id)			{ fAssignedOwnerId = id; }
    ObjectId		assignedOwner(void)	const			{ return fAssignedOwnerId; }
*/
	//protected:
    //    friend class CObjectTable;
    //    friend class CObjectTableIterator;
	
	NEWT_GET_SET_W(0x000, ObjectId, Id);
	NEWT_GET_SET_W(0x004, TObject*, Next);
	NEWT_GET_SET_W(0x008, ObjectId, OwnerId);
	NEWT_GET_SET_W(0x00C, ObjectId, AssignedOwnerId);
};


class TTaskQItem
{
public:
	NEWT_GET_SET_W(0x000, TTask*, Next);
	NEWT_GET_SET_W(0x004, TTask*, Prev);
};


class TTaskContainer
{
};


class TTask : public TObject
{
public:
    KUInt32 GetRegister(KUInt32 r) { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x10+4*r); };
    void SaveRegister(KUInt32 r, KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+0x10+4*r, v); };
    KUInt32 GetPSR() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x50); };
    void SavePSR(KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+0x50, v); };
    TEnvironment *GetEnvironment() { return (TEnvironment*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x74); };
    TEnvironment *GetSMemEnvironment() { return (TEnvironment*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x78); };
    TTask *GetCurrentTask() { return (TTask*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x7C); };
    void *GetGlobals() { return (void*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0xA0); };
    ObjectId GetMonitorId() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0xD8); };
/*
	ULong				fRegister[kNumOfRegisters];	// +10	processor registers
	ULong				fPSR;					// +50	Processor Status Register
	ObjectId			f68;					// +68	set in init() but never referenced
 */
	NEWT_GET_SET_W(0x6C, KernelObjectState, State);
	/*
	CEnvironment * fEnvironment;		// +74
	CEnvironment *	fSMemEnvironment;	// +78	for copying to/from shared memory
	CTask *			fCurrentTask;		// +7C	if this task is a monitor, the task it’s running (ever referenced?)
	 */
	NEWT_GET_SET_W(0x080, int, Priority);
	/*
	ULong				fName;				// +84
	VAddr				fStackTop;			// +88
	VAddr				fStackBase;			// +8C
	 */
	NEWT_GET_SET_W(0x090, TTaskContainer*, Container);
	NEWT_GET_REF  (0x094, TTaskQItem, TaskQItem);			// +94	for tasks in scheduler queue
/*
	ObjectId			f9C;					// +9C	referenced in findAndRemove() but never set
	void *			fGlobals;			// +A0
	CTime				fTaskTime;			// +A4	time spent within this task
	size_t			fTaskDataSize;		// +AC	size of task globals
	long				fPtrsUsed;			// +B0
	long				fHandlesUsed;		// +B4
	long				fMemUsed;			// +B8	max memory (pointers + handles) used
	CDoubleQItem	fCopyQItem;			// +BC	for tasks in queue for copy
	CDoubleQItem	fMonitorQItem;		// +C8	for tasks in monitor queue
	ObjectId			fMonitor;			// +D4	if task is running in a monitor, that monitor
	ObjectId			fMonitorId;			// +D8	if this task is a monitor, its id
	VAddr				fCopySavedPC;		// +DC	PC register saved during copy task
	NewtonErr		fCopySavedErr;		// +E0	error code saved during copy task
	size_t			fCopiedSize;		// +E4
	ObjectId			fCopySavedMemId;	// +E8	CSharedMem object id saved during copy task
	ObjectId			fCopySavedMemMsgId;//+EC	CSharedMemMsg object id saved during copy task
	ObjectId			fSharedMem;			// +F0	shared mem object id
	ObjectId			fSharedMemMsg;		// +F4	shared mem msg object id
	VAddr				fTaskData;			// +F8	address of task globals (usually == this)
	ObjectId			fBequeathId;		// +FC
	ObjectId			fInheritedId;		// +100
 */
};


class TTaskQueue
{
public:
//	CTaskQueue();
//	void		checkBeforeAdd(CTask * inTask);
//	CTask *	findAndRemove(ObjectId inId, KernelObjectState inState);
//	BOOL		removeFromQueue(CTask * inTask, KernelObjectState inState);
	
	TTask*		Remove(KernelObjectState inState);
	TTask*		Peek();
	void		CheckBeforeAdd(TTask*);
	void		Add(TTask *inTask, KernelObjectState inState, TTaskContainer *inContainer);
	
	NEWT_GET_SET_W(0x000, TTask*, Head);
	NEWT_GET_SET_W(0x004, TTask*, Tail);
};


typedef TTaskQueue TTaskQueueArray[32];


class TScheduler : public TObject, TTaskContainer
{
public:
	void		Add(TTask *inTask);
	void		AddWhenNotCurrent(TTask *inTask);

//public:
//	CScheduler();
//	virtual	~CScheduler();
//	
//	virtual void		remove(CTask * inTask);
//	
//	void		setCurrentTask(CTask * inTask);
//	CTask *	schedule(void);
//	
//private:
//	CTask *	removeHighestPriority(void);
//	void		updateCurrentBucket(void);
//	
//	friend class CSharedMemMsg;

	NEWT_GET_SET_W(0x014, int, HighestPriority);
	NEWT_GET_SET_W(0x018, ULong, PriorityMask);
	NEWT_GET_REF  (0x01C, TTaskQueueArray, Task);
	NEWT_GET_SET_W(0x11C, TTask*, CurrentTask);
};


enum SemFlags
{
	kWaitOnBlock,				// ok if we need to block
	kNoWaitOnBlock				// don't wait if need to block
};


class TSemaphore : public TObject, TTaskContainer
{
public:
//	CSemaphore();
//	virtual  ~CSemaphore();
//	
//	virtual void	remove(CTask * inTask);
//	
//	void		blockOnZero(CTask * inTask, SemFlags inBlocking);
//
//private:
//	friend class CSemaphoreGroup;

//	void		BlockOnInc(TTask *inTask, SemFlags inBlocking);
	
	void		WakeTasksOnZero();
	void		WakeTasksOnInc();
	
	NEWT_GET_SET_W(0x014, long, Val);				// +14	semval in unix-speak
	NEWT_GET_REF  (0x018, TTaskQueue, ZeroTasks);		// +18	tasks waiting for fValue to become zero
	NEWT_GET_REF  (0x020, TTaskQueue, IncTasks);		// +20	tasks waiting for fValue to increase
								//	size +28
};


extern void WantSchedule();
extern void ScheduleTask(TTask*);


#endif /* defined(_NEWT_TTASK_) */
