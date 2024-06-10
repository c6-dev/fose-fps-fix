#pragma once
#include "common/IPrefix.h"

DECLSPEC_NOINLINE void SafeWrite8(UInt32 addr, UInt32 data);
DECLSPEC_NOINLINE void SafeWrite16(UInt32 addr, UInt32 data);
DECLSPEC_NOINLINE void SafeWrite32(UInt32 addr, UInt32 data);
DECLSPEC_NOINLINE void SafeWriteBuf(UInt32 addr, void * data, UInt32 len);

// 5 bytes
DECLSPEC_NOINLINE void WriteRelJump(UInt32 jumpSrc, UInt32 jumpTgt);
DECLSPEC_NOINLINE void WriteRelCall(UInt32 jumpSrc, UInt32 jumpTgt);


// 6 bytes
DECLSPEC_NOINLINE void WriteRelJnz(UInt32 jumpSrc, UInt32 jumpTgt);
DECLSPEC_NOINLINE void WriteRelJle(UInt32 jumpSrc, UInt32 jumpTgt);

DECLSPEC_NOINLINE void PatchMemoryNop(ULONG_PTR Address, SIZE_T Size);
void PatchMemoryNopRange(ULONG_PTR StartAddress, ULONG_PTR EndAddress);

template <typename T>
DECLSPEC_NOINLINE void WriteRelCall(UInt32 jumpSrc, T jumpTgt)
{
	WriteRelCall(jumpSrc, (UInt32)jumpTgt);
}

template <typename T>
DECLSPEC_NOINLINE void WriteRelJump(UInt32 jumpSrc, T jumpTgt)
{
	WriteRelJump(jumpSrc, (UInt32)jumpTgt);
}

DECLSPEC_NOINLINE void ReplaceCall(UInt32 jumpSrc, UInt32 jumpTgt);

template <typename T>
DECLSPEC_NOINLINE void ReplaceCall(UInt32 jumpSrc, T jumpTgt)
{
	ReplaceCall(jumpSrc, (UInt32)jumpTgt);
}

void ReplaceVirtualFunc(UInt32 jumpSrc, void* jumpTgt);