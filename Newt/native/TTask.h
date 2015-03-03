//
//  TTask.h
//  Einstein
//
//  Created by Matthias Melcher on 2/22/14.
//
//

#ifndef _NEWT_TTASK_
#define _NEWT_TTASK_

#include "SimGlue.h"
#include "Newt/native/types.h"
#include "Newt/native/globals.h"

enum SemFlags
{
	kWaitOnBlock,				// ok if we need to block
	kNoWaitOnBlock				// don't wait if need to block
};


class TEnvironment
{
public:
//	TEnvironment();
//	~TEnvironment();
	KUInt32			GetDomainAccess() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x10); };
//	NewtonErr		Init(Heap inHeap);
//	NewtonErr		Add(TDomain* inDomain, BOOL inIsManager, BOOL inHasStack, BOOL inHasHeap);
//	NewtonErr		Remove(TDomain* inDomain);
//	NewtonErr		HasDomain(TDomain* inDomain, BOOL* outHasDomain, BOOL* outIsManager);
//	void			IncrRefCount();
//	BOOL			DecrRefCount();
	
	NEWT_GET_SET_W(0x010, ULong, DomainAccess);
	ULong			pDomainAccess;					///< 010 domain access rights bitmap
	NEWT_GET_SET_W(0x014, Heap, MyHeap);
	Heap			pMyHeap;						///< 014 heap information for this environment
	NEWT_GET_SET_W(0x018, ObjectId, StackDomainId);
	ObjectId		pStackDomainId;					///< 018
	NEWT_GET_SET_W(0x01c, ObjectId, HeapDomainId);
	ObjectId		pHeapDomainId;					///< 01C
	NEWT_GET_SET_W(0x020, KSInt32, RefCount);
	KSInt32			pRefCount;						///< 020
	NEWT_GET_SET_B(0x024, BOOL, Unknown24);
	BOOL			pUnknown24;						///< 024
	NEWT_GET_SET_W(0x028, TEnvironment*, NextEnvironment);
	TEnvironment	*pNextEnvironment;				///< 028
};


class TObject
{
public:
//    CObject(ObjectId id = 0)			{ fId = id; }
//    CObject(const CObject & inCopy)	{ fId = inCopy.fId; }
//    operator	ObjectId()	const			{ return fId; }
//    void			setOwner(ObjectId id)				{ fOwnerId = id; }
//    ObjectId		owner(void)				const			{ return fOwnerId; }
//    void			assignToTask(ObjectId id)			{ fAssignedOwnerId = id; }
//    ObjectId		assignedOwner(void)	const			{ return fAssignedOwnerId; }
	
	NEWT_GET_SET_W(0x000, ObjectId, Id);
	ObjectId		pId;							///< 000
	NEWT_GET_SET_W(0x004, TObject*, Next);
	TObject			*pNext;							///< 004
	NEWT_GET_SET_W(0x008, ObjectId, OwnerId);
	ObjectId		pOwnerId;						///< 008
	NEWT_GET_SET_W(0x00C, ObjectId, AssignedOwnerId);
	ObjectId		pAssignedOwnerId;				///< 00C
};


class TTaskQItem
{
public:
	NEWT_GET_SET_W(0x000, TTask*, Next);
	TTask			*pNext;							///< 000
	NEWT_GET_SET_W(0x004, TTask*, Prev);
	TTask			*pPrev;							///< 004
};


class TTaskContainer
{
public:
	KUInt32 something;
	// FIXME: contains a virtual Remove function
};


/**
 * A cooperative mutitasking task unit.
 */
class TTask : public TObject
{
public:
//    KUInt32 GetRegister(KUInt32 r) { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x10+4*r); };
//    void SaveRegister(KUInt32 r, KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+0x10+4*r, v); };
//    KUInt32 GetPSR() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x50); };
//    void SavePSR(KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+0x50, v); };
//    TEnvironment *GetEnvironment() { return (TEnvironment*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x74); };
//    TEnvironment *GetSMemEnvironment() { return (TEnvironment*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x78); };
//    TTask *GetCurrentTask() { return (TTask*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x7C); };
//    void *GetGlobals() { return (void*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0xA0); };
//    ObjectId GetMonitorId() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0xD8); };
	
