//
//  SimulatorGlue.h
//  Einstein
//
//  Created by Matthias Melcher on 3/3/15.
//
//

#ifndef EINSTEIN_NEWT_SIMULATOR_GLUE_H
#define EINSTEIN_NEWT_SIMULATOR_GLUE_H

#include <K/Defines/KDefinitions.h>
#include "TMemory.h"
#include "TARMProcessor.h"
#include "TJITGeneric_Macros.h"
#include "Emulator/JIT/Generic/TJITGenericROMPatch.h"
#include "Emulator/JIT/Generic/UJITGenericRetargetSupport.h"


extern TARMProcessor *gCPU;


#define T_ROM_SIMULATION(addr, name) \
	T_ROM_INJECTION(addr, name) {\
		Func_##addr(ioCPU, 0xFFFFFFFF);\
		return 0;\
	}

#define T_ROM_SIMULATION3(addr, name, nativeCall) \
	T_ROM_INJECTION3(addr, name, nativeCall) {\
		Func_##addr(ioCPU, 0xFFFFFFFF);\
		return 0;\
	}

#define RT_PANIC_UNEXPECTED_RETURN_ADDRESS \
	UJITGenericRetargetSupport::UnexpectedPC(ioCPU);


#define R0 ioCPU->mCurrentRegisters[0]
#define R1 ioCPU->mCurrentRegisters[1]
#define R2 ioCPU->mCurrentRegisters[2]
#define R3 ioCPU->mCurrentRegisters[3]
#define R4 ioCPU->mCurrentRegisters[4]
#define R5 ioCPU->mCurrentRegisters[5]
#define R6 ioCPU->mCurrentRegisters[6]
#define R7 ioCPU->mCurrentRegisters[7]
#define R8 ioCPU->mCurrentRegisters[8]
#define R9 ioCPU->mCurrentRegisters[9]
#define R10 ioCPU->mCurrentRegisters[10]
#define R11 ioCPU->mCurrentRegisters[11]
#define R12 ioCPU->mCurrentRegisters[12]
#define SP ioCPU->mCurrentRegisters[13]
#define LR ioCPU->mCurrentRegisters[14]
#define PC ioCPU->mCurrentRegisters[15]

#define NEWT_RETURN \
	KUInt32 Opnd2 = LR; \
	const KUInt32 theResult = Opnd2; \
	SETPC(theResult + 4); \
	if (ret==0xFFFFFFFF) return; \
	if (PC!=ret) __asm__("int $3\n" : : ); \
	return

/**
 * Get or set a longlong sized (8 bytes) variable or pointer in a class.
 */
