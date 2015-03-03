//
//  SWI.cp
//  Einstein
//
//  Created by Matthias Melcher on 2/28/14.
//  Code and Code Excerpts with kind permission from "Newton Research Group"
//
//

#include "SWI.h"

#include "globals.h"
#include "ScreenUpdateTask.h"

#include "TInterruptManager.h"


NewtonErr PANIC_ExitAtomicUnderflow()
{
	fprintf(stderr, "SYSTEM PANIC: ExitAtomic underflow\n\n");
	return -1;
}


void _ExitFIQAtomicFast()
{
	TARMProcessor *ioCPU = gCPU;
	
	// Decrement the FIQ lock count
	int nestCount = GAtomicFIQNestCountFast() - 1;
	SetGAtomicFIQNestCountFast( nestCount );
	
	// Panic, if we have more Exit calls than Enter calls
	if ( nestCount<0) {
		PANIC_ExitAtomicUnderflow();
		return;
	}
	
	// If we are still locked, do nothing
	if ( nestCount!=0 ) {
		return;
	}
	
	// Grab the interrup mask
	ULong intMask = GIntMaskShadowReg();
	
	// mask out the FIQ interrupt bit
	if ( GAtomicIRQNestCountFast()!=0 ) {
		intMask &= gCPU->GetMemory()->GetInterruptManager()->GetFIQMask();
	}
	
	// add some required bits
	intMask |= 0x0C400000;
	
	// Grab the processor status register
	ULong PSR = ioCPU->GetCPSR() & 0x0000001F;
	if (PSR!=0x10 && PSR!=0x00) {
		// don't do anything if we are not in supervisor mode
		gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( intMask );
	} else {
		// we are in supervisor mode
		if ( GWantDeferred() || GSchedule() ) {
			// allow tasks to switch
			gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( intMask );
			DoSchedulerSWI();
		} else {
			// just re-enable interrupts
			gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( intMask );
		}
	}
	
}

/**
 * _ExitFIQAtomicFast
 * ROM: 0x00392BB0 - 0x00392C4C
 */
void Func_0x00392BB0(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
		_ExitFIQAtomicFast();
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00392BB0, "_ExitFIQAtomicFast", Func_0x00392BB0)




void _EnterFIQAtomic()
{
	gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( 0x0c400000 );
	
	int nestCount = GAtomicFIQNestCountFast() + 1;
	SetGAtomicFIQNestCountFast( nestCount );
}

/**
 * Transcoded function _EnterFIQAtomic
 * ROM: 0x00392B90 - 0x00392BB0
 */
void Func_0x00392B90(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
	_EnterFIQAtomic();
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00392B90, "_EnterFIQAtomic", Func_0x00392B90)



void _ExitAtomic()
{
	// Decrement the IRQ lock count
	int nestCount = GAtomicIRQNestCountFast() - 1;
	SetGAtomicIRQNestCountFast( nestCount );
	
	// if the lock count is negative, we called ExitAtomic too often
	if ( nestCount<0 ) {
		PANIC_ExitAtomicUnderflow();
		return;
	}
	
	// if we are still locked, do nothing
	if ( nestCount!=0 ) {
		return;
	}
	
	// if the FIQ is still locked, don;t do anything either
	if ( GAtomicFIQNestCountFast()!=0 )
		return;
	
	// Grab the interrup mask
	ULong intMask = GIntMaskShadowReg() | 0x0c400000;
	
	// Grab the processor status register
	ULong PSR = gCPU->GetCPSR() & 0x0000001f;
	if (PSR!=0x10 && PSR!=0x00) {
		// don't do anything if we are not in supervisor mode
		gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( intMask );
	} else {
		// we are in supervisor mode
		if ( GWantDeferred() || GSchedule() ) {
			// allow tasks to switch
			gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( intMask );
			DoSchedulerSWI();
		} else {
			// just re-enable interrupts
			gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( intMask );
		}
	}
}

/**
 * Transcoded function _ExitAtomic
 * ROM: 0x00392B1C - 0x00392B90
 */
void Func_0x00392B1C(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
	_ExitAtomic();
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00392B1C, "_ExitAtomic", Func_0x00392B1C)



void PublicEnterAtomic()
{
	if ( GAtomicFIQNestCountFast()!=0 )
	{
		SetGAtomicIRQNestCountFast( GAtomicIRQNestCountFast() + 1 );
			return;
	}
	
	gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( 0x0C400000 );
	
	ULong mask = gCPU->GetMemory()->GetInterruptManager()->GetFIQMask();
	mask &= GIntMaskShadowReg();
	mask |= 0x0C400000;
	
	SetGAtomicIRQNestCountFast( GAtomicIRQNestCountFast() + 1 );
	
	gCPU->GetMemory()->GetInterruptManager()->SetIntCtrlReg( mask );
}


/**
 * PublicEnterAtomic
 * ROM: 0x00392AC0 - 0x00392B1C
 */
void Func_0x00392AC0(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
	PublicEnterAtomic();
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x00392AC0, "PublicEnterAtomic", Func_0x00392AC0)




NewtonErr UndefinedSWI()
{
	fprintf(stderr, "SYSTEM PANIC: Undefiend SWI\n\n");
	return -1;
}

NewtonErr SWIFromNonUserMode()
{
	fprintf(stderr, "SYSTEM PANIC: SWI from non-user mode (should be rebooting)\n\n");
	return -1;
}

/**
 * Transcoded function Func_0x003ADEE4
 * ROM: 0x003ADEE4 - 0x003ADFAC
 */
