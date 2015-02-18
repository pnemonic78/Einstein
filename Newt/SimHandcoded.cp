//
//  SimHandcoded.cpp
//  Einstein
//
//  Created by Matthias Melcher on 7/30/14.
//
//

#include "SimHandcoded.h"

#include "TInterruptManager.h"
#include "TMemory.h"

#ifdef IS_NEWT_SIM
#include <stdlib.h>
#endif

// Dec 23rd, 2014
// Perf:  13525 (Debug, Sim), 38000 (run mode)
// Plain:  9300 (Debug)     , 26700 (run mode)

// keep this empty if possible


TARMProcessor *gCPU = 0;

// Disable this code temporarily for some TTask related testing
#if 0

class InterruptObject;


class TObject {
public:
	ObjectId GetId() { return (ObjectId)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x00); };
private:
	ObjectId	 fId;				// +00
	TObject		*fNext;				// +04
	ObjectId	 fOwnerId;			// +08
	ObjectId	 fAssignedOwnerId;	// +0C

};


class TEnvironment {
public:
	KUInt32 GetDomainAccess() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0x10); };
};


class TTask : public TObject {
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
};


class TParamBlock
{
public:
	void SetAccess(KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+0xB0, v); };
	KUInt32 Access() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+0xB0); };
};


TParamBlock* GParamBlockFromImage()
{
	return (TParamBlock*)0x0C008400;
}


TTask *GCurrentTask()
{
	return (TTask*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100FF8);
}


void GSetCurrentTask(TTask *newTask)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C100FF8, (KUInt32)(uintptr_t)newTask);
}


KUInt32 GCopyDone()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C101040);
}


void GSetCopyDone(KUInt32 v)
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


KUInt32 GSchedule()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100FE4);
}

KUInt32 GWantSchedulerToRun()
{
	return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C101A2C);
}

void GWantSchedulerToRun(KUInt32 v)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C101A2C, v);
}

void GSetCurrentTaskId(ObjectId id)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C101054, id);
}

void GSetCurrentGlobals(void *v)
{
	UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, 0x0C10105C, (uintptr_t)v);
}

void GSetCurrentMonitorId(ObjectId id)
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

InterruptObject *GGetSchedulerIntObj()
{
	return (InterruptObject*)(uintptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, 0x0C100E6C);
}


// This function calls many subroutines
void DoDeferrals()
{
	return Func_0x00148298(gCPU, 0xFFFFFFFF); // DoDeferrals
}


// This is a beast of a function, possibly allocating memory, etc.
TTask *Scheduler()
{
	Func_0x001CC1EC(gCPU, 0xFFFFFFFF); // Scheduler
	return (TTask*)(intptr_t)gCPU->mCurrentRegisters[0];
}


//// rt cjitr DisableInterrupt
//SETPC(0x000E5890+4);
//Func_0x000E5890(ioCPU, 0x001CC4D4);
void DisableInterrupt(InterruptObject *obj)
{
	KUInt32 oldR0 = gCPU->mCurrentRegisters[0];
	KUInt32 oldR2 = gCPU->mCurrentRegisters[2];
	KUInt32 oldR3 = gCPU->mCurrentRegisters[3];
	KUInt32 oldR12 = gCPU->mCurrentRegisters[12];
	KUInt32 oldLR = gCPU->mCurrentRegisters[14];
	gCPU->mCurrentRegisters[0] = (uintptr_t)obj;
	Func_0x000E5890(gCPU, 0xFFFFFFFF);
	gCPU->mCurrentRegisters[0] = oldR0;
	gCPU->mCurrentRegisters[2] = oldR2;
	gCPU->mCurrentRegisters[3] = oldR3;
	gCPU->mCurrentRegisters[12] = oldR12;
	gCPU->mCurrentRegisters[14] = oldLR;
}