	NEWT_GET_ARR_W(0x010, ULong, Register, 16);
	ULong			pRegister[16];					///< 010 Processor Registers
	NEWT_GET_SET_W(0x050, ULong, PSR);
	ULong			pPSR;							///< 050 Processor Status Register
//	ObjectId			f68;					// +68	set in init() but never referenced
	NEWT_GET_SET_W(0x06C, KernelObjectState, State);
	KernelObjectState pState;						///< 06C
	NEWT_GET_SET_W(0x074, TEnvironment*, Environment);
	TEnvironment	*pEnvironment;					///< 074
	NEWT_GET_SET_W(0x078, TEnvironment*, SMemEnvironment);
	TEnvironment	*pSMemEnvironment;				///< 078
	NEWT_GET_SET_W(0x07C, TTask*, CurrentTask);
	TTask			*pCurrentTask;					///< 07C if this task is a monitor, the task it’s running
	NEWT_GET_SET_W(0x080, int, Priority);
	int				pPriority;						///< 080
	/*
	ULong				fName;				// +84
	VAddr				fStackTop;			// +88
	VAddr				fStackBase;			// +8C
	 */
	NEWT_GET_SET_W(0x090, TTaskContainer*, Container)
	TTaskContainer	*pContainer;					///< 090 The container that handles this task
	NEWT_GET_REF  (0x094, TTaskQItem, TaskQItem);
	TTaskQItem		pTaskQItem;						///< 094 for tasks in scheduler queue
	
//	ObjectId			f9C;					// +9C	referenced in findAndRemove() but never set
	
	NEWT_GET_SET_W(0x0A0, void*, Globals);
	void			*pGlobals;						///< 0A0 pointer to the globals variables specific to this task
	
//	CTime				fTaskTime;			// +A4	time spent within this task
//	size_t			fTaskDataSize;		// +AC	size of task globals
//	long				fPtrsUsed;			// +B0
//	long				fHandlesUsed;		// +B4
//	long				fMemUsed;			// +B8	max memory (pointers + handles) used
//	CDoubleQItem	fCopyQItem;			// +BC	for tasks in queue for copy
//	CDoubleQItem	fMonitorQItem;		// +C8	for tasks in monitor queue
//	ObjectId			fMonitor;			// +D4	if task is running in a monitor, that monitor
	
	NEWT_GET_SET_W(0x0D8, ObjectId, MonitorId);
	ObjectId			pMonitorId;			// 0D8 if this task is a monitor, this is its id
	
//	VAddr				fCopySavedPC;		// +DC	PC register saved during copy task
//	NewtonErr		fCopySavedErr;		// +E0	error code saved during copy task
//	size_t			fCopiedSize;		// +E4
//	ObjectId			fCopySavedMemId;	// +E8	CSharedMem object id saved during copy task
//	ObjectId			fCopySavedMemMsgId;//+EC	CSharedMemMsg object id saved during copy task
//	ObjectId			fSharedMem;			// +F0	shared mem object id
//	ObjectId			fSharedMemMsg;		// +F4	shared mem msg object id
//	VAddr				fTaskData;			// +F8	address of task globals (usually == this)
//	ObjectId			fBequeathId;		// +FC
//	ObjectId			fInheritedId;		// +100
};


class TTaskQueue
{
public:
//	CTaskQueue();
//	void		checkBeforeAdd(CTask * inTask);
//	CTask *	findAndRemove(ObjectId inId, KernelObjectState inState);
	
	BOOL		RemoveFromQueue(TTask *inTask, KernelObjectState inState);
	TTask*		Remove(KernelObjectState inState);
	TTask*		Peek();
	void		CheckBeforeAdd(TTask*);
	void		Add(TTask *inTask, KernelObjectState inState, TTaskContainer *inContainer);
	
	NEWT_GET_SET_W(0x000, TTask*, Head);
	TTask			*pHead;							///< 000 first task in list
	NEWT_GET_SET_W(0x004, TTask*, Tail);
	TTask			*pTail;							///< 004 last task in list
};


typedef TTaskQueue TTaskQueueArray[32];


class TScheduler : public TObject, TTaskContainer
{
public:
	void		Add(TTask *inTask);
	void		AddWhenNotCurrent(TTask *inTask);
	void		UpdateCurrentBucket();
	/* FIXME: virtual*/ void Remove(TTask *inTask);

//public:
//	CScheduler();
//	virtual	~CScheduler();
//	
//
//	void		setCurrentTask(CTask * inTask);
//	CTask *	schedule(void);
//	
//private:
//	CTask *	removeHighestPriority(void);
//
//	friend class CSharedMemMsg;

	NEWT_GET_SET_W(0x014, int, HighestPriority);
	int				pHighestPriority;				///< 014 the highes priority of any task in this scheduler
	NEWT_GET_SET_W(0x018, ULong, PriorityMask);
	ULong			pPriorityMask;					///< 018 a mask of any task priority pending
	NEWT_GET_REF  (0x01C, TTaskQueueArray, Task);
	TTaskQueueArray	pTask;							///< 01C multiple queues sorted by priority
	NEWT_GET_SET_W(0x11C, TTask*, CurrentTask);
	TTask			*pCurrentTask;					///< 11C the currently running task
};