void SWI11_SemOp(TARMProcessor* ioCPU)
{
	// parameters come on the stack
	R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP);
	R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP+4);
	
	// get the currently running task
	TTask *task = GCurrentTask();
	
	// do the semaphore operation on it (all the code is native)
	R0 = (KUInt32)DoSemaphoreOp((ObjectId)(R0), (ObjectId)(R1), (SemFlags)(R2), task);
	
	// if there is still a current task, purge the arguments from the stack and return
	if ( GCurrentTask() ) {
		SP = SP + 8; // clean arguments from stack
		return;
	}
	
	LR = LR - 0x00000004;
	// TODO: in the following code, all registers are restored to the
	// values before calling DoSemaphoreOp. The above call adjusts the LR/PC
	// so that the same Semaphore Op is called again when this task is rescheduled.
	// In native mode, we will have to implement some kind of loop.
	
	ULong savedPSR   = ioCPU->GetSPSR();
	ULong currentPSR = ioCPU->GetCPSR();
	
	task->SetPSR( savedPSR );
	
	if ( (savedPSR&0x0000001F) != TARMProcessor::kUndefinedMode )
	{
		ioCPU->SetCPSR(0x000000D3);
		
		task->SetRegister( 2,  R2);
		task->SetRegister( 3,  R3);
		task->SetRegister( 4,  R4);
		task->SetRegister( 5,  R5);
		task->SetRegister( 6,  R6);
		task->SetRegister( 7,  R7);
		if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
			task->SetRegister( 8,  R8);
			task->SetRegister( 9,  R9);
			task->SetRegister(10, R10);
			task->SetRegister(11, R11);
			task->SetRegister(12, R12);
		} else {
			task->SetRegister( 9,  ioCPU->mR8_Bkup);
			task->SetRegister( 9,  ioCPU->mR9_Bkup);
			task->SetRegister(10, ioCPU->mR10_Bkup);
			task->SetRegister(11, ioCPU->mR11_Bkup);
			task->SetRegister(12, ioCPU->mR12_Bkup);
		}
		task->SetRegister(13, ioCPU->mR13_Bkup);
		task->SetRegister(14, ioCPU->mR14_Bkup);
		
		ioCPU->SetCPSR(currentPSR);
	}
	else
	{
		// untested code:
		task->SetRegister( 2,  R2);
		task->SetRegister( 3,  R3);
		task->SetRegister( 4,  R4);
		task->SetRegister( 5,  R5);
		task->SetRegister( 6,  R6);
		task->SetRegister( 7,  R7);
		task->SetRegister( 8,  R8);
		task->SetRegister( 9,  R9);
		task->SetRegister(10, R10);
		task->SetRegister(11, R11);
		task->SetRegister(12, R12);
		
		ioCPU->SetCPSR( savedPSR );
		task->SetRegister(13, SP);
		task->SetRegister(14, LR);
		ioCPU->SetCPSR( currentPSR );
	}
	
	task->SetRegister(15, LR);
	task->SetRegister(0, UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP));
	task->SetRegister(1, UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP+4));
	
	SP += 8; // clean up emulator stack
	
	return;
}

T_ROM_PATCH(0x003ADEE4, "SWI11_SemOp") {
	NEWT_NATIVE({
	SWI11_SemOp(ioCPU);
	})
	SETPC(0x003AD750+4);
	MMUCALLNEXT_AFTERSETPC
}



void SWI34_Scheduler(TARMProcessor* ioCPU)
{
	R0 = 0;
	R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, SP+4);
	SP += 8;
}

T_ROM_PATCH(0x003AE14C, "SWI34_Scheduler") {
	NEWT_NATIVE({
	SWI34_Scheduler(ioCPU);
	})
	SETPC(0x003AD750+4);
	MMUCALLNEXT_AFTERSETPC
}


/**
 * Transcoded function SWIBoot
 * ROM: 0x003AD698 - 0x003ADBB4
 */