#define NEWT_GET_SET_L(offset, type, var) \
	type var() { return (type)( (((KUInt64)var##Hi())<<32) | ( ((KUInt64)var##Lo())) ); }; \
	KUInt32 var##Hi() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+offset); }; \
	KUInt32 var##Lo() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+offset+4); }; \
	void Set##var(type v) { Set##var##Hi(v>>32); Set##var##Lo(v); } \
	void Set##var##Hi(KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+offset, v); } \
	void Set##var##Lo(KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+offset+4, v); }
#define NEWT_GET_SET_L_V(offset, type, var) NEWT_GET_SET_W(offset, type, var)\
	type p##var;
//	union { type p##var; struct { KUInt32 p##var##Hi, p##var##Lo; } }

/**
 * Get or set a word sized (4 bytes) variable or pointer in a class.
 */
#define NEWT_GET_SET_W(offset, type, var) \
	type var() { return (type)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+offset); }; \
	void Set##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+offset, (KUInt32)(intptr_t)v); }
#define NEWT_GET_SET_W_V(offset, type, var) NEWT_GET_SET_W(offset, type, var)\
	type p##var;

/**
 * Get or set a short sized (2 bytes) variable or pointer in a class.
 */
#define NEWT_GET_SET_S(offset, type, var) \
	type var() { return (type)UJITGenericRetargetSupport::ManagedMemoryReadS(gCPU, (KUInt32(intptr_t(this)))+offset); }; \
	void Set##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWriteS(gCPU, (KUInt32(intptr_t(this)))+offset, (KUInt32)(intptr_t)v); }
#define NEWT_GET_SET_S_V(offset, type, var) NEWT_GET_SET_S(offset, type, var)\
	type p##var;

/**
 * Get or set a byte sized variable or pointer in a class.
 */
#define NEWT_GET_SET_B(offset, type, var) \
	type var() { return (type)UJITGenericRetargetSupport::ManagedMemoryReadB(gCPU, (KUInt32(intptr_t(this)))+offset); }; \
	void Set##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWriteB(gCPU, (KUInt32(intptr_t(this)))+offset, (KUInt8)v); }
#define NEWT_GET_SET_B_V(offset, type, var) NEWT_GET_SET_B(offset, type, var)\
	type p##var;

/**
 * Get the address of a member of a class.
 */
#define NEWT_GET_REF(offset, type, var) \
	type& var() { return *( (type*)((KUInt32(intptr_t(this)))+offset) ); }
#define NEWT_GET_REF_V(offset, type, var) NEWT_GET_REF(offset, type, var)\
	type p##var;

/**
 * Get or set a word sized (4 bytes) member of an array in a class.
 */
#define NEWT_GET_ARR_W(offset, type, var, n) \
	type var(int ix) { return (type)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+offset+4*ix); }; \
	void Set##var(int ix, type v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+offset+4*ix, (KUInt32)(intptr_t)v); }
#define NEWT_GET_ARR_W_V(offset, type, var, n) NEWT_GET_ARR_W_V(offset, type, var, n)\
	type p##var[n];


/**
 * Get the address of a global (fixed address) structure or array.
 */
#define NEWT_GLOBAL_W_REF(offset, type, var) \
	inline type G##var() { return (type)offset; }

/**
 * Read and write a word (4 bytes) at a fixed address.
 */
#define NEWT_GLOBAL_W_DEF(offset, type, var) \
	extern type G##var(); \
	extern void SetG##var(type);

#define NEWT_GLOBAL_W_IMP(offset, type, var) \
	type G##var() { return (type)(intptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, offset); } \
	void SetG##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, offset, (KUInt32)(intptr_t)(v)); }

/**
 * Read and write a byte at a fixed address.
 */
#define NEWT_GLOBAL_B_DEF(offset, type, var) \
	extern type G##var(); \
	extern void SetG##var(type);

#define NEWT_GLOBAL_B_IMP(offset, type, var) \
	type G##var() { return (type)(intptr_t)UJITGenericRetargetSupport::ManagedMemoryReadB(gCPU, offset); } \
	void SetG##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWriteB(gCPU, offset, (KUInt32)(intptr_t)(v)); }

/**
 * Read and write a longlong (8 bytes) at a fixed address.
 */
#define NEWT_GLOBAL_L_DEF(offset, type, var) \
	extern type G##var(); \
	extern KUInt32 G##var##Hi(); \
	extern KUInt32 G##var##Lo(); \
	extern void SetG##var(type); \
	extern void SetG##var##Hi(KUInt32); \
	extern void SetG##var##Lo(KUInt32);

#define NEWT_GLOBAL_L_IMP(offset, type, var) \
	type G##var() { return (type)( (((KUInt64)G##var##Hi())<<32) | ( ((KUInt64)G##var##Lo())) ); }; \
	KUInt32 G##var##Hi() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, offset); }; \
	KUInt32 G##var##Lo() { return UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, offset+4); }; \
	void SetG##var(type v) { SetG##var##Hi(v>>32); SetG##var##Lo(v); } \
	void SetG##var##Hi(KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, offset, v); } \
	void SetG##var##Lo(KUInt32 v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, offset+4, v); }


/**
 * Jump into the debugger if an assertion is not treu.
 */
#define NEWT_ASSERT(cond) \
	if (!(cond)) __asm__("int $3\n" : : );

/**
 * Use this Macro around emulator calls into native code.
 *
 * This code makes sure that native code does not misbehave when intertwined
 * with emulated code.
 *
 * \param a can be one or more C and C++ statments
 */
#define NEWT_NATIVE(a) \
	UJITGenericRetargetSupport::BeginNativeCode(); a; UJITGenericRetargetSupport::EndNativeCode();


#endif