//// rt cjitr QuickEnableInterrupt
//SETPC(0x000E57BC+4);
//Func_0x000E57BC(ioCPU, 0x001CC4FC);
void QuickEnableInterrupt(InterruptObject *obj)
{
	KUInt32 oldR0 = gCPU->mCurrentRegisters[0];
	KUInt32 oldR2 = gCPU->mCurrentRegisters[2];
	KUInt32 oldR3 = gCPU->mCurrentRegisters[3];
	KUInt32 oldR12 = gCPU->mCurrentRegisters[12];
	KUInt32 oldLR = gCPU->mCurrentRegisters[14];
	gCPU->mCurrentRegisters[0] = (uintptr_t)obj;
	Func_0x000E57BC(gCPU, 0xFFFFFFFF);
	gCPU->mCurrentRegisters[0] = oldR0;
	gCPU->mCurrentRegisters[2] = oldR2;
	gCPU->mCurrentRegisters[3] = oldR3;
	gCPU->mCurrentRegisters[12] = oldR12;
	gCPU->mCurrentRegisters[14] = oldLR;
}



/**
 * Start the scheduler and set a time for the next timer interrupt.
 */
void StartScheduler()
{
	TInterruptManager *interruptManager = gCPU->GetMemory()->GetInterruptManager();
	if (!GSchedulerRunning())
	{
		DisableInterrupt( GGetSchedulerIntObj() );
		KUInt32 time = interruptManager->GetTimer();
		interruptManager->SetTimerMatchRegister( 3, time + 73720 ); // 73720/3686 = 20ms
		GSchedulerRunning( 1 );
		QuickEnableInterrupt( GGetSchedulerIntObj() );
	}
	GWantSchedulerToRun(0);
}

// Glue.
void Func_0x001CC4A8(TARMProcessor* ioCPU, KUInt32 ret)
{
	gCPU = ioCPU;
	StartScheduler();
	NEWT_RETURN;
}
T_ROM_SIMULATION3(0x001CC4A8, "StartScheduler", Func_0x001CC4A8)


/**
 * This function sets a few global variables according to the task at hand.
 */
void SwapInGlobals(TTask *task)
{
	ObjectId id = task->GetId();
	GSetCurrentTaskId( id );

	void *globals = task->GetGlobals();
	GSetCurrentGlobals(globals);
	
	ObjectId monitorId = task->GetMonitorId();
	GSetCurrentMonitorId(monitorId);
}

// Glue.
void Func_0x0025215C(TARMProcessor* ioCPU, KUInt32 ret)
{
	gCPU = ioCPU;
	SwapInGlobals((TTask*)(uintptr_t)R0);
	NEWT_RETURN;
}
T_ROM_SIMULATION3(0x0025215C, "SwapInGlobals", Func_0x0025215C)



/**
 * This function is the common exit code for all SWI calls.
 *
 * This function handles task switching. The code below does pretty much the 
 * same that the original code would do to switch tasks, only it does that
 * for the emulator directly.
 * 
 * A nativ version of this code would mainly contain SwapContext() or the 
 * corresponding function for the host Fiber system.
 *
 * This function is very important in handling memory access violations. When
 * a DataAbort aoccurs, NewtonOS starts a Monitor (a Task that repairs the
 * memory access by another task) by raising the priority of the Monitor and
 * switching tasks. When the Monitor has fixed the issue, the original task
 * resumes at the instruction that caused the violation.
 *
 * This function calls other functions in NewtonOS that have no handcoded
 * version yet. The automatic translations of these functions will have side
 * effects on register.
 */