void Func_0x003AD698(TARMProcessor* ioCPU, KUInt32 ret, KUInt32 inSWI)
{
	TTask *oldTask, *newTask, *task;
	TEnvironment *env;
	KUInt32 PSR;
	
	
	// 003AD698: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003AD698+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
	PSR = ( ioCPU->GetSPSR() & 0x1F );
	if (PSR!=0x10 && PSR!=0x00) {
		SWIFromNonUserMode();
	}
	
	GParamBlockFromImage()->SetDomainAccess( 0x55555555 );
	ioCPU->GetMemory()->SetDomainAccessControl( 0x55555555 );
	
	if (   GAtomicFIQNestCountFast()
		|| GAtomicIRQNestCountFast()
		|| GAtomicNestCount()
		|| GAtomicFIQNestCount())
	{
		ioCPU->SetCPSR(0x00000013);
	}
	
	switch (inSWI) {
		case 0x0000000B: SWI11_SemOp(ioCPU); break;
		case 0x00000022: SWI34_Scheduler(ioCPU); break;
		default: R0 = (KUInt32)UndefinedSWI(); break;
	}
	
	// 003AD74C: E790F101  ldr	pc, [r0, r1, lsl #2]
	//	switch (R1) {
	//		case  0: SETPC(0x003ADD3C+4); Func_0x003ADD3C(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  1: SETPC(0x003ADFAC+4); Func_0x003ADFAC(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  2: SETPC(0x003AE070+4); Func_0x003AE070(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  3: SETPC(0x00393D8C+4); Func_0x00393D8C(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  4: SETPC(0x00393E2C+4); Func_0x00393E2C(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  5: SETPC(0x003ADBB4+4); Func_0x003ADBB4(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  6: SETPC(0x003ADEDC+4); Func_0x003ADEDC(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  7: SETPC(0x003ADC88+4); Func_0x003ADC88(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  8: SETPC(0x003ADCB0+4); Func_0x003ADCB0(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case  9: SETPC(0x003ADCD4+4); Func_0x003ADCD4(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 10: SETPC(0x003ADD10+4); Func_0x003ADD10(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 11: SETPC(0x003ADEE4+4); Func_0x003ADEE4(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 12: SETPC(0x003ADD1C+4); Func_0x003ADD1C(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 13: SETPC(0x00394050+4); Func_0x00394050(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 14: SETPC(0x00394064+4); Func_0x00394064(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 15: SETPC(0x003940C0+4); Func_0x003940C0(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 16: SETPC(0x00394120+4); Func_0x00394120(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 17: SETPC(0x003941A4+4); Func_0x003941A4(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 18: SETPC(0x003941B8+4); Func_0x003941B8(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 19: SETPC(0x003941CC+4); Func_0x003941CC(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 20: SETPC(0x003941F8+4); Func_0x003941F8(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 21: SETPC(0x0039420C+4); Func_0x0039420C(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 22: SETPC(0x00394238+4); Func_0x00394238(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 23: SETPC(0x00394264+4); Func_0x00394264(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 24: SETPC(0x003ADE7C+4); Func_0x003ADE7C(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 25: SETPC(0x003ADEC8+4); Func_0x003ADEC8(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 26: SETPC(0x00394180+4); Func_0x00394180(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 27: SETPC(0x00394278+4); Func_0x00394278(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 28: SETPC(0x00394370+4); Func_0x00394370(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 29: SETPC(0x00394384+4); Func_0x00394384(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 30: SETPC(0x00393F10+4); Func_0x00393F10(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 31: SETPC(0x00393FD4+4); Func_0x00393FD4(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 32: SETPC(0x00394398+4); Func_0x00394398(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 33: SETPC(0x003AE138+4); Func_0x003AE138(ioCPU, 0x003AD750+4); goto L003AD750;
	//		case 34: SETPC(0x003AE14C+4); Func_0x003AE14C(ioCPU, 0x003AD750+4); goto L003AD750;
	//	}
	//	{
	//		KUInt32 offset = GetShiftNoCarryNoR15( 0xE790F101, ioCPU->mCurrentRegisters, ioCPU->mCPSR_C );
	//		KUInt32 theAddress = R0 + offset;
	//		SETPC(0x003AD74C+8);
	//		KUInt32 theData = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, theAddress);
	//		SETPC(theData + 4);
	//		return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
	//	}
	
	// 003AD750: E92D0003  stmfd	r13!, {r0-r1}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (2 * 4);
		baseAddress -= (2 * 4);
		SETPC(0x003AD750+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		SP = wbAddress;
	}
	
	ioCPU->SetCPSR(0x000000D3);
	
	// 003AD760: E59F1734  ldr	r1, 003ADE9C=SWIBoot+804
	R1 = 0x0C100E58; // gAtomicFIQNestCountFast
	
	// 003AD764: E5911000  ldr	r1, [r1]
	SETPC(0x003AD764+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003AD768: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD76C: 059F172C  ldreq	r1, 003ADEA0=SWIBoot+808
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100E5C; // gAtomicIRQNestCountFast
	}
	
	// 003AD770: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD770+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD774: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD778: 059F1724  ldreq	r1, 003ADEA4=SWIBoot+80C
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100FE8; // gAtomicNestCount
	}
	
	// 003AD77C: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD77C+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD780: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD784: 059F171C  ldreq	r1, 003ADEA8=SWIBoot+810
	if (ioCPU->TestEQ()) {
		R1 = 0x0C100FF0; // gAtomicFIQNestCount
	}
	
	// 003AD788: 05911000  ldreq	r1, [r1]
	if (ioCPU->TestEQ()) {
		SETPC(0x003AD788+8);
		R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	}
	
	// 003AD78C: 03510000  cmpeq	r1, #0x00000000
	if (ioCPU->TestEQ()) {
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003AD790: CA0000B7  bgt	003ADA74=SWIBoot+3DC
	if (ioCPU->TestLE()) {
		
		ioCPU->SetCPSR(0x00000093);
		
		R1 = GWantDeferred();
		
		// 003AD7A8: E3510000  cmp	r1, #0x00000000
		{
			KUInt32 Opnd2 = 0x00000000;
			KUInt32 Opnd1 = R1;
			const KUInt32 theResult = Opnd1 - Opnd2;
			ioCPU->mCPSR_Z = (theResult==0);
			ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
			ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
			ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
		}
		
		// 003AD7AC: 1A000005  bne	003AD7C8=SWIBoot+130
		if (ioCPU->TestNE()) {
			SETPC(0x003AD7C8+4);
			goto L003AD7C8;
		}
		
		R1 = GSchedule();
		
		// 003AD7B8: E3510000  cmp	r1, #0x00000000
		{
			KUInt32 Opnd2 = 0x00000000;
			KUInt32 Opnd1 = R1;
			const KUInt32 theResult = Opnd1 - Opnd2;
			ioCPU->mCPSR_Z = (theResult==0);
			ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
			ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
			ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
		}
		
		// 003AD7BC: 0A0000AC  beq	003ADA74=SWIBoot+3DC
		if (ioCPU->TestEQ()) {
			SETPC(0x003ADA74+4);
			goto L003ADA74;
		}
		
		// 003AD7C0: E92D5C0C  stmfd	r13!, {r2-r3, r10-r12, lr}
		{
			KUInt32 baseAddress = SP;
			KUInt32 wbAddress = baseAddress - (6 * 4);
			baseAddress -= (6 * 4);
			SETPC(0x003AD7C0+8);
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
			SP = wbAddress;
		}
		
		// 003AD7C4: EA00000B  b	003AD7F8=SWIBoot+160
		SETPC(0x003AD7F8+4);
		goto L003AD7F8;
		
	L003AD7C8:
		// 003AD7C8: E92D5C0C  stmfd	r13!, {r2-r3, r10-r12, lr}
		{
			KUInt32 baseAddress = SP;
			KUInt32 wbAddress = baseAddress - (6 * 4);
			baseAddress -= (6 * 4);
			SETPC(0x003AD7C8+8);
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
			UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
			SP = wbAddress;
		}
		
		ioCPU->SetCPSR(0x00000013);
		
		// 003AD7D8: EB5D28EA  bl	DoDeferrals
		{
			SETPC(0x003AD7D8+8);
			KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01AF7B88);
			if (jumpInstr!=0xEA9941C2) {
				__asm__("int $3\n" : : ); // unexpected jump table entry
				UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01AF7B88);
			}
		}
		LR = 0x003AD7D8 + 4;
		// rt cjitr DoDeferrals
		SETPC(0x00148298+4);
		Func_0x00148298(ioCPU, 0x003AD7E0);
		
		ioCPU->SetCPSR(0x00000093);
		
		R1 = GSchedule();
		
		if (R1==0)
		{
			
		L003AD7F8:
			// 003AD7F8: EB5D5EA8  bl	Scheduler
			{
				SETPC(0x003AD7F8+8);
				KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B052A0);
				if (jumpInstr!=0xEA9B1BD1) {
					__asm__("int $3\n" : : ); // unexpected jump table entry
					UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B052A0);
				}
			}
			LR = 0x003AD7F8 + 4;
			// rt cjitr Scheduler
			SETPC(0x001CC1EC+4);
			Func_0x001CC1EC(ioCPU, 0x003AD800);
			
			if (GWantSchedulerToRun()) {
				
				// 003AD80C: E92D0001  stmfd	r13!, {r0}
				{
					KUInt32 baseAddress = SP;
					KUInt32 wbAddress = baseAddress - (1 * 4);
					baseAddress -= (1 * 4);
					SETPC(0x003AD80C+8);
					UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
					SP = wbAddress;
				}
				
				// 003AD810: EB5D66F1  bl	StartScheduler
				{
					SETPC(0x003AD810+8);
					KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B073DC);
					if (jumpInstr!=0xEA9B1431) {
						__asm__("int $3\n" : : ); // unexpected jump table entry
						UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B073DC);
					}
				}
				LR = 0x003AD810 + 4;
				// rt cjitr StartScheduler
				SETPC(0x001CC4A8+4);
				Func_0x001CC4A8(ioCPU, 0x003AD818);
				
				// 003AD814: E8BD0001  ldmea	r13!, {r0}
				{
					KUInt32 baseAddress = SP;
					KUInt32 wbAddress = baseAddress + (1 * 4);
					SETPC(0x003AD814+8);
					R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					SP = wbAddress;
				}
			}
			
			R1 = (KUInt32)GCurrentTask();
			
			if (R1!=R0) {
				
				SetGCurrentTask( (TTask*)R0 );
				
				if (R1==0) SP = SP + 0x20;
				
				if (R1!=0)
				{
					
					// 003AD83C: E8BD5C0C  ldmea	r13!, {r2-r3, r10-r12, lr}
					{
						KUInt32 baseAddress = SP;
						KUInt32 wbAddress = baseAddress + (6 * 4);
						SETPC(0x003AD83C+8);
						R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						LR = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						SP = wbAddress;
					}
					
					// 003AD840: E1A00001  mov	r0, r1
					R0 = R1;
					
					if ( GCopyDone()==0 )
					{
						
						R1 = ioCPU->GetSPSR();
						
						// 003AD858: E5801050  str	r1, [r0, #0x050]
						SETPC(0x003AD858+8);
						UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R0 + 0x0050, R1);
						
						// 003AD85C: E201101F  and	r1, r1, #0x0000001f
						R1 = R1 & 0x0000001F;
						
						// 003AD860: E351001B  cmp	r1, #0x0000001b
						{
							KUInt32 Opnd2 = 0x0000001B;
							KUInt32 Opnd1 = R1;
							const KUInt32 theResult = Opnd1 - Opnd2;
							ioCPU->mCPSR_Z = (theResult==0);
							ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
							ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
							ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
						}
						
						// 003AD864: E2801018  add	r1, r0, #0x00000018
						R1 = R0 + 0x00000018; // 24
						
						// 003AD868: E92D0001  stmfd	r13!, {r0}
						{
							KUInt32 baseAddress = SP;
							KUInt32 wbAddress = baseAddress - (1 * 4);
							baseAddress -= (1 * 4);
							SETPC(0x003AD868+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
							SP = wbAddress;
						}
						
						R0 = ioCPU->GetCPSR();
						ioCPU->SetCPSR(0x000000D3);
						
						// 003AD87C: 18A100FC  stmneia	r1!, {r2-r7}
						if (ioCPU->TestNE()) {
							KUInt32 baseAddress = R1;
							KUInt32 wbAddress = baseAddress + (6 * 4);
							SETPC(0x003AD87C+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
							R1 = wbAddress;
						}
						
						// 003AD880: 18C17F00  stmneia	r1, {r8-lr}^
						if (ioCPU->TestNE()) {
							KUInt32 baseAddress = R1;
							SETPC(0x003AD880+8);
							if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R8); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R9); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
							} else {
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR8_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR9_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR10_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR11_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR12_Bkup); baseAddress += 4;
							}
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR13_Bkup); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR14_Bkup); baseAddress += 4;
						}
						
						ioCPU->SetCPSR(R0);
						
						// 003AD894: E8BD0001  ldmea	r13!, {r0}
						{
							KUInt32 baseAddress = SP;
							KUInt32 wbAddress = baseAddress + (1 * 4);
							SETPC(0x003AD894+8);
							R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
							SP = wbAddress;
						}
						
						// 003AD898: 1A00000B  bne	003AD8CC=SWIBoot+234
						if (ioCPU->TestNE()) {
							SETPC(0x003AD8CC+4);
							goto L003AD8CC;
						}
						
						// 003AD89C: E8A11FFC  stmia	r1!, {r2-r12}
						{
							KUInt32 baseAddress = R1;
							KUInt32 wbAddress = baseAddress + (11 * 4);
							SETPC(0x003AD89C+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R8); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R9); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
							R1 = wbAddress;
						}
						
						R4 = ioCPU->GetCPSR();
						R5 = ioCPU->GetSPSR();
						ioCPU->SetCPSR(R5);
						
						// 003AD8B8: E8816000  stmia	r1, {r13-lr}
						{
							KUInt32 baseAddress = R1;
							SETPC(0x003AD8B8+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, SP); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
						}
						
						ioCPU->SetCPSR(R4);
						
					L003AD8CC:
						// 003AD8CC: E2801010  add	r1, r0, #0x00000010
						R1 = R0 + 0x00000010; // 16
						
						// 003AD8D0: E581E03C  str	lr, [r1, #0x03c]
						SETPC(0x003AD8D0+8);
						UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x003C, LR);
						
						// 003AD8D4: E8BD000C  ldmea	r13!, {r2-r3}
						{
							KUInt32 baseAddress = SP;
							KUInt32 wbAddress = baseAddress + (2 * 4);
							SETPC(0x003AD8D4+8);
							R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
							R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
							SP = wbAddress;
						}
						
						// 003AD8D8: E881000C  stmia	r1, {r2-r3}
						{
							KUInt32 baseAddress = R1;
							SETPC(0x003AD8D8+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
						}
						
					}
					else
					{
						
						// 003AD8E0: E1A01000  mov	r1, r0
						R1 = R0;
						
						// 003AD8E4: E280004C  add	r0, r0, #0x0000004c
						R0 = R0 + 0x0000004C; // 76
						
						// 003AD8E8: E590E000  ldr	lr, [r0]
						SETPC(0x003AD8E8+8);
						LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
						
						// 003AD8EC: E2810010  add	r0, r1, #0x00000010
						R0 = R1 + 0x00000010; // 16
						
						// 003AD8F0: E890000C  ldmia	r0, {r2-r3}
						{
							KUInt32 baseAddress = R0;
							SETPC(0x003AD8F0+8);
							R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
							R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						}
						
						// 003AD8F4: E88D000C  stmea	r13, {r2-r3}
						{
							KUInt32 baseAddress = SP;
							SETPC(0x003AD8F4+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
						}
						
						// 003AD8F8: E1A00001  mov	r0, r1
						R0 = R1;
						
						R1 = ioCPU->GetSPSR();
						
						// 003AD900: E5801050  str	r1, [r0, #0x050]
						SETPC(0x003AD900+8);
						UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R0 + 0x0050, R1);
						
						// 003AD904: E201101F  and	r1, r1, #0x0000001f
						R1 = R1 & 0x0000001F;
						
						// 003AD908: E351001B  cmp	r1, #0x0000001b
						{
							KUInt32 Opnd2 = 0x0000001B;
							KUInt32 Opnd1 = R1;
							const KUInt32 theResult = Opnd1 - Opnd2;
							ioCPU->mCPSR_Z = (theResult==0);
							ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
							ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
							ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
						}
						
						// 003AD90C: E2801018  add	r1, r0, #0x00000018
						R1 = R0 + 0x00000018; // 24
						
						// 003AD910: E92D0001  stmfd	r13!, {r0}
						{
							KUInt32 baseAddress = SP;
							KUInt32 wbAddress = baseAddress - (1 * 4);
							baseAddress -= (1 * 4);
							SETPC(0x003AD910+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
							SP = wbAddress;
						}
						
						R0 = ioCPU->GetCPSR();
						
						ioCPU->SetCPSR(0x000000D3);
						
						// 003AD924: 18A100FC  stmneia	r1!, {r2-r7}
						if (ioCPU->TestNE()) {
							KUInt32 baseAddress = R1;
							KUInt32 wbAddress = baseAddress + (6 * 4);
							SETPC(0x003AD924+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
							R1 = wbAddress;
						}
						
						// 003AD928: 18C17F00  stmneia	r1, {r8-lr}^
						if (ioCPU->TestNE()) {
							KUInt32 baseAddress = R1;
							SETPC(0x003AD928+8);
							if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R8); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R9); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
							} else {
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR8_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR9_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR10_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR11_Bkup); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR12_Bkup); baseAddress += 4;
							}
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR13_Bkup); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, ioCPU->mR14_Bkup); baseAddress += 4;
						}
						
						ioCPU->SetCPSR(R0);
						
						// 003AD93C: E8BD0001  ldmea	r13!, {r0}
						{
							KUInt32 baseAddress = SP;
							KUInt32 wbAddress = baseAddress + (1 * 4);
							SETPC(0x003AD93C+8);
							R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
							SP = wbAddress;
						}
						
						// 003AD940: 1A00000B  bne	003AD974=SWIBoot+2DC
						if (ioCPU->TestEQ()) {
							
							// 003AD944: E8A11FFC  stmia	r1!, {r2-r12}
							{
								KUInt32 baseAddress = R1;
								KUInt32 wbAddress = baseAddress + (11 * 4);
								SETPC(0x003AD944+8);
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R4); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R5); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R6); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R7); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R8); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R9); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R10); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R11); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
								R1 = wbAddress;
							}
							
							
							R4 = ioCPU->GetCPSR();
							
							R5 = ioCPU->GetSPSR();
							
							ioCPU->SetCPSR(R5);
							
							// 003AD960: E8816000  stmia	r1, {r13-lr}
							{
								KUInt32 baseAddress = R1;
								SETPC(0x003AD960+8);
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, SP); baseAddress += 4;
								UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
							}
							
							ioCPU->SetCPSR(R4);
							
						}
						
						// 003AD974: E2801010  add	r1, r0, #0x00000010
						R1 = R0 + 0x00000010; // 16
						
						// 003AD978: E581E03C  str	lr, [r1, #0x03c]
						SETPC(0x003AD978+8);
						UJITGenericRetargetSupport::ManagedMemoryWrite(ioCPU, R1 + 0x003C, LR);
						
						// 003AD97C: E8BD000C  ldmea	r13!, {r2-r3}
						{
							KUInt32 baseAddress = SP;
							KUInt32 wbAddress = baseAddress + (2 * 4);
							SETPC(0x003AD97C+8);
							R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
							R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
							SP = wbAddress;
						}
						
						// 003AD980: E881000C  stmia	r1, {r2-r3}
						{
							KUInt32 baseAddress = R1;
							SETPC(0x003AD980+8);
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
							UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
						}
						
						SetGCopyDone( 0 );
					}
				}
				
			L003AD990:
				
				R0 = (KUInt32)GCurrentTask();
				
				// 003AD998: E1A04000  mov	r4, r0
				R4 = R0;
				
				// 003AD99C: EB5D6A9B  bl	SwapInGlobals
				{
					SETPC(0x003AD99C+8);
					KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B08410);
					if (jumpInstr!=0xEA9D2751) {
						__asm__("int $3\n" : : ); // unexpected jump table entry
						UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B08410);
					}
				}
				LR = 0x003AD99C + 4;
				// rt cjitr SwapInGlobals
				SETPC(0x0025215C+4);
				Func_0x0025215C(ioCPU, 0x003AD9A4);
				
				// 003AD9A0: E1A00004  mov	r0, r4
				R0 = R4;
				
				// 003AD9A4: E2800010  add	r0, r0, #0x00000010
				R0 = R0 + 0x00000010; // 16
				
				// 003AD9A8: E590E03C  ldr	lr, [r0, #0x03c]
				SETPC(0x003AD9A8+8);
				LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x003C);
				
				// 003AD9AC: E5901040  ldr	r1, [r0, #0x040]
				SETPC(0x003AD9AC+8);
				R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0040);
				
				// 003AD9B0: E169F001  msr	spsr_cf, r1
				ioCPU->SetSPSR(R1);
				
				// 003AD9BC: E201101F  and	r1, r1, #0x0000001f
				R1 = R1 & 0x0000001F;
				
				// 003AD9C0: E351001B  cmp	r1, #0x0000001b
				{
					KUInt32 Opnd2 = 0x0000001B;
					KUInt32 Opnd1 = R1;
					const KUInt32 theResult = Opnd1 - Opnd2;
					ioCPU->mCPSR_Z = (theResult==0);
					ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
					ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
					ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
				}
				
				// 003AD9C4: 18D07FFF  ldmneia	r0, {r0-lr}^
				if (ioCPU->TestNE()) {
					KUInt32 baseAddress = R0;
					SETPC(0x003AD9C4+8);
					R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R4 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R5 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R6 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R7 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					if (ioCPU->GetMode() != TARMProcessor::kFIQMode) {
						R8 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R9 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					} else {
						ioCPU->mR8_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						ioCPU->mR9_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						ioCPU->mR10_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						ioCPU->mR11_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
						ioCPU->mR12_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					}
					ioCPU->mR13_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					ioCPU->mR14_Bkup = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
				}
				
				// 003AD9CC: 1A00000A  bne	003AD9FC=SWIBoot+364
				if (ioCPU->TestNE()) {
					SETPC(0x003AD9FC+4);
					goto L003AD9FC;
				}
				
				// 003AD9D0: E5902040  ldr	r2, [r0, #0x040]
				SETPC(0x003AD9D0+8);
				R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0040);
				
				// 003AD9D4: E10F1000  mrs	r1, cpsr
				R1 = ioCPU->GetCPSR();
				
				ioCPU->SetCPSR(R2);
				
				// 003AD9E4: E590D034  ldr	r13, [r0, #0x034]
				SETPC(0x003AD9E4+8);
				SP = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0034);
				
				// 003AD9E8: E590E038  ldr	lr, [r0, #0x038]
				SETPC(0x003AD9E8+8);
				LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0 + 0x0038);
				
				ioCPU->SetCPSR(R1);
				
				// 003AD9F8: E8901FFF  ldmia	r0, {r0-r12}
				{
					KUInt32 baseAddress = R0;
					SETPC(0x003AD9F8+8);
					R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R4 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R5 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R6 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R7 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R8 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R9 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
				}
				
			L003AD9FC:
				// 003AD9FC: E92D0007  stmfd	r13!, {r0-r2}
				{
					KUInt32 baseAddress = SP;
					KUInt32 wbAddress = baseAddress - (3 * 4);
					baseAddress -= (3 * 4);
					SETPC(0x003AD9FC+8);
					UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
					UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
					UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
					SP = wbAddress;
				}
				
				// 003ADA00: E59F14B0  ldr	r1, 003ADEB8=SWIBoot+820
				R1 = 0x0C100FF8; // gCurrentTask
				
				// 003ADA04: E5911000  ldr	r1, [r1]
				SETPC(0x003ADA04+8);
				R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
				
				// 003ADA08: E3A00000  mov	r0, #0x00000000
				R0 = 0x00000000; // 0
				
				// 003ADA0C: E5912074  ldr	r2, [r1, #0x074]
				SETPC(0x003ADA0C+8);
				R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
				
				// 003ADA10: E3520000  cmp	r2, #0x00000000
				{
					KUInt32 Opnd2 = 0x00000000;
					KUInt32 Opnd1 = R2;
					const KUInt32 theResult = Opnd1 - Opnd2;
					ioCPU->mCPSR_Z = (theResult==0);
					ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
					ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
					ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
				}
				
				// 003ADA14: 15922010  ldrne	r2, [r2, #0x010]
				if (ioCPU->TestNE()) {
					SETPC(0x003ADA14+8);
					R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
				}
				
				// 003ADA18: 11800002  orrne	r0, r0, r2
				if (ioCPU->TestNE()) {
					R0 = R0 | R2;
				}
				
				// 003ADA1C: E5912078  ldr	r2, [r1, #0x078]
				SETPC(0x003ADA1C+8);
				R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0078);
				
				// 003ADA20: E3520000  cmp	r2, #0x00000000
				{
					KUInt32 Opnd2 = 0x00000000;
					KUInt32 Opnd1 = R2;
					const KUInt32 theResult = Opnd1 - Opnd2;
					ioCPU->mCPSR_Z = (theResult==0);
					ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
					ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
					ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
				}
				
				// 003ADA24: 15922010  ldrne	r2, [r2, #0x010]
				if (ioCPU->TestNE()) {
					SETPC(0x003ADA24+8);
					R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
				}
				
				// 003ADA28: 11800002  orrne	r0, r0, r2
				if (ioCPU->TestNE()) {
					R0 = R0 | R2;
				}
				
			L003ADA2C:
				// 003ADA2C: E591107C  ldr	r1, [r1, #0x07c]
				SETPC(0x003ADA2C+8);
				R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x007C);
				
				// 003ADA30: E3510000  cmp	r1, #0x00000000
				{
					KUInt32 Opnd2 = 0x00000000;
					KUInt32 Opnd1 = R1;
					const KUInt32 theResult = Opnd1 - Opnd2;
					ioCPU->mCPSR_Z = (theResult==0);
					ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
					ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
					ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
				}
				
				// 003ADA34: 0A000005  beq	003ADA50=SWIBoot+3B8
				if (ioCPU->TestEQ()) {
					SETPC(0x003ADA50+4);
					goto L003ADA50;
				}
				
				// 003ADA38: E5912074  ldr	r2, [r1, #0x074]
				SETPC(0x003ADA38+8);
				R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
				
				// 003ADA3C: E3520000  cmp	r2, #0x00000000
				{
					KUInt32 Opnd2 = 0x00000000;
					KUInt32 Opnd1 = R2;
					const KUInt32 theResult = Opnd1 - Opnd2;
					ioCPU->mCPSR_Z = (theResult==0);
					ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
					ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
					ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
				}
				
				// 003ADA40: 0A000002  beq	003ADA50=SWIBoot+3B8
				if (ioCPU->TestEQ()) {
					SETPC(0x003ADA50+4);
					goto L003ADA50;
				}
				
				// 003ADA44: E5922010  ldr	r2, [r2, #0x010]
				SETPC(0x003ADA44+8);
				R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
				
				// 003ADA48: E1800002  orr	r0, r0, r2
				R0 = R0 | R2;
				
				// 003ADA4C: EAFFFFF6  b	003ADA2C=SWIBoot+394
				SETPC(0x003ADA2C+4);
				goto L003ADA2C;
				
			L003ADA50:
				
				GParamBlockFromImage()->SetDomainAccess( R0 );
				ioCPU->GetMemory()->SetDomainAccessControl( R0 );
				
				// 003ADA68: E8BD0007  ldmea	r13!, {r0-r2}
				{
					KUInt32 baseAddress = SP;
					KUInt32 wbAddress = baseAddress + (3 * 4);
					SETPC(0x003ADA68+8);
					R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
					SP = wbAddress;
				}
				
				// 003ADA6C: E1B0F00E  movs	pc, lr
				{
					KUInt32 Opnd2 = LR;
					const KUInt32 theResult = Opnd2;
					SETPC(theResult + 4);
					ioCPU->SetCPSR( ioCPU->GetSPSR() );
					if (ret==0xFFFFFFFF)
						return; // Return to emulator
					if (PC!=ret)
						return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
					return;
				}
				
			}
		}
		
	L003ADA70:
		// 003ADA70: E8BD5C0C  ldmea	r13!, {r2-r3, r10-r12, lr}
		{
			KUInt32 baseAddress = SP;
			KUInt32 wbAddress = baseAddress + (6 * 4);
			SETPC(0x003ADA70+8);
			R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R10 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R11 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			LR = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			SP = wbAddress;
		}
		
	}
	
