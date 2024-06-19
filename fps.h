#pragma once
#include <chrono>

double g_iMaxFPSTolerance = 800;
double g_iMinFPSTolerance = 5;
double fDesiredMin = 1;
double fDesiredMax = 1000;
float fLowerMaxTimeBoundary = 0.016;
float fMaxTimeDefault = 0;
constexpr float fTimerOffsetMult = 0.9875;

float* g_FPSGlobal = (float*)0x1090BA4;
float* fMaxTime = (float*)0x10F2BE8;

DWORD** InterfSingleton = (DWORD**)0x1075B24;
DWORD** BGSLoadGameSingleton = (DWORD**)0x1079858;

bool* g_DialogMenu = (bool*)0x107613C;
bool* g_DialogMenu2 = (bool*)0x107A0F4;
bool* g_bIsMenuMode = (bool*)0x107A0F3;
bool* g_bIsInPauseFade = (bool*)0x107A0F5;
bool* g_bIsLoadingNewGame = (bool*)0x1075A07;

namespace QPC {

	double lastCount = 0;
	signed int tickCountBias = 0;
	using namespace std::chrono;

	void StartCounter()
	{
		auto now = duration_cast<milliseconds>(duration_cast<milliseconds>(time_point_cast<milliseconds>(steady_clock::now()).time_since_epoch()));
		long duration = now.count();


		QPC::tickCountBias = duration - long(GetTickCount()); //this will fail if your system was started 53 or more days ago
		QPC::lastCount = duration;

	}
	DWORD ReturnCounter()
	{
		auto now = duration_cast<milliseconds>(duration_cast<milliseconds>(time_point_cast<milliseconds>(steady_clock::now()).time_since_epoch()));
		return unsigned long(now.count() + tickCountBias);
	}

	double UpdateCounterMS()
	{

		auto now = duration_cast<nanoseconds>(duration_cast<nanoseconds>(time_point_cast<nanoseconds>(steady_clock::now()).time_since_epoch()));
		double currentCount = double(now.count()) / 1000000;
		double toReturn = currentCount - lastCount;
		lastCount = currentCount;
		return toReturn;

	}

}


void* __stdcall TimeGlobalHook() {

	double delta = QPC::UpdateCounterMS();
	
	if (delta <= FLT_EPSILON) {
		*fMaxTime = fLowerMaxTimeBoundary;
	}
	else if (delta >= fDesiredMax) {
		*fMaxTime = fDesiredMax / 1000;
	}
	else if (delta <= fDesiredMin) {
		*fMaxTime = fDesiredMin / 1000;
	}
	else {
		*fMaxTime = delta / 1000;
	}
	
	if ((*BGSLoadGameSingleton && (*(*BGSLoadGameSingleton + 0x91) & 2) != 0) || *g_bIsLoadingNewGame || delta <= 0) {
		*g_FPSGlobal = 0;
	} 
	else if (delta >= fDesiredMax) {
		*g_FPSGlobal = fDesiredMax;
	} 
	else if (delta <= fDesiredMin) {
		*g_FPSGlobal = fDesiredMin;
	} 
	else {
		*g_FPSGlobal = delta;
	}
	
	if (*g_FPSGlobal > FLT_EPSILON)
	{
		*g_FPSGlobal = 1000 / ((1000 / *g_FPSGlobal) * fTimerOffsetMult);
		*fMaxTime = 1000 / ((1000 / *fMaxTime) * fTimerOffsetMult);

		if (*fMaxTime < FLT_EPSILON) {
			*fMaxTime = FLT_EPSILON;
		}

	}
	if (*g_bIsInPauseFade || *g_FPSGlobal < FLT_EPSILON) {
		*fMaxTime = fMaxTimeDefault;
	}
	else if (*fMaxTime > fLowerMaxTimeBoundary) {
		*fMaxTime = fLowerMaxTimeBoundary;
	}

	return ThisStdCall<void*>(0x7F92A0, nullptr);
}

void FastExit()
{
	TerminateProcess(GetCurrentProcess(), 0);
}

void RemoveRenderer180CriticalSections()
{
	SafeWrite16(0x873165, 0x0BEB);
	SafeWrite8(0x87317C, 0x13);
	SafeWrite16(0x873189, 0x05EB);

	SafeWrite16(0x88B92F, 0x0BEB);
	SafeWrite8(0x88B942, 0x2F);
	SafeWrite8(0x88B954, 0x4E);
	SafeWrite16(0x88B96B, 0x05EB);

	SafeWrite16(0x88BA7A, 0x0BEB);
	SafeWrite8(0x88BA8D, 0x28);
	SafeWrite8(0x88BA9F, 0x5);
	SafeWrite16(0x88BAAF, 0x05EB);
}


void WritePatches() {

	
	QPC::StartCounter();
	SafeWrite32(0xD9B090, (uintptr_t)QPC::ReturnCounter);

	fDesiredMin = 1000.0 / g_iMaxFPSTolerance;
	fDesiredMax = 1000.0 / g_iMinFPSTolerance;
	fMaxTimeDefault = *fMaxTime;

	WriteRelCall(0x6EDC02, (uintptr_t)TimeGlobalHook);

	WriteRelJump(0x6EED54, UInt32(FastExit));
	RemoveRenderer180CriticalSections();
	
}