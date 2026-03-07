#include <fps.h>
#include <SafeWrite.h>
#include <cfloat>

#pragma comment(lib, "winmm.lib")

namespace {
	float* const g_fMaxTime = reinterpret_cast<float*>(0x10F2BE8);

	float fMaxTimeDefault = 0.f;
	constexpr float fTimerOffsetMult = 0.9875f;
	constexpr float fMaxTimeLowerBoundaryS = 0.016f;
	double dMaxFPSTolerance = 300;
	double dMinFPSTolerance = 20;
	double dFrameTimeMinMS = 0;
	double dFrameTimeMaxMS = 0;
	double dFrameTimeMinS = 0;
	double dFrameTimeMaxS = 0;
}

namespace QPC {

	double GetTime() {
		LARGE_INTEGER liCounter;
		QueryPerformanceCounter(&liCounter);
		return double(liCounter.QuadPart) / double(liFrequency.QuadPart);
	}

	double GetTimeMS() {
		return GetTime() * 1000.0;
	}

	DWORD ReturnCounter() {
		return static_cast<DWORD>(GetTimeMS());
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
void BSTimerSafe::ClampGameCounters(float& fClamp) {


	float& fMaxTime = *g_fMaxTime;
	if (fClamp > FLT_EPSILON) {
		fClamp = 1000.f / ((1000.f / fClamp) * fTimerOffsetMult);
		fMaxTime = 1000.f / ((1000.f / fMaxTime) * fTimerOffsetMult);
		if (fMaxTime < FLT_EPSILON) {
			fMaxTime = FLT_EPSILON;
		}
	}

	float fClampS = fClamp / 1000.f;
	if (IsInPauseFade() || fClampS < FLT_EPSILON) {
		fMaxTime = fMaxTimeDefault;
	}
	else if (fMaxTime > fMaxTimeLowerBoundaryS) {
		fMaxTime = fMaxTimeLowerBoundaryS;
	}
}

void BSTimerSafe::CustomUpdate(int time) {
	double dDelta = QPC::GetTimeDelta();

	float fMaxTime = fMaxTimeLowerBoundaryS;
	if (dDelta > FLT_EPSILON) {
		if (dDelta < dFrameTimeMaxMS) {
			fMaxTime = dDelta > dFrameTimeMinMS ? static_cast<float>(dDelta / 1000.0) : static_cast<float>(dFrameTimeMinS);
		}
		else {
			fMaxTime = static_cast<float>(dFrameTimeMaxS);
		}
	}

	*g_fMaxTime = fMaxTime;

	fClamp = 0.f;

	BGSSaveLoadGame* pSaveLoad = BGSSaveLoadGame::GetSingleton();
	bool bSaveLoading = pSaveLoad && pSaveLoad->IsLoading();
	if (!bSaveLoading && !IsLoadingNewGame() && dDelta > 0.f) {
		if (dDelta < dFrameTimeMaxMS) {
			fClamp = dDelta > dFrameTimeMinMS ? static_cast<float>(dDelta) : static_cast<float>(dFrameTimeMinMS);
		}
		else {
			fClamp = static_cast<float>(dFrameTimeMaxMS);
		}
	}

	ClampGameCounters(fClamp);

	Update(QPC::ReturnCounter());
}

namespace {

	void FastExit()
	{
		StartMenu* pStartMenu = StartMenu::GetSingleton();
		if (pStartMenu && pStartMenu->GetSettingsChanged())
			pStartMenu->SaveSettings();

		TerminateProcess(GetCurrentProcess(), 0);
	}

	void __fastcall TimerUpdateHook(BSTimerSafe* a1, void* edx, int time)
	{
		a1->CustomUpdate(time);
	}

}

void WritePatches() {

	QPC::Initialize();
	SafeWrite32(0xD9B090, (uintptr_t)QPC::ReturnCounter);

	fMaxTimeDefault = *g_fMaxTime;
	dFrameTimeMinMS = 1000.0 / dMaxFPSTolerance;
	dFrameTimeMaxMS = 1000.0 / dMinFPSTolerance;
	dFrameTimeMinS = dFrameTimeMinMS / 1000.0;
	dFrameTimeMaxS = dFrameTimeMaxMS / 1000.0;
	WriteRelCall(0x6E43C7, UInt32(TimerUpdateHook));

	WriteRelJump(0x6EED54, UInt32(FastExit));

}