L003ADA74:
	// 003ADA74: E92D500C  stmfd	r13!, {r2-r3, r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (4 * 4);
		baseAddress -= (4 * 4);
		SETPC(0x003ADA74+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R3); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R12); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, LR); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADA84: 1B5D6654  blne	StartScheduler
	if ( GWantSchedulerToRun() ) {
		SETPC(0x003ADA84+8);
		KUInt32 jumpInstr = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, 0x01B073DC);
		if (jumpInstr!=0xEA9B1431) {
			__asm__("int $3\n" : : ); // unexpected jump table entry
			UJITGenericRetargetSupport::ReturnToEmulator(ioCPU, 0x01B073DC);
		}
		LR = 0x003ADA84 + 4;
		// rt cjitr StartScheduler
		SETPC(0x001CC4A8+4);
		Func_0x001CC4A8(ioCPU, 0x003ADA8C);
	}
	
	// 003ADA88: E8BD500C  ldmea	r13!, {r2-r3, r12, lr}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (4 * 4);
		SETPC(0x003ADA88+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R3 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R12 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		LR = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	if ( GCopyDone()==0 ) {
		
		// 003ADA9C: E8BD0003  ldmea	r13!, {r0-r1}
		{
			KUInt32 baseAddress = SP;
			KUInt32 wbAddress = baseAddress + (2 * 4);
			SETPC(0x003ADA9C+8);
			R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
			SP = wbAddress;
		}
		
		ULong domainAccess = 0;
		
		task = GCurrentTask();
		
		if (task->Environment())
			domainAccess |= task->Environment()->DomainAccess();
		
		if (task->SMemEnvironment())
			domainAccess |= task->SMemEnvironment()->DomainAccess();
		
		for (;;) {
			task = task->CurrentTask();
			if (task==NULL)
				break;
			
			TEnvironment *env = task->Environment();
			if (env==NULL)
				break;
			
			domainAccess |= env->DomainAccess();
		}
		
		GParamBlockFromImage()->SetDomainAccess( domainAccess );
		ioCPU->GetMemory()->SetDomainAccessControl( domainAccess );
		
		// 003ADB10: E1B0F00E  movs	pc, lr
		{
			KUInt32 Opnd2 = LR;
			const KUInt32 theResult = Opnd2;
			SETPC(theResult + 4);
			ioCPU->SetCPSR( ioCPU->GetSPSR() );
			if (ret==0xFFFFFFFF)
				return; // Return to emulator
			if (PC!=ret)
				return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
			return;
		}
		
	}
	
	SetGCopyDone( 0 );
	
	// 003ADB20: E59F0390  ldr	r0, 003ADEB8=SWIBoot+820
	R0 = 0x0C100FF8; // gCurrentTask
	
	// 003ADB24: E5900000  ldr	r0, [r0]
	SETPC(0x003ADB24+8);
	R0 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 003ADB28: E1A01000  mov	r1, r0
	R1 = R0;
	
	// 003ADB2C: E280004C  add	r0, r0, #0x0000004c
	R0 = R0 + 0x0000004C; // 76
	
	// 003ADB30: E590E000  ldr	lr, [r0]
	SETPC(0x003ADB30+8);
	LR = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R0);
	
	// 003ADB34: E2810010  add	r0, r1, #0x00000010
	R0 = R1 + 0x00000010; // 16
	
	// 003ADB38: E8900003  ldmia	r0, {r0-r1}
	{
		KUInt32 baseAddress = R0;
		SETPC(0x003ADB38+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
	}
	
	// 003ADB3C: E28DD008  add	r13, r13, #0x00000008
	SP = SP + 0x00000008; // 8
	
	// 003ADB40: E92D0007  stmfd	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress - (3 * 4);
		baseAddress -= (3 * 4);
		SETPC(0x003ADB40+8);
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R0); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R1); baseAddress += 4;
		UJITGenericRetargetSupport::ManagedMemoryWriteAligned(ioCPU, baseAddress, R2); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADB44: E59F136C  ldr	r1, 003ADEB8=SWIBoot+820
	R1 = 0x0C100FF8; // gCurrentTask
	
	// 003ADB48: E5911000  ldr	r1, [r1]
	SETPC(0x003ADB48+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1);
	
	// 003ADB4C: E3A00000  mov	r0, #0x00000000
	R0 = 0x00000000; // 0
	
	// 003ADB50: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADB50+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADB54: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB58: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADB58+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADB5C: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
	// 003ADB60: E5912078  ldr	r2, [r1, #0x078]
	SETPC(0x003ADB60+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0078);
	
	// 003ADB64: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB68: 15922010  ldrne	r2, [r2, #0x010]
	if (ioCPU->TestNE()) {
		SETPC(0x003ADB68+8);
		R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	}
	
	// 003ADB6C: 11800002  orrne	r0, r0, r2
	if (ioCPU->TestNE()) {
		R0 = R0 | R2;
	}
	