void Func_0x003AD750(TARMProcessor* ioCPU, KUInt32 ret)
{
	// Some variable that we will need throughout the code.
	bool mustSwitchTasks = false;
	TTask *oldTask = NULL, *newTask = NULL, *currentTask = 0L;
	KUInt32 currentCPUState = 0;
	KUInt32 callerCPUState = 0;
	KUInt32 newCPUState = 0;
	
	// Save some register for later.
	KUInt32 initialR0 = R0;
	KUInt32 initialR1 = R1;
	
	KUInt32 initialR2;
	KUInt32 initialR3;
	KUInt32 initialR10;
	KUInt32 initialR11;
	KUInt32 initialR12;
	KUInt32 initialLR;

	// Make things easier by having this globally available
	gCPU = ioCPU;
	
	ioCPU->SetCPSR(0x000000D3); // disable IRQ, disable FIQ
	
	// Check if we are in a nested SWI call.
	// If we are not, check if we must switch tasks.
	if (   GAtomicFIQNestCountFast()==0
		&& GAtomicIRQNestCountFast()==0
		&& GAtomicNestCount()==0
		&& GAtomicFIQNestCount()==0 )
	{
		// OK, we are not in a nested call. Do the full task management.
		ioCPU->SetCPSR(0x00000093);  // disable IRQ, enable FIQ
		
		// Were there any tasks put off for later?
		if ( GWantDeferred()==0 )
		{
			// No tasks deferred.
			
			// Leave the task switcher if gSchedule is not set.
			if (GSchedule()==0)
				goto skipTaskSwitcher;
			
			initialR2 = R2;
			initialR3 = R3;
			initialR10 = R10;
			initialR11 = R11;
			initialR12 = R12;
			initialLR = LR;
		}
		else
		{
			// Run the deferred tasks
			initialR2 = R2;
			initialR3 = R3;
			initialR10 = R10;
			initialR11 = R11;
			initialR12 = R12;
			initialLR = LR;
			
			ioCPU->SetCPSR(0x00000013);  // enable IRQ, enable FIQ
			DoDeferrals();
			ioCPU->SetCPSR(0x00000093);  // disable IRQ, enable FIQ
			
			// Leave the task switcher if gSchedule is not set.
			if (GSchedule()==0)
				goto skipTaskSwitcher2;
			
		}
		
		// Check if we want the scheduler to run
		newTask = Scheduler();
		
		if (GWantSchedulerToRun())
		{
			StartScheduler();
		}
		
		oldTask = GCurrentTask();
		
		// Check, if the Scheduler wants to run another task instead of the
		// current one
		if ( newTask!=oldTask )
		{
			// the old and the new tasks differ. Go ahead and switch tasks
			mustSwitchTasks = true;
			
			// This will be the new task.
			GSetCurrentTask( newTask );
			
			// Unless we were ideling around, there is a current task that must
			// be suspended.
			if ( oldTask )
			{
				// Store the current machine state in 'oldTask'
				R2 = initialR2;
				R3 = initialR3;
				R10 = initialR10;
				R11 = initialR11;
				R12 = initialR12;
				LR = initialLR;

				// This variable is related to the Memory Access Fault handling.
				if (GCopyDone()==0)
				{
					// save all registers as they are used in the CPU mode of the caller
					
					// save and evalute the caller CPU mode
					callerCPUState = ioCPU->GetSPSR();
					oldTask->SavePSR( callerCPUState );
					KUInt32 callerCPUMode = callerCPUState & 0x0000001F;
					bool isUnknownMode = (callerCPUMode==0x0000001B);
					
					if (isUnknownMode==false)
					{
						// save all common registers
						currentCPUState = ioCPU->GetCPSR();
						ioCPU->SetCPSR(0x000000D3); // disable IRQ, disable FIQ
						
						oldTask->SaveRegister(2, R2);
						oldTask->SaveRegister(3, R3);
						oldTask->SaveRegister(4, R4);
						oldTask->SaveRegister(5, R5);
						oldTask->SaveRegister(6, R6);
						oldTask->SaveRegister(7, R7);
						if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
							oldTask->SaveRegister(8, R8);
							oldTask->SaveRegister(9, R9);
							oldTask->SaveRegister(10, R10);
							oldTask->SaveRegister(11, R11);
							oldTask->SaveRegister(12, R12);
						} else {
							oldTask->SaveRegister(8, ioCPU->mR8_Bkup);
							oldTask->SaveRegister(9, ioCPU->mR9_Bkup);
							oldTask->SaveRegister(10, ioCPU->mR10_Bkup);
							oldTask->SaveRegister(11, ioCPU->mR11_Bkup);
							oldTask->SaveRegister(12, ioCPU->mR12_Bkup);
						}
						oldTask->SaveRegister(13, ioCPU->mR13_Bkup);
						oldTask->SaveRegister(14, ioCPU->mR14_Bkup);
						
						// restore the old CPU mode
						ioCPU->SetCPSR( currentCPUState );
					}
					else // isUnknownMode==true
					{
						// save all current registers
						oldTask->SaveRegister(2, R2);
						oldTask->SaveRegister(3, R3);
						oldTask->SaveRegister(4, R4);
						oldTask->SaveRegister(5, R5);
						oldTask->SaveRegister(6, R6);
						oldTask->SaveRegister(7, R7);
						oldTask->SaveRegister(8, R8);
						oldTask->SaveRegister(9, R9);
						oldTask->SaveRegister(10, R10);
						oldTask->SaveRegister(11, R11);
						oldTask->SaveRegister(12, R12);
						
						currentCPUState = ioCPU->GetCPSR();
						callerCPUState = ioCPU->GetSPSR();
						ioCPU->SetCPSR(callerCPUState);
						
						oldTask->SaveRegister(13, SP);
						oldTask->SaveRegister(14, LR);
						
						ioCPU->SetCPSR(currentCPUState);
					}
					// save the remaining regsters
					oldTask->SaveRegister(15, LR);
					oldTask->SaveRegister(0, initialR0);
					oldTask->SaveRegister(1, initialR1);
				}
				else // GCopyDone()==1
				{
					LR = oldTask->GetRegister(15);
					
					initialR0 = oldTask->GetRegister(0);
					initialR1 = oldTask->GetRegister(1);
					
					callerCPUState = ioCPU->GetSPSR();
					oldTask->SavePSR( callerCPUState );
					KUInt32 callerCPUMode = callerCPUState & 0x0000001F;
					bool isUnknownMode = (callerCPUMode==0x0000001B);
					
					if (isUnknownMode==false)
					{
						// save all common registers
						currentCPUState = ioCPU->GetCPSR();
						ioCPU->SetCPSR(0x000000D3); // disable IRQ, disable FIQ
						
						oldTask->SaveRegister(2, R2);
						oldTask->SaveRegister(3, R3);
						oldTask->SaveRegister(4, R4);
						oldTask->SaveRegister(5, R5);
						oldTask->SaveRegister(6, R6);
						oldTask->SaveRegister(7, R7);
						if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
							oldTask->SaveRegister(8, R8);
							oldTask->SaveRegister(9, R9);
							oldTask->SaveRegister(10, R10);
							oldTask->SaveRegister(11, R11);
							oldTask->SaveRegister(12, R12);
						} else {
							oldTask->SaveRegister(8, ioCPU->mR8_Bkup);
							oldTask->SaveRegister(9, ioCPU->mR9_Bkup);
							oldTask->SaveRegister(10, ioCPU->mR10_Bkup);
							oldTask->SaveRegister(11, ioCPU->mR11_Bkup);
							oldTask->SaveRegister(12, ioCPU->mR12_Bkup);
						}
						oldTask->SaveRegister(13, ioCPU->mR13_Bkup);
						oldTask->SaveRegister(14, ioCPU->mR14_Bkup);
						
						// restore the old CPU mode
						ioCPU->SetCPSR( currentCPUState );
					}
					else // isUnknownMode==true
					{
						// save all current registers
						oldTask->SaveRegister(2, R2);
						oldTask->SaveRegister(3, R3);
						oldTask->SaveRegister(4, R4);
						oldTask->SaveRegister(5, R5);
						oldTask->SaveRegister(6, R6);
						oldTask->SaveRegister(7, R7);
						oldTask->SaveRegister(8, R8);
						oldTask->SaveRegister(9, R9);
						oldTask->SaveRegister(10, R10);
						oldTask->SaveRegister(11, R11);
						oldTask->SaveRegister(12, R12);
						
						currentCPUState = ioCPU->GetCPSR();
						callerCPUState = ioCPU->GetSPSR();
						ioCPU->SetCPSR(callerCPUState);
						
						oldTask->SaveRegister(13, SP);
						oldTask->SaveRegister(14, LR);
						
						ioCPU->SetCPSR(currentCPUState);
					}
					// Save remaining registers
					oldTask->SaveRegister(15, LR);
					oldTask->SaveRegister(0, initialR0);
					oldTask->SaveRegister(1, initialR1);
					
					GSetCopyDone(0);
				}
			}
			
			// Restore the CPU state to that of the new Task by copying the PSR
			// and all regsiters from the TTask class
			
			// A side note: the floating point registers are emulated, but never
			// saved or restored. This means that there must be no FP operations
			// when switching tasks. So no SWI cals during math, unless task
			// switching is explicitly disabled!
			
			newTask = GCurrentTask();
			SwapInGlobals(newTask);
			
			LR = newTask->GetRegister(15);
			
			newCPUState = newTask->GetPSR();
			ioCPU->SetSPSR(newCPUState);
			KUInt32 newCPUMode = newCPUState & 0x0000001F;
			
			if ( newCPUMode!=0x0000001B ) // isUnknownMode==false
			{
				// Restore registers from Task
				R0 = newTask->GetRegister(0);
				R1 = newTask->GetRegister(1);
				R2 = newTask->GetRegister(2);
				R3 = newTask->GetRegister(3);
				R4 = newTask->GetRegister(4);
				R5 = newTask->GetRegister(5);
				R6 = newTask->GetRegister(6);
				R7 = newTask->GetRegister(7);
				if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
					R8 = newTask->GetRegister(8);
					R9 = newTask->GetRegister(9);
					R10 = newTask->GetRegister(10);
					R11 = newTask->GetRegister(11);
					R12 = newTask->GetRegister(12);
				} else {
					ioCPU->mR8_Bkup = newTask->GetRegister(8);
					ioCPU->mR9_Bkup = newTask->GetRegister(9);
					ioCPU->mR10_Bkup = newTask->GetRegister(10);
					ioCPU->mR11_Bkup = newTask->GetRegister(11);
					ioCPU->mR12_Bkup = newTask->GetRegister(12);
				}
				ioCPU->mR13_Bkup = newTask->GetRegister(13);
				ioCPU->mR14_Bkup = newTask->GetRegister(14);
			}
			else // isUnknownMode==true
			{
				// Restore registers from Task
				newCPUState = newTask->GetPSR();
				currentCPUState = ioCPU->GetCPSR();
				ioCPU->SetCPSR(newCPUState);
				
				SP = newTask->GetRegister(13);
				LR = newTask->GetRegister(14);
				
				ioCPU->SetCPSR(currentCPUState);
				
				R0 = newTask->GetRegister(0);
				R1 = newTask->GetRegister(1);
				R2 = newTask->GetRegister(2);
				R3 = newTask->GetRegister(3);
				R4 = newTask->GetRegister(4);
				R5 = newTask->GetRegister(5);
				R6 = newTask->GetRegister(6);
				R7 = newTask->GetRegister(7);
				
				R8 = newTask->GetRegister(8);
				R9 = newTask->GetRegister(9);
				R10 = newTask->GetRegister(10);
				R11 = newTask->GetRegister(11);
				R12 = newTask->GetRegister(12);
			}
			
			// Now we need to make sure that the Domain Access is set correctly
			newTask = GCurrentTask();
			
			KUInt32 taskDomainAccess = 0;
			TEnvironment *env;
			
			env = newTask->GetEnvironment();
			if (env) {
				taskDomainAccess |= env->GetDomainAccess();
			}
			
			env = newTask->GetSMemEnvironment();
			if (env) {
				taskDomainAccess |= env->GetDomainAccess();
			}
			
			currentTask = newTask;
			for (;;) {
				currentTask = currentTask->GetCurrentTask();
				if (!currentTask)
					break;
				
				env = currentTask->GetEnvironment();
				if (!env)
					break;
				
				taskDomainAccess |= env->GetDomainAccess();
			}
			
			GParamBlockFromImage()->SetAccess( taskDomainAccess );
			ioCPU->GetMemory()->SetDomainAccessControl( taskDomainAccess );
			
			// Jump back into the simulation or emulation
			PC = LR + 4;
			ioCPU->SetCPSR( ioCPU->GetSPSR() );
			
			// If we called this function from the JIT emulator, simply return to the emulation
			if (ret==0xFFFFFFFF)
				return;
			
			// If we are in native mode, and the task was switched, force a task switch on the simulator side as well.
			// In this context, this should be the standard way to retrun.
			// TODO: we want to avoid this call. Eventually, we just swicth host OS tasks.
			if (PC!=ret || mustSwitchTasks)
				return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
			
			// If we are in native mode, and the task was not switched, simply return to the caller.
			// In this context, this should never happen.
			return;
		}
		
	skipTaskSwitcher2:
		
		R2 = initialR2;
		R3 = initialR3;
		R10 = initialR10;
		R11 = initialR11;
		R12 = initialR12;
		LR = initialLR;
	}
	
	// We can only get here if this is a nested SWI call, or gSchedule was set
	// to false, or if the scheduler determined that we continue
	// with the same task
	
