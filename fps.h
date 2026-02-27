#pragma once
#include "SafeWrite.h"

#pragma comment(lib, "winmm.lib")

float* const g_fMaxTime = reinterpret_cast<float*>(0x10F2BE8);

float fMaxTimeDefault = 0.f;
constexpr float fTimerOffsetMult = 0.9875f;
double dMaxTimeLowerBoundaryS = 0.016;
double dMaxFPSTolerance = 300;
double dMinFPSTolerance = 5;
double	dMaxTimeLowerBoundaryMS = 0;
double	dDesiredMaxMS = 0;
double	dDesiredMinMS = 0;
double	dDesiredMaxS = 0;
double	dDesiredMinS = 0;

bool IsInPauseFade() {
	return *reinterpret_cast<bool*>(0x107A0F5);
}

bool IsLoadingNewGame() {
	return *reinterpret_cast<bool*>(0x1075A07);
}

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

class BGSSaveLoadGame {
public:
	char		padding[0x244];
	uint32_t	uiGlobalFlags;
	uint8_t		ucCurrentMinorVersion;

	static BGSSaveLoadGame* GetSingleton() {
		return *reinterpret_cast<BGSSaveLoadGame**>(0x1079858);
	}
	bool IsLoading() const {
		return (uiGlobalFlags & 2) != 0;
	}
};

static_assert(sizeof(BGSSaveLoadGame) == 0x24C);

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

void ClampGameCounters(float& fClamp) {
	float& fMaxTime = *g_fMaxTime;
	if (fClamp > FLT_EPSILON) {
		fClamp = 1000.f / ((1000.f / fClamp) * fTimerOffsetMult);
		fMaxTime = 1000.f / ((1000.f / fMaxTime) * fTimerOffsetMult);
		if (fMaxTime < FLT_EPSILON) {
			fMaxTime = FLT_EPSILON;
		}
	}

	if (IsInPauseFade() || fClamp < FLT_EPSILON) {
		fMaxTime = fMaxTimeDefault;
	} else if (fMaxTime > dMaxTimeLowerBoundaryS) {
		fMaxTime = dMaxTimeLowerBoundaryS;
	}
	
}
class BSTimerSafe : public BSTimer {
public:
	void TimeGlobalHook(int time) {
		double dDelta = QPC::GetTimeDelta();

		float fMaxTime = dMaxTimeLowerBoundaryS;
		if (dDelta > FLT_EPSILON) {
			if (dDelta < dDesiredMinMS) {
				fMaxTime = dDelta > dDesiredMaxMS ? (dDelta / 1000.0) : dDesiredMaxS;
			}
			else {
				fMaxTime = dDesiredMinS;
			}
		}

		*g_fMaxTime = fMaxTime;

		fClamp = 0.f;

		BGSSaveLoadGame* pSaveLoad = BGSSaveLoadGame::GetSingleton();
		bool bSaveLoading = pSaveLoad && pSaveLoad->IsLoading();
		if (!bSaveLoading && !IsLoadingNewGame() && dDelta > 0.f) {
			if (dDelta < dDesiredMinMS)
				fClamp = dDelta > dDesiredMaxMS ? dDelta : dDesiredMaxMS;
			else
				fClamp = dDesiredMinMS;
		}

		ClampGameCounters(fClamp);

		Update(QPC::ReturnCounter());
	}

};

void FastExit()
{
	StartMenu* pStartMenu = StartMenu::GetSingleton();
	if (pStartMenu && pStartMenu->GetSettingsChanged())
		pStartMenu->SaveSettings();

	TerminateProcess(GetCurrentProcess(), 0);
}

void __fastcall TimerUpdateHook(BSTimerSafe* a1, void* edx, int time)
{
	a1->TimeGlobalHook(time);
}
void WritePatches() {
	
	QPC::Initialize();
	SafeWrite32(0xD9B090, (uintptr_t)QPC::ReturnCounter);

	fMaxTimeDefault = *g_fMaxTime;
	dDesiredMaxMS = 1000.0 / dMaxFPSTolerance;
	dDesiredMinMS = 1000.0 / dMinFPSTolerance;
	dDesiredMaxS = dDesiredMaxMS / 1000.0;
	dDesiredMinS = dDesiredMinMS / 1000.0;
	dMaxTimeLowerBoundaryMS = dMaxTimeLowerBoundaryS * 1000.0;
	WriteRelCall(0x6E43C7, UInt32(TimerUpdateHook));

	WriteRelJump(0x6EED54, UInt32(FastExit));
	
}