L003ADB70:
	// 003ADB70: E591107C  ldr	r1, [r1, #0x07c]
	SETPC(0x003ADB70+8);
	R1 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x007C);
	
	// 003ADB74: E3510000  cmp	r1, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R1;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB78: 0A000005  beq	003ADB94=SWIBoot+4FC
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADB94+4);
		goto L003ADB94;
	}
	
	// 003ADB7C: E5912074  ldr	r2, [r1, #0x074]
	SETPC(0x003ADB7C+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R1 + 0x0074);
	
	// 003ADB80: E3520000  cmp	r2, #0x00000000
	{
		KUInt32 Opnd2 = 0x00000000;
		KUInt32 Opnd1 = R2;
		const KUInt32 theResult = Opnd1 - Opnd2;
		ioCPU->mCPSR_Z = (theResult==0);
		ioCPU->mCPSR_N = ((theResult&0x80000000)!=0);
		ioCPU->mCPSR_C = ( ((Opnd1&~Opnd2)|(Opnd1&~theResult)|(~Opnd2&~theResult)) >> 31);
		ioCPU->mCPSR_V = ( ((Opnd1&~Opnd2&~theResult)|(~Opnd1&Opnd2&theResult)) >> 31);
	}
	
	// 003ADB84: 0A000002  beq	003ADB94=SWIBoot+4FC
	if (ioCPU->TestEQ()) {
		SETPC(0x003ADB94+4);
		goto L003ADB94;
	}
	
	// 003ADB88: E5922010  ldr	r2, [r2, #0x010]
	SETPC(0x003ADB88+8);
	R2 = UJITGenericRetargetSupport::ManagedMemoryRead(ioCPU, R2 + 0x0010);
	
	// 003ADB8C: E1800002  orr	r0, r0, r2
	R0 = R0 | R2;
	
	// 003ADB90: EAFFFFF6  b	003ADB70=SWIBoot+4D8
	SETPC(0x003ADB70+4);
	goto L003ADB70;
	