skipTaskSwitcher:
	
	if ( GWantSchedulerToRun() )
	{
		StartScheduler();
	}
	
	// This variable is related to the Memory Access Fault handling.
	if (GCopyDone()==0) {
		
		R0 = initialR0;
		R1 = initialR1;
		
		newTask = GCurrentTask();
		
		// Now we need to make sure that the Domain Access is set correctly
		KUInt32 taskDomainAccess = 0;
		TEnvironment *env;
		
		env = newTask->GetEnvironment();
		if (env) {
			taskDomainAccess |= env->GetDomainAccess();
		}
		
		env = newTask->GetSMemEnvironment();
		if (env) {
			taskDomainAccess |= env->GetDomainAccess();
		}
		
		currentTask = newTask;
		for (;;) {
			currentTask = currentTask->GetCurrentTask();
			if (!currentTask)
				break;
			
			env = currentTask->GetEnvironment();
			if (!env)
				break;
			
			taskDomainAccess |= env->GetDomainAccess();
		}
		
		GParamBlockFromImage()->SetAccess(taskDomainAccess);
		ioCPU->GetMemory()->SetDomainAccessControl( taskDomainAccess );
		
		// Jump back into the simulation or emulation
		PC = LR + 4;
		ioCPU->SetCPSR( ioCPU->GetSPSR() );
		
		// If we called this function from the JIT emulator, simply return to the emulation
		if (ret==0xFFFFFFFF)
			return;
		
		// If we are in native mode, and the task was switched, force a task switch on the simulator side as well.
		// In this context, this should not happen.
		// TODO: we want to avoid this call. Eventually, we just swicth host OS tasks.
		if (PC!=ret || mustSwitchTasks)
			return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
		
		// If we are in native mode, and the task was not switched, simply return to the caller.
		// In this context, this should be the right way to return.
		return;
	}
	else
	{		
		GSetCopyDone(0);
		
		newTask = GCurrentTask();
		LR = newTask->GetRegister(15);
		R0 = newTask->GetRegister(0);
		R1 = newTask->GetRegister(1);
		
		// Now we need to make sure that the Domain Access is set correctly
		KUInt32 taskDomainAccess = 0;
		TEnvironment *env;
		
		env = newTask->GetEnvironment();
		if (env) {
			taskDomainAccess |= env->GetDomainAccess();
		}
		
		env = newTask->GetSMemEnvironment();
		if (env) {
			taskDomainAccess |= env->GetDomainAccess();
		}

		currentTask = newTask;
		for (;;) {
			currentTask = currentTask->GetCurrentTask();
			if (!currentTask)
				break;

			env = currentTask->GetEnvironment();
			if (!env)
				break;
			
			taskDomainAccess |= env->GetDomainAccess();
		}
		
		GParamBlockFromImage()->SetAccess(taskDomainAccess);
		ioCPU->GetMemory()->SetDomainAccessControl( taskDomainAccess );
		
		// Jump back into the simulation or emulation
		PC = LR + 4;
		ioCPU->SetCPSR( ioCPU->GetSPSR() );
		
		// If we called this function from the JIT emulator, simply return to the emulation
		if (ret==0xFFFFFFFF)
			return;
		
		// If we are in native mode, and the task was switched, force a task switch on the simulator side as well.
		// In this context, this should not happen.
		// TODO: we want to avoid this call. Eventually, we just swicth host OS tasks.
		if (PC!=ret || mustSwitchTasks)
			return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
		
		// If we are in native mode, and the task was not switched, simply return to the caller.
		// In this context, this should be the right way to return.
		return;
	}
}