class TSemaphore : public TObject, TTaskContainer
{
public:
//	CSemaphore();
//	virtual  ~CSemaphore();
//	
//	virtual void	remove(CTask * inTask);
//	
//
//private:
//	friend class CSemaphoreGroup;

	void		BlockOnZero(TTask *inTask, SemFlags inBlocking);
	void		BlockOnInc(TTask *inTask, SemFlags inBlocking);
	void		WakeTasksOnZero();
	void		WakeTasksOnInc();
	
	NEWT_GET_SET_W(0x014, long, Value);
	long			pValue;							///< 014 semval in unix-speak
	NEWT_GET_REF  (0x018, TTaskQueue, ZeroTasks);
	TTaskQueue		pZeroTasks;						///< 018 tasks waiting for fValue to become zero
	NEWT_GET_REF  (0x020, TTaskQueue, IncTasks);
	TTaskQueue		pIncTasks;						///< 020 tasks waiting for fValue to increase
};


class SemOp {
public:
	NEWT_GET_SET_S(0x000, unsigned short,	Num);
	unsigned short	pNum;							///< 000 index of Semaphore
	NEWT_GET_SET_S(0x002, short,			Op);
	short			pOp;							///< 002 new value
};


class TSemaphoreOpList : public TObject
{
public:
//	~CSemaphoreOpList();
//	
//	NewtonErr	init(ULong inNumOfOps, SemOp * inOps);
//	
//private:
//	friend class CSemaphoreGroup;
	
	NEWT_GET_SET_W(0x010, SemOp*,	OpList);
	SemOp			*pOpList;						///< 010
	NEWT_GET_SET_W(0x014, int,		Count);
	int				pCount;							///< 014
};


class TSemaphoreGroup : public TObject
{
public:
//	~CSemaphoreGroup();
//	
//	NewtonErr	init(ULong inCount);
//	
//
//	void			setRefCon(void * inRefCon);
//	void *		getRefCon(void) const;

	void		UnwindOp(TSemaphoreOpList *inList, long index);
	NewtonErr	SemOp(TSemaphoreOpList *inList, SemFlags inBlocking, TTask *inTask);
	
	NEWT_GET_SET_W(0x010, TSemaphore*,	Group);
	TSemaphore		*pGroup;						///< 010
	NEWT_GET_SET_W(0x014, int,			Count);
	int				pCount;							///< 014
	NEWT_GET_SET_W(0x018, void*,		RefCon);
	void			*pRefCon;						///< 018
};


typedef void (*ScavengeProcPtr)(TObject *);
typedef ScavengeProcPtr (*GetScavengeProcPtr)(TObject *, ULong);

class TObjectTable
{
public:
//	long				init(void);
//	void				setScavengeProc(GetScavengeProcPtr inScavenger);
//	void				scavenge(void);
//	void				scavengeAll(void);
//	void				reassignOwnership(ObjectId inFromOwner, ObjectId inToOwner);
//	ObjectId			newId(KernelTypes inType);
//	ObjectId			nextGlobalUniqueId(void);
//	BOOL				exists(ObjectId inId);
//	ObjectId			add(CObject * inObject, KernelTypes inType, ObjectId inOwnerId);
//	long				remove(ObjectId inId);
//	
//private:
//	friend class CObjectTableIterator;
//	
	static const int		kObjectTableSize = 0x80;		// 128
	static const int		kObjectTableMask = 0x7F;

	TObject*	Get(ObjectId inId);

	NEWT_GET_SET_W(0x000, GetScavengeProcPtr, Scavenge);
	GetScavengeProcPtr pScavenge;					///< 000
	NEWT_GET_SET_W(0x004, TObject*, ThisObj);
	TObject				*pThisObj;					///< 004
	NEWT_GET_SET_W(0x008, TObject*, PrevObj);
	TObject				*pPrevObj;					///< 008
	NEWT_GET_SET_W(0x00C, int, Index);
	int					pIndex;						///< 00C
	NEWT_GET_ARR_W(0x010, TObject*, Entry, 0x80);
	TObject				*pEntry[0x80];				///< 010 Array of TObject*
};


class SingleObject
{
public:
	// blank
};


class TUObject : public SingleObject
{
public:
//	CUObject(ObjectId id = 0);
//	CUObject(CUObject & inCopy);
//	~CUObject();
//	
//	operator	ObjectId()	const;
//	void		operator=(ObjectId id);
//	void		operator=(const CUObject & inCopy);
//	
//	NewtonErr	makeObject(ObjectTypes inObjectType, struct ObjectMessage * inMsg, size_t inMsgSize);
//	void		destroyObject(void);
//	void		denyOwnership(void);
//	BOOL		isExtPage(void);
	
//	friend	void MonitorExitAction(ObjectId inMonitorId, ULong inAction);
//	friend	class CUMonitor;
//	
//	void		copyObject(const ObjectId inId);
//	void		copyObject(const CUObject & inCopy);
	