L003ADB94:
	
	GParamBlockFromImage()->SetDomainAccess( R0 );
	ioCPU->GetMemory()->SetDomainAccessControl( R0 );
	
	// 003ADBAC: E8BD0007  ldmea	r13!, {r0-r2}
	{
		KUInt32 baseAddress = SP;
		KUInt32 wbAddress = baseAddress + (3 * 4);
		SETPC(0x003ADBAC+8);
		R0 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R1 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		R2 = UJITGenericRetargetSupport::ManagedMemoryReadAligned(ioCPU, baseAddress); baseAddress += 4;
		SP = wbAddress;
	}
	
	// 003ADBB0: E1B0F00E  movs	pc, lr
	{
		KUInt32 Opnd2 = LR;
		const KUInt32 theResult = Opnd2;
		SETPC(theResult + 4);
		ioCPU->SetCPSR( ioCPU->GetSPSR() );
		if (ret==0xFFFFFFFF)
			return; // Return to emulator
		if (PC!=ret)
			return UJITGenericRetargetSupport::JumpToCalculatedAddress(ioCPU, PC, ret);
		return;
	}
	
	__asm__("int $3\n" : : ); // There was no return instruction found
}
//T_ROM_SIMULATION3(0x003AD698, "SWIBoot", Func_0x003AD698)