T_ROM_SIMULATION3(0x003AD750, "Func_0x003AD750", Func_0x003AD750)

#endif


// In the following code blocks, I try to understand which tasks are generated,
// how many tasks we need, and if they have a certain life span.
//
// Hardware interrupts are a special kind of task that should go through the
// scheduler as well. They could even be implemented as OS-level threads.
//
// I am still undecided if SWIs and FP emulation should be implemented with a
// context switch, or if they remain part of the current task with better
// privileges

// Findings:
// - The system creates 50 or so tasks and occasionally deletes them
// - Codecs can create tasks (MainEventLoop__13TCodecChannelSFPP13TCodecChannel)
// - we allocate 12 or so Monitor tasks for DataAbort management (MonitorEntryGlue)
// - one task has an entry in the REx
// - there is a screen update task (ScreenUpdateTask__FPvUlT2)
// - most tasks launch through TaskEntry__11TUTaskWorldFUlT1

// ScreenUpdateTask is a nice candidate. It's short and uses Semaphores and
// Sleep. It also uses an Einstein driver.

// TODO: TUTask (User Tasks) are not derived from TTask. Are they related, and how?
//       How do we connect these two systems?
//       At first look, User Tasks seem to be a subsystem to one specific
//       System Task, having much less overhead, seemingly not even a stack.

