#pragma once

#include <game.h>
#include <Windows.h>

namespace QPC {
	inline LARGE_INTEGER	liFrequency;
	inline double			dLastCount = 0;

	double GetTime();

	double GetTimeMS();

	DWORD ReturnCounter();

	double GetTimeDelta();

	void Initialize();
}


class BSTimerSafe : public BSTimer {
public:
	void CustomUpdate(int time);

	static void ClampGameCounters(float& fClamp);
};

void WritePatches();