/**
 * Transcoded function _SemaphoreOpGlue
 * ROM: 0x003AE1FC - 0x003AE204
 */
void Func_0x003AE1FC(TARMProcessor* ioCPU, KUInt32 ret)
{
	// 003AE1FC: EF00000B  swi	0x0000000b
	SETPC(0x003AE1FC+8);
	ioCPU->DoSWI();
	Func_0x003AD698(ioCPU, 0x003AE204, 0x0000000b);
	
	// 003AE200: E1A0F00E  mov	pc, lr
	{
		KUInt32 Opnd2 = LR;
		const KUInt32 theResult = Opnd2;
		SETPC(theResult + 4);
		if (ret==0xFFFFFFFF)
			return; // Return to emulator
		if (PC!=ret)
			__asm__("int $3\n" : : ); // Unexpected return address
		return;
	}
	
	__asm__("int $3\n" : : ); // There was no return instruction found
}
T_ROM_SIMULATION3(0x003AE1FC, "_SemaphoreOpGlue", Func_0x003AE1FC)


/**
 * Atomically swap a memory location with a new value.
 * \param inGroupId semaphore group
 * \param inListId semaphore op list
 * \param inBlocking should we wait if the semphore is blocked?
 * \return error code
 */
NewtonErr SemaphoreOpGlue(ObjectId inGroupId, ObjectId inListId, SemFlags inBlocking)
{
	NEWT_NATIVE({
	gCPU->SetRegister(0, inGroupId);
	gCPU->SetRegister(1, inListId);
	gCPU->SetRegister(2, inBlocking);
	gCPU->SetRegister(15, 0x003AE1FC+8);
	gCPU->DoSWI();
	Func_0x003AD698(gCPU, -1, 0x0000000B);
	})
	return gCPU->GetRegister(0);
}


