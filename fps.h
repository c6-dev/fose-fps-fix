#pragma once
#include <chrono>
#include "SafeWrite.h"

float* fMaxTime = (float*)0x10F2BE8;
DWORD** BGSLoadGameSingleton = (DWORD**)0x1079858;
bool* g_bIsLoadingNewGame = (bool*)0x1075A07;

float fMaxTimeDefault = 0.f;
constexpr float fTimerOffsetMult = 0.9875f;
double dMaxTimeLowerBoundaryS = 0.016;
double dMaxFPSTolerance = 800;
double dMinFPSTolerance = 5;
double	dMaxTimeLowerBoundaryMS = 16;
double	dDesiredMaxMS = 1.25;
double	dDesiredMinMS = 200;
double	dDesiredMaxS = 0.00125;
double	dDesiredMinS = 0.2;

namespace QPC {
	LARGE_INTEGER	liFrequency;
	double			dLastCount = 0;

	double GetTime() {
		LARGE_INTEGER liCounter;
		QueryPerformanceCounter(&liCounter);
		return double(liCounter.QuadPart) / double(liFrequency.QuadPart);
	}

	double GetTimeMS() {
		return GetTime() * 1000.0;
	}

	DWORD ReturnCounter() {
		return GetTimeMS();
	}

	double GetTimeDelta() {
		double dCurrentCount = GetTimeMS();
		double dToReturn = dCurrentCount - dLastCount;
		dLastCount = dCurrentCount;
		return dToReturn;
	}

	void Initialize() {
		QueryPerformanceFrequency(&liFrequency);
		dLastCount = GetTimeMS();
	}
}

bool IsInPauseFade() {
	return *reinterpret_cast<bool*>(0x107A0F5);
}


class BSTimer {
public:
	uint8_t		ucDisableCounter;
	float		fClamp;
	float		fClampRemainder;
	float		fRealTimeDelta;
	uint32_t	uiLastTime;
	uint32_t	uiFirstTime;
	bool		bIsChangeTimeMultSlowly;
	bool		bSameFrameDelta;

	void Update(uint32_t auiTime) {
		ThisCall(0x86C280, this, auiTime);
	}
};

static_assert(sizeof(BSTimer) == 0x1C);

class BSTimerSafe : public BSTimer {
public:
	void TimeGlobalHook() {
		double dDelta = QPC::GetTimeDelta();


		float lfMaxTime = dMaxTimeLowerBoundaryS;
		if (dDelta > FLT_EPSILON) {
			if (dDelta < dDesiredMinMS) {
				lfMaxTime = dDelta > dDesiredMaxMS ? (dDelta / 1000.0) : dDesiredMaxS;
			}
			else {
				lfMaxTime = dDesiredMinS;
			}
		}

		*fMaxTime = lfMaxTime;


		fClamp = 0.f;

		bool bSaveLoading = (*BGSLoadGameSingleton && (*(*BGSLoadGameSingleton + 0x91) & 2) != 0);
		if (!bSaveLoading && !*g_bIsLoadingNewGame && dDelta > 0.f) {
			if (dDelta < dDesiredMinMS) {
				fClamp = dDelta > dDesiredMaxMS ? dDelta : dDesiredMaxMS;
			} else {
				fClamp = dDesiredMinMS;
			}
		}


		if (fClamp > FLT_EPSILON) {
			fClamp = 1000.f / ((1000.f / fClamp) * fTimerOffsetMult);

			*fMaxTime = 1000.f / ((1000.f / *fMaxTime) * fTimerOffsetMult);
			if (*fMaxTime < FLT_EPSILON) {
				*fMaxTime = FLT_EPSILON;
			}

		}

		if (IsInPauseFade() || fClamp < FLT_EPSILON) {
			*fMaxTime = fMaxTimeDefault;
		} else if (*fMaxTime > dMaxTimeLowerBoundaryS) {
			*fMaxTime = dMaxTimeLowerBoundaryS;
		}

		Update(QPC::ReturnCounter());
	}

};

class StartMenu {
public:
	char		padding[0x1A0];
	uint32_t	uiFlags;

	static StartMenu* GetSingleton() {
		return *reinterpret_cast<StartMenu**>(0x10770AC);
	}

	bool GetSettingsChanged() const {
		return uiFlags & 2;
	}

	static void SaveSettings() {
		CdeclCall(0x6806F0);
	}
};

static_assert(offsetof(StartMenu, uiFlags) == 0x1A0);


void FastExit()
{
	StartMenu* pStartMenu = StartMenu::GetSingleton();
	if (pStartMenu && pStartMenu->GetSettingsChanged())
		pStartMenu->SaveSettings();

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

	
	QPC::Initialize();
	SafeWrite32(0xD9B090, (uintptr_t)QPC::ReturnCounter);

	fMaxTimeDefault = *fMaxTime;

	ReplaceCallEx(0x6E43C7, &BSTimerSafe::TimeGlobalHook);

	WriteRelJump(0x6EED54, UInt32(FastExit));
	RemoveRenderer180CriticalSections();
	
}