// 0x00252190: TTask::TTask(void)
// 0x00252250: TTask::FreeStack(void)
// 0x00252278: TTask::SetBequeathId(unsigned long)
// 0x002522B0: TTask::Init(void (*)(void *, unsigned long, #12), unsigned long, void *, unsigned long, unsigned long, unsigned long, TEnvironment *)
//             NewtonErr Init(TaskProcPtr inProc, size_t inStackSize, ObjectId inTaskId, ObjectId inDataId, ULong inPriority, ULong inName, CEnvironment * inEnvironment);
// 0x002526E4: TTask::~TTask(void)

// ioCPU->GetMemory()->GetFaultAddressRegister()

#include "TSymbolList.h"

T_ROM_INJECTION(0x00252190, "TTask::TTask(void)") {
	fprintf(stderr, "TTask::TTask(void)\n");
	return ioUnit;
}

T_ROM_INJECTION(0x002522B0, "TTask::Init(...)") {
	char buf[1024];
	KUInt32 r1 = ioCPU->GetRegister(1);
	TSymbolList::List->GetSymbol(r1, buf, 0, 0);
	KUInt32 sp = ioCPU->GetRegister(13);
	KUInt32 a6 = 0; ioCPU->GetMemory()->Read(sp+8, a6);
	// argument 6 is the name as a FourCC
	fprintf(stderr, "TTask::Init(this=0x%08x, '%c%c%c%c', fn=0x%08x(%s))\n",
			ioCPU->GetRegister(0),
			a6>>24, a6>>16, a6>>8, a6,
			r1,
			buf
			);
	return ioUnit;
}