	NEWT_GET_SET_W(0x000, ObjectId, Id);
	ObjectId		pId;							///< 000
	NEWT_GET_SET_B(0x004, BOOL, ObjectCreatedByUs);
	BOOL			pObjectCreatedByUs;				///< 004
};


class TUSemaphoreOpList : public TUObject
{
public:
//	NewtonErr	init(ULong inNumOfArgs, ...);
};


class TUSemaphoreGroup : public TUObject
{
public:
//	NewtonErr	init(ULong inCount);
//	
//	NewtonErr	semOp(ObjectId inListId, SemFlags inBlocking);
//	
//	NewtonErr	setRefCon(void * inRefCon);
//	NewtonErr	getRefCon(void ** outRefCon);

	NewtonErr		semOp(TUSemaphoreOpList  inList, SemFlags inBlocking);

	//protected:
	NEWT_GET_SET_W(0x008, ULong*, RefCon);
	ULong			*pRefCon;						///< 008
};


class TParamBlock
{
public:
	NEWT_GET_SET_W(0x0B0, ULong, DomainAccess);
	ULong			pDomainAccess;					///< 0B0
};


typedef long (*NewtonInterruptHandler)(void *);


class InterruptObject
{
public:
	NEWT_GET_SET_W(0x008, ULong, HWIntMask);
	ULong			pHWIntMask;						///< 000 Hardware register mask 
//	long					x04;
	NEWT_GET_SET_W(0x008, ULong, Flags);
	ULong			pFlags;							///< 008 0x40->ClearInterrupt()
	NEWT_GET_SET_W(0x00C, InterruptObject*, Next);
	InterruptObject	*pNext;							///< 00C Next interrupt in queue
	NEWT_GET_SET_W(0x010, NewtonInterruptHandler, Handler);
	NewtonInterruptHandler gHandler;				///< 010 call this when the interupt is triggered
	NEWT_GET_SET_W(0x014, InterruptObject*, Queue);
	InterruptObject	*pQueue;						///< 014 queue this interrupt belongs to
//	long					x18;
//	short					x1C;			// priority?
//	short					x1E;			// |
};


extern void WantSchedule();
extern void ScheduleTask(TTask*);
extern void UnScheduleTask(TTask *inTask);
extern void SwapInGlobals(TTask*);
extern NewtonErr ClearInterrupt(class InterruptObject* interrupt);

extern "C" NewtonErr DoSemaphoreOp(ObjectId inGroupId, ObjectId inListId, SemFlags inBlocking, TTask *inTask);


extern void Func_0x0025215C(TARMProcessor* ioCPU, KUInt32 ret); // SwapInGlobals
extern void Func_0x001CC7F4(TARMProcessor* ioCPU, KUInt32 ret); // WantSchedule__Fv
extern void Func_0x001CC1B0(TARMProcessor* ioCPU, KUInt32 ret); // UpdateCurrentBucket__10TSchedulerFv
extern void Func_0x00359AA8(TARMProcessor* ioCPU, KUInt32 ret); // CheckBeforeAdd__10TTaskQueueFP5TTask
extern void Func_0x00359BE0(TARMProcessor* ioCPU, KUInt32 ret); // Peek__10TTaskQueueFv
extern void Func_0x00359AAC(TARMProcessor* ioCPU, KUInt32 ret); // Add__10TTaskQueueFP5TTask17KernelObjectStateP14TTaskContainer
extern void Func_0x00359B5C(TARMProcessor* ioCPU, KUInt32 ret); // RemoveFromQueue__10TTaskQueueFP5TTask17KernelObjectState
extern void Func_0x001CC564(TARMProcessor* ioCPU, KUInt32 ret); // Add__10TSchedulerFP5TTask
extern void Func_0x000E5960(TARMProcessor* ioCPU, KUInt32 ret); // ClearInterrupt
extern void Func_0x0009C77C(TARMProcessor* ioCPU, KUInt32 ret); // Remove__17TDoubleQContainerFv
extern void Func_0x00319F14(TARMProcessor* ioCPU, KUInt32 ret); // Get__12TObjectTableFUl
extern void Func_0x001918FC(TARMProcessor* ioCPU, KUInt32 ret); // UnScheduleTask__FP5TTask

#endif /* defined(_NEWT_TTASK_) */