/**
 * Transcoded function DoSchedulerSWI
 * ROM: 0x003AD658 - 0x003AD698
 */
void Func_0x003AD658(TARMProcessor* ioCPU, KUInt32 ret)
{
	// 003AD658: EF000022  swi	0x00000022
	SETPC(0x003AD658+8);
	ioCPU->DoSWI();
	Func_0x003AD698(ioCPU, 0x003AD660, 0x22);
	
	// 003AD65C: E1A0F00E  mov	pc, lr
	{
		KUInt32 Opnd2 = LR;
		const KUInt32 theResult = Opnd2;
		SETPC(theResult + 4);
		if (ret==0xFFFFFFFF)
			return; // Return to emulator
		if (PC!=ret)
			__asm__("int $3\n" : : ); // Unexpected return address
		return;
	}
}
T_ROM_SIMULATION3(0x003AD658, "DoSchedulerSWI", Func_0x003AD658)

/**
 * Call the Scheduler to allow task switching.
 * \return error code
 */
NewtonErr DoSchedulerSWI()
{
	gCPU->SetRegister(15, 0x003AD658+8);
	gCPU->DoSWI();
	Func_0x003AD698(gCPU, -1, 0x0000000B);
	return gCPU->GetRegister(0);
}


void SetAlarm1(ULong inWhen)
{
	TInterruptManager *intMgr = gCPU->GetMemory()->GetInterruptManager();
	
	intMgr->SetTimerMatchRegister(2, inWhen);
	intMgr->SetIntEDReg1( intMgr->GetIntEDReg1() | 0x20 );
	SetGIntMaskShadowReg( GIntMaskShadowReg() | 0x20 );
}


/**
 * Transcoded function SetAlarm1
 * ROM: 0x003AD390 - 0x003AD3BC
 */
void Func_0x003AD390(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
		ULong inWhen = R0;
		SetAlarm1(inWhen);
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x003AD390, "SetAlarm1", Func_0x003AD390)


void DisableAlarm1()
{
	TInterruptManager *intMgr = gCPU->GetMemory()->GetInterruptManager();	
	
	intMgr->SetIntEDReg1( intMgr->GetIntEDReg1() & ~0x20 );
	SetGIntMaskShadowReg( GIntMaskShadowReg() & ~0x20 );
	intMgr->ClearInterrupts( 0x20 );
}

/**
 * Transcoded function DisableAlarm1
 * ROM: 0x003AD3BC - 0x003AD3EC
 */
void Func_0x003AD3BC(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
		DisableAlarm1();
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x003AD3BC, "DisableAlarm1", Func_0x003AD3BC)


BOOL SetAlarm(TTime *inTime)
{
	TInterruptManager *intMgr = gCPU->GetMemory()->GetInterruptManager();

	// get the alarm time
	KSInt32 alarmTimeHi = inTime->TimeHi();
	KUInt32 alarmTimeLo = inTime->TimeLo();
	
	// ignore requests in the past
	if (alarmTimeHi<0)
		return 0;
	
	// get the last timer sample
	KSInt32 sampleTimeHi = GTimerSampleHi();
	KUInt32 sampleTimeLo = GTimerSampleLo();
	
	// update the time to the 64bit time now
	KUInt32 nowLo = intMgr->GetTimer();

	// increment the high word if there was an overflow in the low word
	if ( nowLo<=sampleTimeLo )
		sampleTimeHi++;
	sampleTimeLo = nowLo;

	// return from function if the alarm time already passed
	if ( alarmTimeHi<sampleTimeHi || ( alarmTimeHi==sampleTimeHi && alarmTimeLo<sampleTimeLo ) )
		return 0;
	
	// register an interrupt at the alarm time
	SetAlarm1( alarmTimeLo );
	
	// calculate the new current time
	nowLo = intMgr->GetTimer();
	if ( nowLo<=sampleTimeLo )
		sampleTimeHi++;
	sampleTimeLo = nowLo;
	
	// if the alarm time did now pass, disable the timer interrupt again
	if ( alarmTimeHi<sampleTimeHi || ( alarmTimeHi==sampleTimeHi && alarmTimeLo<sampleTimeLo ) ) {
		DisableAlarm1();
		return 0;
	}
	
	return 1;
}

/**
 * Transcoded function SetAlarm
 * ROM: 0x003AD448 - 0x003AD4C4
 */
void Func_0x003AD448(TARMProcessor* ioCPU, KUInt32 ret)
{
	NEWT_NATIVE({
		TTime *inTime = (TTime*)R0;
		R0 = SetAlarm(inTime);
	})
	SETPC(LR+4);
}
T_ROM_SIMULATION3(0x003AD448, "SetAlarm", Func_0x003AD448)




