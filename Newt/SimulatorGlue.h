//
//  TTask.h
//  Einstein
//
//  Created by Matthias Melcher on 7/11/14.
//
//

#ifndef EINSTEIN_NEWT_SIMULATOR_GLUE_H
#define EINSTEIN_NEWT_SIMULATOR_GLUE_H


#ifdef IS_NEWT_SIM

#include "NewtSim.h"
#include <MacTypes.h>


/**
 * This macro declares global variables for access through the simulator.
 * \param type is the C++ type of the variable, KUInt32 if unknown.
 * \param name is the name of the variable.
 *   A constant numeric value, prefixed with ptr_ will be generated, pointing
 *   to the memory address of teh variable (real or simulated).
 * \param addr is the address in emulated memory.
 *   This parameter is ignored when compiled in native code.
 */
#define NEWT_DEF_GLOBAL(type, name, addr) \
extern type name; \
extern const KUInt32 ptr_##name;

#define NEWT_IMP_GLOBAL(type, name, addr) \
type name = 0; \
KUInt32 const ptr_##name = (KUInt32)(&name);

#else

#include <K/Defines/KDefinitions.h>
#include "TMemory.h"
#include "TARMProcessor.h"
#include "TJITGeneric_Macros.h"
#include "Emulator/JIT/Generic/TJITGenericROMPatch.h"
#include "Emulator/JIT/Generic/UJITGenericRetargetSupport.h"

#if 0
#define T_ROM_SIMULATION(addr, name) \
	T_ROM_INJECTION(addr, name) {\
		if (ioCPU->GetMode()==TARMProcessor::kUserMode) {\
			return ioUnit;\
		} else {\
			Func_##addr(ioCPU);\
			return 0;\
		}\
	}
#else
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
#endif

#define RT_PANIC_UNEXPECTED_RETURN_ADDRESS \
	UJITGenericRetargetSupport::UnexpectedPC(ioCPU);

#endif


#define NEWT_DEF_GLOBAL(type, name, addr) \
extern type name; \
extern const KUInt32 ptr_##name;

#define NEWT_IMP_GLOBAL(type, name, addr) \
KUInt32 const ptr_##name = (addr);

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

#define NEWT_GET_SET_W(offset, type, var) \
	type var() { return (type)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+offset); }; \
	void Set##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+offset, (KUInt32)(intptr_t)v); }
#define NEWT_GET_SET_W_V(offset, type, var) NEWT_GET_SET_W(offset, type, var)\
	type p##var;

#define NEWT_GET_SET_S(offset, type, var) \
	type var() { return (type)UJITGenericRetargetSupport::ManagedMemoryReadS(gCPU, (KUInt32(intptr_t(this)))+offset); }; \
	void Set##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWriteS(gCPU, (KUInt32(intptr_t(this)))+offset, (KUInt32)(intptr_t)v); }
#define NEWT_GET_SET_S_V(offset, type, var) NEWT_GET_SET_S(offset, type, var)\
	type p##var;

#define NEWT_GET_SET_B(offset, type, var) \
	type var() { return (type)UJITGenericRetargetSupport::ManagedMemoryReadB(gCPU, (KUInt32(intptr_t(this)))+offset); }; \
	void Set##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWriteB(gCPU, (KUInt32(intptr_t(this)))+offset, (KUInt8)v); }
#define NEWT_GET_SET_B_V(offset, type, var) NEWT_GET_SET_B(offset, type, var)\
	type p##var;

#define NEWT_GET_REF(offset, type, var) \
	type& var() { return *( (type*)((KUInt32(intptr_t(this)))+offset) ); }
#define NEWT_GET_REF_V(offset, type, var) NEWT_GET_REF(offset, type, var)\
	type p##var;

#define NEWT_GET_ARR_W(offset, type, var, n) \
	type var(int ix) { return (type)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, (KUInt32(intptr_t(this)))+offset+4*ix); }; \
	void Set##var(int ix, type v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, (KUInt32(intptr_t(this)))+offset+4*ix, (KUInt32)(intptr_t)v); }
#define NEWT_GET_ARR_W_V(offset, type, var, n) NEWT_GET_ARR_W_V(offset, type, var, n)\
	type p##var[n];

#define NEWT_GLOBAL_W_REF(offset, type, var) \
	inline type G##var() { return (type)offset; }

#define NEWT_GLOBAL_W_DEF(offset, type, var) \
	extern type G##var(); \
	extern void SetG##var(type);

#define NEWT_GLOBAL_W_IMP(offset, type, var) \
	type G##var() { return (type)(intptr_t)UJITGenericRetargetSupport::ManagedMemoryRead(gCPU, offset); } \
	void SetG##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWrite(gCPU, offset, (KUInt32)(intptr_t)(v)); }

#define NEWT_GLOBAL_B_DEF(offset, type, var) \
	extern type G##var(); \
	extern void SetG##var(type);

#define NEWT_GLOBAL_B_IMP(offset, type, var) \
	type G##var() { return (type)(intptr_t)UJITGenericRetargetSupport::ManagedMemoryReadB(gCPU, offset); } \
	void SetG##var(type v) { UJITGenericRetargetSupport::ManagedMemoryWriteB(gCPU, offset, (KUInt32)(intptr_t)(v)); }


#define NEWT_ASSERT(cond) \
	if (!(cond)) __asm__("int $3\n" : : );

#define NEWT_NATIVE(a) \
	UJITGenericRetargetSupport::BeginNativeCode(); a; UJITGenericRetargetSupport::EndNativeCode();

extern TARMProcessor *gCPU;


#include "native/types.h"


//#include "unsorted/unsorted_001.h"
//#include "unsorted/unsorted_002.h"
//#include "unsorted/unsorted_003.h"
//#include "unsorted/unsorted_004.h"
//#include "unsorted/unsorted_005.h"
//#include "unsorted/unsorted_006.h"
//#include "unsorted/unsorted_007.h"
//#include "unsorted/unsorted_008.h"
//#include "unsorted/unsorted_009.h"
//#include "unsorted/unsorted_010.h"
//#include "unsorted/unsorted_011.h"
//#include "unsorted/unsorted_012.h"
//#include "unsorted/unsorted_013.h"
//#include "unsorted/unsorted_014.h"
//#include "unsorted/unsorted_015.h"
//#include "unsorted/unsorted_016.h"
//#include "unsorted/unsorted_017.h"
//#include "unsorted/unsorted_018.h"
//#include "unsorted/unsorted_019.h"
//#include "unsorted/unsorted_020.h"
//#include "unsorted/unsorted_021.h"
//#include "unsorted/unsorted_022.h"
//#include "unsorted/unsorted_023.h"
//#include "unsorted/unsorted_024.h"
//#include "unsorted/unsorted_025.h"
//#include "unsorted/unsorted_026.h"
//#include "unsorted/unsorted_027.h"
//#include "unsorted/unsorted_028.h"

#include "classes/TSingleQContainer.h"
#include "classes/TInterpreter.h"
#include "classes/TSerialChipVoyager.h"

// hand coded files


#include "SimHandcoded.h"

#include "native/TTask.h"
#include "native/globals.h"
#include "native/swi.h"
#include "native/ScreenUpdateTask.h"

#endif /* EINSTEIN_NEWT_SIMULATOR_GLUE_H */