T_ROM_INJECTION(0x002526E4, "TTask::~TTask(void)") {
	fprintf(stderr, "TTask::~TTask(this=0x%08x)\n",
			ioCPU->GetRegister(0)
			);
	return ioUnit;
}


/** A list of tasks as they are created, their initial function call, and their fourCC name
 
TTask::Init(this=0x0c108624, 'OBJM', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c1111d0, 'idle', fn=0x01b00078(OsBoot))
TTask::Init(this=0x0c111404, 'user', fn=0x01b094a8(UserBoot__Fv))
TTask::Init(this=0x0c1120ac, 'PMGR', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c1126d8, 'PTBL', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c112e00, 'STKF', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c113654, 'STKP', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c113dd8, 'STKU', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c118cd0, 'ROMF', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c119a54, 'ROMP', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c11aa88, 'ksrv', fn=0x01afde80(InitialKSRVTask__Fv))
TTask::Init(this=0x0c11ad7c, 'mntr', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c119c74, 'name', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c118dd8, 'pckm', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c1180a8, 'drvl', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c115c00, 'drvr', fn=0x00800884(ROM$$Size))
TTask::Init(this=0x0c115db4, 'alrt', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c113ee0, 'sndm', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c111720, 'cmgr', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c111ca0, 'cdfm', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c11b2c0, 'cdsv', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c11f450, 'cdpr', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c11f7f4, 'pg&e', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c120030, 'Tmux', fn=0x01a00024(MonitorEntryGlue))
TTask::Init(this=0x0c12043c, 'pssm', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c120b60, 'main', fn=0x01aa52d8(UserMain__Fv))
TTask::Init(this=0x0c120dfc, 'newt', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c1227f8, 'scrn', fn=0x01b4c600(ScreenUpdateTask__FPvUlT2))
TTask::Init(this=0x0c122bac, 'inkr', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c123598, 'codc', fn=0x01b7ef08(MainEventLoop__13TCodecChannelSFPP13TCodecChannel))
TTask::Init(this=0x0c12498c, 'newt', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c120b60, 'scpl', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c1218f0, 'fser', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c1218f0, 'fser', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c1252ec, 'fser', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c125dc0, 'fser', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))
TTask::Init(this=0x0c125380, 'codc', fn=0x01b7ef08(MainEventLoop__13TCodecChannelSFPP13TCodecChannel))
TTask::Init(this=0x0c1257a8, 'mnps', fn=0x01bdce7c(TaskEntry__11TUTaskWorldFUlT1))

 */

